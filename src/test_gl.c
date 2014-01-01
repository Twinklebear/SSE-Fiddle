#include <stdio.h>
#include <GL/glew.h>
#include <SDL.h>
#include "vec4.h"
#include "mat4.h"

const int WIN_WIDTH = 640;
const int WIN_HEIGHT = 480;

const char *vert_shader_src =
"#version 330 core\n\
layout(location = 0) in vec4 pos;\n\
void main(void){ gl_Position = pos; };";

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

int main(int argc, char **argv){
	/* vertices for a triangle */
	vec4_t triangle[3];
	triangle[0] = vec4_new(0, 0, 0, 1);
	triangle[1] = vec4_new(1, 0, 0, 1);
	triangle[2] = vec4_new(1, 1, 0, 1);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

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

	glClearColor(0, 0, 0, 1);
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);

	//Model's vao and vbo
	GLuint model[2];
	glGenVertexArrays(1, model);
	glBindVertexArray(model[0]);
	glGenBuffers(1, model + 1);
	glBindBuffer(GL_ARRAY_BUFFER, model[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), triangle, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

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

