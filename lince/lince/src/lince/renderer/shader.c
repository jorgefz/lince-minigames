#include "core/profiler.h"
#include "core/memory.h"
#include "core/fileio.h"
#include "renderer/shader.h"
#include <string.h>
#include <glad/glad.h>
//#include <cglm>




/* --- Static declarations --- */

/* Compiles a shader file from source, returns OpenGL ID */
static int LinceCompileShader(const char* source, int type);


/* --- Public API --- */

/* Create shader from paths to vertex and fragment shader source files */
LinceShader* LinceCreateShader(
	const char* vertex_path,
	const char* fragment_path
){
	LINCE_PROFILER_START(timer);
	LINCE_INFO("Creating shader from file");
	LINCE_INFO(" ---> Vertex filepath:   '%s'", vertex_path);
	LINCE_INFO(" ---> Fragment filepath: '%s'", fragment_path);
	LinceShader* shader;
	char *vsrc, *fsrc;

	vsrc = LinceLoadTextFile(vertex_path);
	fsrc = LinceLoadTextFile(fragment_path);
	shader = LinceCreateShaderFromSrc(vsrc, fsrc);

	LinceFree(vsrc);
	LinceFree(fsrc);

	LINCE_PROFILER_END(timer);
	return shader;
}

/* Create shader from source code of vertex and fragment shaders */
LinceShader* LinceCreateShaderFromSrc(
	const char* vertex_src,
	const char* fragment_src
){
	LINCE_PROFILER_START(timer);
	LINCE_INFO("Creating shader from source");

	LinceShader* shader = LinceCalloc(sizeof(LinceShader));
	shader->id = glCreateProgram();
	LINCE_INFO(" ---> ID: %d", shader->id);

	LINCE_INFO(" ---> Compiling ...");
	int vs, fs;
	vs = LinceCompileShader(vertex_src, GL_VERTEX_SHADER);
	fs = LinceCompileShader(fragment_src, GL_FRAGMENT_SHADER);

	LINCE_INFO(" ---> Linking and validating ...");
	glAttachShader(shader->id, vs);
	glAttachShader(shader->id, fs);
	glLinkProgram(shader->id);
	glValidateProgram(shader->id);
	
	// Compiled shader files are no longer necessary
	glDeleteShader(vs);
	glDeleteShader(fs);

	// Start out with hashmap of 21 buckets to avoid costs of
	// Resizing often at small sizes (e.g. at sizes 2, 3, 5, 7, 11, etc).
	int ret = hashmap_init(&shader->uniforms, 20);
	LINCE_ASSERT(ret == 0,
		"Failed to create hashmap for shader %d uniforms", shader->id);

	LINCE_INFO(" ---> Done!")

	LINCE_PROFILER_END(timer);
    return shader;
}

void LinceBindShader(LinceShader* shader){
	glUseProgram(shader->id);
}

void LinceUnbindShader(void){
	glUseProgram(0);
}


/* Destroys and deallocates given shader */
void LinceDeleteShader(LinceShader* shader){
	if(!shader) return;
	LINCE_INFO("Deleting shader with ID %d", shader->id);
	if(shader->id > 0) glDeleteProgram(shader->id);
	hashmap_uninit(&shader->uniforms);
	LinceFree(shader);
}

int LinceGetShaderUniformID(LinceShader* shader, const char* name){
	LINCE_PROFILER_START(timer);

	if(!shader || !name) return -1;

	/*
	Uniform locations are saved as void* addresses in the hashmap
	to avoid allocating memory and freein it afterwards.
	The void* type should be 64 bits long (in 64bit systems).
	*/

	long location;
	if(hashmap_has_key(&shader->uniforms, name)){
		location = (long)hashmap_get(&shader->uniforms, name);
		LINCE_PROFILER_END(timer);
		return (int)location;
	}
	
	location = glGetUniformLocation(shader->id, name);
	hashmap_set(&shader->uniforms, name, (void*)location);
	
	// Implementation without hashmap - slower?
	// int location =  glGetUniformLocation(shader->id, name);
	
	if(location < 0){
		LINCE_INFO("Uniform '%s' not found in shader %d",
			name, shader->id);
	}
	
	LINCE_PROFILER_END(timer);
	return (int)location;
}

/* Set integer uniform */
void LinceSetShaderUniformInt(LinceShader* sh, const char* name, int val){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniform1i(loc, val);
}

/* Set integer array uniform */
void LinceSetShaderUniformIntN(
	LinceShader* sh, const char* name,
	int* arr, uint32_t count
) {
	int loc = LinceGetShaderUniformID(sh, name);
	glUniform1iv(loc, count, arr);
}

/* Set float uniform */
void LinceSetShaderUniformFloat(LinceShader* sh, const char* name, float val){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniform1f(loc, val);
}

/* Set vec2 uniform */
void LinceSetShaderUniformVec2(LinceShader* sh, const char* name, vec2 v){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniform2f(loc, v[0], v[1]);
}

/* Set vec3 uniform */
void LinceSetShaderUniformVec3(LinceShader* sh, const char* name, vec3 v){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniform3f(loc, v[0], v[1], v[2]);
}

/* Set vec4 uniform */
void LinceSetShaderUniformVec4(LinceShader* sh, const char* name, vec4 v){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniform4f(loc, v[0], v[1], v[2], v[3]);
}

/* Set mat3 uniform */
void LinceSetShaderUniformMat3(LinceShader* sh, const char* name, mat3 m){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

/* Set mat4 uniform */
void LinceSetShaderUniformMat4(LinceShader* sh, const char* name, mat4 m){
	int loc = LinceGetShaderUniformID(sh, name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}


/* --- Static functions --- */


int LinceCompileShader(const char* source, int type){
	LINCE_PROFILER_START(timer);
	
	int compile_sucess = LinceFalse;
	int id;

	id = glCreateShader(type);
	glShaderSource(id, 1, &source, NULL);
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &compile_sucess);

	if (compile_sucess != GL_TRUE) {
		// Retrieve GLSL compiler error message
		int length = 0;
		char msg[LINCE_TEXT_MAX] = {0};
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		glGetShaderInfoLog(id, 1000, &length, &msg[0]);
		glDeleteShader(id);
		LINCE_ERROR("Failed to compile shader source");
		LINCE_ASSERT(0, "%s", msg);
	}
	
	LINCE_PROFILER_END(timer);
	return id;
}