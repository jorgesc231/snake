#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>

#include "assets_loader.h"

enum _shaders {
    TEXTURE_SHADER,
    TEXT_SHADER,
    SHADER_COUNTER
};

typedef struct Vertex_s {
	float x, y;	// position
	uint8_t r, g, b, a;
	float u, v;	// texCoord
} Vertex;

struct Camera {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
};


struct batch_renderer {
    GLuint textures[8];
    GLuint shaders[SHADER_COUNTER];

    GLuint vertex_buffer;
    GLuint index_buffer;
};

struct batch {
    GLuint vertex_buffer;
    GLuint index_buffer;
};


int init_batch_renderer(batch_renderer *renderer, char* path_buffer, char* base_path);

void init_camera_2d(Camera *camera, float width, float height, glm::vec2 camera_pos);
void init_camera_3d(Camera *camera, float FOV, float width, float height, float nearPlane, float farPlane, glm::vec3 camera_pos);

// Funcion que agrega 4 vertices al buffer target para hacer un nuevo Quad
// Retorna la posicion en el buffer en el que esta
//Vertex *create_quad(Vertex *target, float x, float y, int textureID);
Vertex *create_texture_quad(Vertex* target, float x, float y, SPRITE_ID sprite_id);
Vertex *create_color_quad(Vertex *target, glm::vec2 position, glm::vec3 color);

void print_gles_errors();