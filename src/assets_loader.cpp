// Carga de shaders, texturas, sonidos y fuentes de texto.

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengles2.h>

#ifdef NDEBUG
// TODO: Deberia cambiar dependiendo de la plataforma...
#define ASSET_FOLDER "%s../assets/%s"
#else
// Release
#define ASSET_FOLDER "%sassets/%s"
#endif

// NOTE: el working dir en raspberry no es el que deberia
// Construye la ruta hacia los assets basado en la ubicacion del ejecutable:
char* data_path(char *path_buffer, char* base_path, const char* asset_path) 
{
	snprintf(path_buffer, PATH_MAX, ASSET_FOLDER, base_path, asset_path);
    return path_buffer;
}


static size_t get_file_length(FILE *file)
{
    size_t length;
    
    size_t current_pos = ftell(file);
    
    fseek(file, 0, SEEK_END);
    
    length = ftell(file);
    
    // Return the file to its previous position
    fseek(file, current_pos, SEEK_SET);
    
    return length;
}


static GLuint shader_load(const char *filename, GLenum shader_type)
{
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        SDL_Log("Can't open file: %s\n", filename);
        
        return 0;
    }
    
    size_t length = get_file_length(file);
    
    // Alloc space for the file (plus '\0' termination)
    
    GLchar* shader_src = (GLchar*)calloc(length + 1, 1);
    
    if (!shader_src) {
        
		SDL_Log("Out of memory when reading file: %s\n", filename);
		fclose(file);
		file = NULL;
        
		return 0;
	}
    
    fread(shader_src, 1, length, file);
    
    // Done with the file
	fclose(file);
	file = NULL;
    
    
    // Create the shader
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, (const GLchar**)&shader_src, NULL);
	free(shader_src);
	shader_src = NULL;
    
    
    // Compile it
	glCompileShader(shader);
	GLint compileSucceeded = GL_FALSE;
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSucceeded);
    
    if (!compileSucceeded) {
        
		// Compilation failed. Print error info
		SDL_Log("Compilation of shader: %s failed\n", filename);
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
		GLchar* errLog = (GLchar*)malloc(logLength);
        
		if (errLog) {
			glGetShaderInfoLog(shader, logLength, &logLength, errLog);
			SDL_Log("%s\n", errLog);
			free(errLog);
		}
		else {
			SDL_Log("Couldn't get shader log; out of memory\n");
		}
        
		glDeleteShader(shader);
		shader = 0;
	}
    
	return shader;
}


/*
* Destroys a shader.
*/
static void shaderDestroy(GLuint shaderID) {
	glDeleteShader(shaderID);
}


GLuint shaderProgLoad(const char* vertFilename, const char* fragFilename) {
	
	GLuint vertShader = shader_load(vertFilename, GL_VERTEX_SHADER);
    
	if (!vertShader) {
		SDL_Log("Couldn't load vertex shader: %s\n", vertFilename);
		return 0;
	}
    
	GLuint fragShader = shader_load(fragFilename, GL_FRAGMENT_SHADER);
    
	if (!fragShader) {
		SDL_Log("Couldn't load fragment shader: %s\n", fragFilename);
		shaderDestroy(vertShader);
		vertShader = 0;
        
		return 0;
	}
    
	GLuint shaderProg = glCreateProgram();
    
	if (shaderProg) {
		glAttachShader(shaderProg, vertShader);
		glAttachShader(shaderProg, fragShader);
        
		glLinkProgram(shaderProg);
        
		GLint linkingSucceeded = GL_FALSE;
		glGetProgramiv(shaderProg, GL_LINK_STATUS, &linkingSucceeded);
        
		if (!linkingSucceeded) {
			SDL_Log("Linking shader failed (vert. shader: %s, frag. shader: %s\n)", vertFilename, fragFilename);
            
			GLint logLength = 0;
			glGetProgramiv(shaderProg, GL_INFO_LOG_LENGTH, &logLength);
            
			GLchar* errLog = (GLchar*)malloc(logLength);
            
			if (errLog) {
				glGetProgramInfoLog(shaderProg, logLength, &logLength, errLog);
				SDL_Log("%s\n", errLog);
				free(errLog);
			}
			else {
				SDL_Log("Couldn't get shader link log; out of memory\n");
			}
            
			glDeleteProgram(shaderProg);
			shaderProg = 0;
		}
	}
    
	else {
		SDL_Log("Couldn't create shader program\n");
	}
    
	// Don't need these any more
    
	shaderDestroy(vertShader);
	shaderDestroy(fragShader);
    
	return shaderProg;
}

