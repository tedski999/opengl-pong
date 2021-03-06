#include "renderer.h"
#include "core.h"
#include "resources.h"
#include "log.h"
#include "error.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define SHADER_ERROR_MSG_BUF_SIZE 256

static GLuint pong_renderer_internal_compileShader(const char *source, GLenum type);
static GLuint pong_renderer_internal_linkShaders(GLuint *shader_ids, unsigned int count);
#ifdef PONG_GL_DEBUG
static void pong_renderer_internal_glDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

static GLuint program_id;
static GLuint rect_vao_id;

void pong_renderer_init(void) {
	PONG_LOG_SUBGROUP_START("Renderer");
	PONG_LOG("Initializing renderer...", PONG_LOG_INFO);

	int gl_version = gladLoadGL(glfwGetProcAddress);
	if (gl_version == 0)
		PONG_ERROR("Could not load OpenGL!");
	if (GLAD_VERSION_MAJOR(gl_version) < 3 || (GLAD_VERSION_MAJOR(gl_version) == 3 && GLAD_VERSION_MINOR(gl_version) < 3))
		PONG_ERROR("Your graphics driver does not support OpenGL 3.3!");

#ifdef PONG_GL_DEBUG
	if (GLAD_VERSION_MAJOR(gl_version) < 4 || (GLAD_VERSION_MAJOR(gl_version) == 4 && GLAD_VERSION_MINOR(gl_version) < 3))
		PONG_ERROR("OpenGL debug output requested, but your graphics driver does not support OpenGL 4.3!");
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(pong_renderer_internal_glDebugMessageCallback, NULL);
#endif

	PONG_LOG("Vendor: %s", PONG_LOG_INFO, glGetString(GL_VENDOR));
	PONG_LOG("Renderer: %s", PONG_LOG_INFO, glGetString(GL_RENDERER));
	PONG_LOG("OpenGL v%d.%d", PONG_LOG_INFO, GLAD_VERSION_MAJOR(gl_version), GLAD_VERSION_MINOR(gl_version));
	PONG_LOG("GLSL %s", PONG_LOG_INFO, glGetString(GL_SHADING_LANGUAGE_VERSION));

	GLfloat rect_vertices[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	GLushort rect_indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &rect_vao_id);
	glBindVertexArray(rect_vao_id);

	GLuint rect_vbo_id;
	glGenBuffers(1, &rect_vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, rect_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 2 * 4, rect_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (GLfloat) * 2, 0);

	GLuint rect_ibo_id;
	glGenBuffers(1, &rect_ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect_ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (GLushort) * 2 * 3, rect_indices, GL_STATIC_DRAW);

	PONG_LOG_SUBGROUP_START("Shaders");
	PONG_LOG("Loading shaders...", PONG_LOG_VERBOSE);
	pong_resources_load("res/shaders/basic.vert", "basicVertShader");
	pong_resources_load("res/shaders/basic.frag", "basicFragShader");

	PONG_LOG("Compiling shaders...", PONG_LOG_VERBOSE);
	unsigned int shader_count = 2;
	GLuint shader_ids[shader_count];
	shader_ids[0] = pong_renderer_internal_compileShader(pong_resources_get("basicVertShader"), GL_VERTEX_SHADER);
	shader_ids[1] = pong_renderer_internal_compileShader(pong_resources_get("basicFragShader"), GL_FRAGMENT_SHADER);
	pong_resources_unload("basicVertShader");
	pong_resources_unload("basicFragShader");

	PONG_LOG("Linking shaders...", PONG_LOG_VERBOSE);
	program_id = pong_renderer_internal_linkShaders(shader_ids, shader_count);
	glDeleteShader(shader_ids[0]);
	glDeleteShader(shader_ids[1]);

	PONG_LOG("Configuring shaders...", PONG_LOG_VERBOSE);
	glUseProgram(program_id);
	mat4 projection_matrix = GLM_MAT4_IDENTITY_INIT;
	glm_ortho(-PONG_WINDOW_WIDTH / 2.f, PONG_WINDOW_WIDTH / 2.f, PONG_WINDOW_HEIGHT / 2.f, -PONG_WINDOW_HEIGHT / 2.f, 1.f, -1.f, projection_matrix);
	GLint projection_uniform_id = glGetUniformLocation(program_id, "projection"); // TODO: should projection (set only once) be a uniform?
	glUniformMatrix4fv(projection_uniform_id, 1, GL_FALSE, (float *) projection_matrix);
	PONG_LOG_SUBGROUP_END();

	PONG_LOG("Finishing OpenGL configuration...", PONG_LOG_VERBOSE);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	pong_renderer_clearScreen();

	PONG_LOG("Renderer initialized!", PONG_LOG_VERBOSE);
	PONG_LOG_SUBGROUP_END();
}

