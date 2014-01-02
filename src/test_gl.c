#include <stdio.h>
#include <GL/glew.h>

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include "vec4.h"
#include "mat4.h"

const int WIN_WIDTH = 640;
const int WIN_HEIGHT = 480;

const char *vert_shader_src =
"#version 330 core\n\
layout(location = 0) in vec4 pos;\n\
uniform mat4 model, view, proj;\n\
void main(void){ gl_Position = proj * view * model * pos; }";

const char *frag_shader_src =
"#version 330 core\n\
out vec4 color;\n\
void main(void){ color = vec4(1.f, 1.f, 1.f, 1.f); }";

/* Check if an OpenGL error occured, if one occured log it w/ the msg and return 1 */
int check_GL_error(const char *msg);
/* Read all contents from a file */
char* read_file(const char *fname);
/* Compile a shader program using the source given, return -1 on failure */
GLint make_shader(const char *src, GLenum type);
/* Link the shaders into a program, return -1 on failure */
GLint make_program(GLuint vert, GLuint frag);
/* OpenGL debug callback */
#ifdef _WIN32
void APIENTRY gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei len, const GLchar *msg, GLvoid *usr);
#else
void gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei len, const GLchar *msg, GLvoid *usr);
#endif

int main(int argc, char **argv){
	/* vertices for a triangle */
	/* Why does triangle[] = { vec4_new(...) } result in a segfault when returning
	 * from _mm_load_ps?
	 */
	/* Just want a sort of 3D object, but not a closed object otherwise it's hard
	 * to tell what's going on w/ flat shading */
	vec4_t object[18];
	//+Z face
	object[0] = vec4_new(-1, -1, 1, 1);
	object[1] = vec4_new(1, -1, 1, 1);
	object[2] = vec4_new(1, 1, 1, 1);
	object[3] = vec4_new(1, 1, 1, 1);
	object[4] = vec4_new(-1, 1, 1, 1);
	object[5] = vec4_new(-1, -1, 1, 1);
	//+X face
	object[6] = vec4_new(1, -1, 1, 1);
	object[7] = vec4_new(1, -1, -1, 1);
	object[8] = vec4_new(1, 1, -1, 1);
	object[9] = vec4_new(1, 1, -1, 1);
	object[10] = vec4_new(1, 1, 1, 1);
	object[11] = vec4_new(1, -1, 1, 1);
	//-X face
	object[12] = vec4_new(-1, -1, 1, 1);
	object[13] = vec4_new(-1, -1, -1, 1);
	object[14] = vec4_new(-1, 1, -1, 1);
	object[15] = vec4_new(-1, 1, -1, 1);
	object[16] = vec4_new(-1, 1, 1, 1);
	object[17] = vec4_new(-1, -1, 1, 1);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#ifdef DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	SDL_Window *win = SDL_CreateWindow("SSE GL Test", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(win);

	if (check_GL_error("Opened win + context")){
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);
		return 1;
	}

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK){
		fprintf(stderr, "GLEW init error %d\n", err);
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);
		return 1;
	}
	check_GL_error("Post GLEW init");
#ifdef DEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(gl_debug_callback, NULL);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
		0, NULL, GL_TRUE);