void shaderProgDestroy(GLuint shaderProg)
{
	glDeleteProgram(shaderProg);
}



// Texture loading


GLuint texture_load(const char* filename)
{

// Ya esta inicializado
#if 0
// Windows: Los dlls de los formatos de imagenes de SDL_Image tienen que estar en la carpeta root 
    int flags = IMG_INIT_PNG;
    

    if ((IMG_Init(flags) & flags) == 0)
    {
        // Failed
        SDL_Log("ERROR: Texture loading failed. Could't get JPEG and PNG loaders.\n");
        return 0;
    }
#endif
    
    // Load the image
    SDL_Surface* texSurf = IMG_Load(filename);
    
    if (!texSurf)
    {
        SDL_Log("Loading image %s failed with error: %s", filename, IMG_GetError());
        return 0;
    }
    
    // Determine the format
    // NOTE: Only supporting 24 and 32-bit images
    
    GLenum format;
    GLenum type = GL_UNSIGNED_BYTE;
    switch (texSurf->format->BytesPerPixel)
    {
        case 3:
        	format = GL_RGB;
        break;
        case 4:
        	format = GL_RGBA;
        break;
        default:
			SDL_Log("Can't load image %s; it isn't a 24/32-bit image\n", filename);
			SDL_FreeSurface(texSurf);
			texSurf = NULL;
			return 0;
    }
    
    // Create the texture
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, texSurf->w, texSurf->h, 0, format, type, texSurf->pixels);
    
    // Set up the filtering
    // NOTE: Failure to do this may result in no texture
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        // Failed
        glDeleteBuffers(1, &texture);
        texture = 0;
        SDL_FreeSurface(texSurf);
        texSurf = NULL;
        SDL_Log("Creating texture %s failed, code %u\n", filename, err);
        //		__debugbreak();
        return 0;
    }
    
    // Cleanup
    SDL_FreeSurface(texSurf);
    texSurf = NULL;
    
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void texture_destroy(GLuint texName)
{
    glDeleteTextures(1, &texName);
}



// Load Audio





#if 0
// Otra forma de compilar el shader.
static GLuint compile_shader(GLuint type, const char* glsl)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &glsl, NULL);
	glCompileShader(shader);
    
	GLint compiled;
    
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    
	if (!compiled) {
		char message[4096];
		glGetShaderInfoLog(shader, sizeof(message), NULL, message);
		printf("Error compiling %s shader!\n%s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", message); 
		assert(0); 
	}
    
	return shader;
}

// Uso
GLuint vsh = compile_shader(GL_VERTEX_SHADER, vertex_glsl);
GLuint fsh = compile_shader(GL_FRAGMENT_SHADER, fragment_glsl);

GLuint program = glCreateProgram();
glAttachShader(program, fsh);
glAttachShader(program, vsh);
glBindAttribLocation(program, 0, "a_pos");
glBindAttribLocation(program, 1, "a_color");
glBindAttribLocation(program, 2, "a_texCoord");
glLinkProgram(program);
glDeleteShader(fsh);
glDeleteShader(vsh);

GLint linked;
glGetProgramiv(program , GL_LINK_STATUS, &linked);

if (!linked) {
    char message[4096];
    glGetProgramInfoLog(program, sizeof(message), NULL, message);
    printf ("Error linking shader!\n%s\n", message);
    assert(0);
}
#endif