void pong_renderer_drawrect(float x, float y, float w, float h) {
	PONG_LOG_SUBGROUP_START("DrawRect");
	glUseProgram(program_id);

	mat4 transformation_matrix = GLM_MAT4_IDENTITY_INIT;
	vec3 translation_vector = { x, y, 0.0f };
	vec3 scaling_vector = { w, h, 0.0f };
	glm_translate(transformation_matrix, translation_vector);
	glm_scale(transformation_matrix, scaling_vector);

	// TODO: is there a better way of passing data to shader? uniforms vs attribs, etc
	GLint transformation_uniform_id = glGetUniformLocation(program_id, "transformation");
	glUniformMatrix4fv(transformation_uniform_id, 1, GL_FALSE, (float *) transformation_matrix);

	glBindVertexArray(rect_vao_id);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	PONG_LOG_SUBGROUP_END();
}

void pong_renderer_clearScreen(void) {
	PONG_LOG_SUBGROUP_START("ClearScreen");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PONG_LOG_SUBGROUP_END();
}

void pong_renderer_cleanup(void) {
	PONG_LOG_SUBGROUP_START("Renderer");
	PONG_LOG("Cleaning up renderer...", PONG_LOG_INFO);
	if (program_id)
		glDeleteProgram(program_id);
	PONG_LOG_SUBGROUP_END();
}

static GLuint pong_renderer_internal_compileShader(const char *source, GLenum type) {
	PONG_LOG("Compiling shader...", PONG_LOG_VERBOSE);
	GLuint shader_id = glCreateShader(type);
	glShaderSource(shader_id, 1, &source, NULL);
	glCompileShader(shader_id);

	GLint compiled_status;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_status);
	if (compiled_status != GL_TRUE) {
		GLchar message[SHADER_ERROR_MSG_BUF_SIZE];
		glGetShaderInfoLog(shader_id, SHADER_ERROR_MSG_BUF_SIZE, NULL, message);
		PONG_ERROR("Unable to compile shader: %s", message);
	}

	return shader_id;
}

static GLuint pong_renderer_internal_linkShaders(GLuint *shader_ids, unsigned int count) {
	PONG_LOG("Linking %i shaders into a program...", PONG_LOG_VERBOSE, count);
	GLuint program_id = glCreateProgram();
	while (count--)
		glAttachShader(program_id, shader_ids[count]);
	glLinkProgram(program_id);

	GLint linked_status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &linked_status);
	if (linked_status != GL_TRUE) {
		GLchar message[SHADER_ERROR_MSG_BUF_SIZE];
		glGetShaderInfoLog(program_id, SHADER_ERROR_MSG_BUF_SIZE, NULL, message);
		PONG_ERROR("Unable to link shaders: %s", message);
	}

	return program_id;
}

#ifdef PONG_GL_DEBUG
static void pong_renderer_internal_glDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	enum PongLogUrgency urgency = PONG_LOG_VERBOSE;
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			urgency = PONG_LOG_NOTEWORTHY;
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			urgency = PONG_LOG_INFO;
			break;
		case GL_DEBUG_SEVERITY_LOW:
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			urgency = PONG_LOG_VERBOSE;
			break;
	}

	const char *type_str = "UNKNOWN";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			type_str = "ERROR";
			urgency = PONG_LOG_ERROR;
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			type_str = "DEPRECATED";
			urgency = PONG_LOG_WARNING;
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			type_str = "PERFORMANCE";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			type_str = "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_MARKER:
		case GL_DEBUG_TYPE_POP_GROUP:
		case GL_DEBUG_TYPE_PUSH_GROUP:
		case GL_DEBUG_TYPE_OTHER:
			type_str = "DEBUG";
			break;
	}

	PONG_LOG("OpenGL (%s) -> %s", urgency, type_str, message);
}
#endif