#endif
	glClearColor(0, 0, 0, 1);
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);

	//Model's vao and vbo
	GLuint model[2];
	glGenVertexArrays(1, model);
	glBindVertexArray(model[0]);
	glGenBuffers(1, model + 1);
	glBindBuffer(GL_ARRAY_BUFFER, model[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 6 * 3 * sizeof(GLfloat), object, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	if (check_GL_error("Setup buffers")){
		return 1;
	}

	GLint vshader = make_shader(vert_shader_src, GL_VERTEX_SHADER);
	GLint fshader = make_shader(frag_shader_src, GL_FRAGMENT_SHADER);
	if (vshader == -1 || fshader == -1){
		return 1;
	}
	GLint program = make_program(vshader, fshader);
	if (program == -1){
		return 1;
	}
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	mat4_t model_mat = mat4_mult(mat4_rotate(45, vec4_new(1, 1, 0, 0)), mat4_scale(2, 2, 2));
	model_mat = mat4_mult(mat4_translate(vec4_new(0, 2, -5, 1)), model_mat);
	mat4_t view_mat = mat4_look_at(vec4_new(0, 0, 5, 0), vec4_new(0, 0, 0, 0), vec4_new(0, 1, 0, 0));
	mat4_t proj_mat = mat4_perspective(75, ((float)WIN_WIDTH) / WIN_HEIGHT, 1, 100);
	glUseProgram(program);
	GLuint model_unif = glGetUniformLocation(program, "model");
	GLuint view_unif = glGetUniformLocation(program, "view");
	GLuint proj_unif = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(model_unif, 1, GL_FALSE, (GLfloat*)&model_mat);
	glUniformMatrix4fv(view_unif, 1, GL_FALSE, (GLfloat*)&view_mat);
	glUniformMatrix4fv(proj_unif, 1, GL_FALSE, (GLfloat*)&proj_mat);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 18);
	SDL_GL_SwapWindow(win);
	check_GL_error("Post Draw");

	SDL_Event e;
	int quit = 0;
	while (!quit){
		while (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN){
				quit = 1;
			}
		}
	}

	glDeleteProgram(program);
	glDeleteVertexArrays(1, model);
	glDeleteBuffers(1, model + 1);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);

	return 0;
}
int check_GL_error(const char *msg){
	GLenum err = glGetError();
	if (err != GL_NO_ERROR){
		fprintf(stderr, "OpenGL Error: %s - %s\n", gluErrorString(err), msg);
		return 1;
	}
	return 0;
}
char* read_file(const char *fname){
	FILE *fp = fopen(fname, "rb");
	if (!fp){
		fprintf(stderr, "Failed to open file: %s\n", fname);
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	size_t sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *content = malloc(sizeof(char) * sz + sizeof(char));
	if (!content){
		fprintf(stderr, "Failed to allocate room for file content\n");
		return NULL;
	}
	if (fread(content, sizeof(char), sz, fp) != sz){
		fprintf(stderr, "Failed to read all bytes, expected %u\n", sz);
		free(content);
		return NULL;
	}
	content[sz] = '\0';
	return content;
}
GLint make_shader(const char *src, GLenum type){
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, 0);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE){
		switch (type){
		case GL_VERTEX_SHADER:
			fprintf(stderr, "Vertex shader ");
			break;
		case GL_FRAGMENT_SHADER:
			fprintf(stderr, "Fragment shader ");
			break;
		default:
			fprintf(stderr, "Other shader ");
		}
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		char *log = malloc(sizeof(char) * len);
		glGetShaderInfoLog(shader, len, 0, log);
		fprintf(stderr, " failed to compile. Log:\n %s\n", log);

		free(log);
		glDeleteShader(shader);
		return -1;
	}
	return shader;
}
GLint make_program(GLuint vert, GLuint frag){
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == GL_FALSE){
		GLint len;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		char *log = malloc(sizeof(char) * len);
		glGetProgramInfoLog(prog, len, 0, log);
		fprintf(stderr, "Program failed to link, log:\n%s", log);
		free(log);
	}
	glDetachShader(prog, vert);
	glDetachShader(prog, frag);
	if (status == GL_FALSE){
		glDeleteProgram(prog);
		return -1;
	}
	return prog;
}
#ifdef _WIN32
void APIENTRY gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei len, const GLchar *msg, GLvoid *usr)
#else
void gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei len, const GLchar *msg, GLvoid *usr)
#endif
{
	fprintf(stderr, "OpenGL Debug Msg: ");
	switch (severity){
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		fprintf(stderr, "High severity ");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		fprintf(stderr, "Medium severity ");
		break;
	case GL_DEBUG_SEVERITY_LOW_ARB:
		fprintf(stderr, "Low severity ");
	}
	switch (src){
	case GL_DEBUG_SOURCE_API_ARB:
		fprintf(stderr, "API ");
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		fprintf(stderr, "Window system ");
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		fprintf(stderr, "Shader compiler ");
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		fprintf(stderr, "Third party ");
		break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		fprintf(stderr, "Application ");
		break;
	default:
		fprintf(stderr, "Other ");
	}
	switch (type){
	case GL_DEBUG_TYPE_ERROR_ARB:
		fprintf(stderr, "Error ");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		fprintf(stderr, "Deprecated behavior ");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		fprintf(stderr, "Undefined behavior ");
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		fprintf(stderr, "Portability ");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		fprintf(stderr, "Performance ");
		break;
	default:
		fprintf(stderr, "Other ");
	}
	fprintf(stderr, ":\n\t%s\n", msg);
}

