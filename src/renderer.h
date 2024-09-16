#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_image.h>

#include "assets_loader.h"
#include <imgui.h>

#define PHONE_MAIN_FONT_SIZE 64.0f
#define PHONE_LARGE_FONT_SIZE 84.0f
#define PHONE_TITLE_FONT_SIZE 182.0f
#define PHONE_SCORE_BAR_SIZE 148.0f

#define PC_MAIN_FONT_SIZE 28.0f
#define PC_LARGE_FONT_SIZE 32.0f
#define PC_TITLE_FONT_SIZE 144.0f
#define PC_SCORE_BAR_SIZE 60.0f


// NOTE: To debug IMGUI OPENGL
//#define IMGUI_IMPL_OPENGL_DEBUG

#define OPENGL_ERROR { GLenum err = glGetError(); IM_ASSERT(err == GL_NO_ERROR); }

// Colores
#define DEFAULT_CELL_COLOR1 glm::vec4(0.67f, 0.84f, 0.32f, 1.0f)
#define DEFAULT_CELL_COLOR2 glm::vec4(0.64f, 0.82f, 0.29f, 1.0f)

enum DEVICE_TYPE {
    DESKTOP,
    PHONE,
};

enum _SHADERS {
    TEXTURE_SHADER,
    TEXT_SHADER,
    SHADER_COUNTER
};

typedef struct Vertex_s {
    float x, y;            // position
    float r, g, b, a;      // color
    float u, v;            // texCoord
} Vertex;

typedef struct {
    int32_t x, y;
} simple_vertex;

// Quad se usa para renderizar elementos del juego
// Rect para renderizar elementos de de la UI
// Son solo distintos nombres.
typedef struct Quad_s {
    int32_t x, y;  // Position
    int32_t width, height;
} Quad, Rect;

typedef struct {
	simple_vertex a, b, c;
} Triangle;

struct Camera {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
};

struct Batch {
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLushort index_count;
    GLuint vertex_count;
    Vertex vertices[4000];
    Vertex* vertices_ptr;
};

struct Renderer {
    SDL_Window* window = NULL;
    SDL_GLContext context = NULL;

    SDL_DisplayOrientation disp_orientation;
    // Resolucion por defecto... Aspect Ratio por defecto...
    int32_t DISP_WIDTH = 1024;
    int32_t DISP_HEIGHT = 576;
    float dpi = 0;

    // TODO: Temporal (Tiene que haber una forma menos estupida...)
	#if defined(__ANDROID__)
    DEVICE_TYPE device_type = PHONE;
    int scale_factor = 2;
    #else
    DEVICE_TYPE device_type = DESKTOP;
    int scale_factor = 1;
	#endif

    Camera camera;
    GLuint textures[1];
    GLuint shaders[SHADER_COUNTER];

    // Uniforms location
    GLint  AttribLocationProjMtx;  // Only have projection matrix in 2D
    GLint  AttribLocationVtxPos;    // Vertex attributes location
    GLint  AttribLocationVtxUV;
    GLint  AttribLocationVtxColor;

    int32_t attribs_enabled = false;

    Batch main_batch;
#if defined(NDEBUG)
    ImFont* font_debug = NULL;
#endif
    ImFont* font_main = NULL;
    ImFont* font_large = NULL;
    ImFont* font_title = NULL;
    float font_main_size;
    float font_large_size;
    float font_title_size;
    
    glm::vec4 clear_color = glm::vec4(0.34f, 0.54f, 0.20f, 1.0f);
    glm::vec4 cell_color1 = DEFAULT_CELL_COLOR1;
    glm::vec4 cell_color2 = DEFAULT_CELL_COLOR2;
};


uint32_t init_renderer(Renderer *renderer);
void shutdown_renderer(Renderer *renderer);

void init_camera_2d(Camera *camera, float width, float height, glm::vec2 camera_pos);

void begin_batch(Batch *batch);
void draw_batch(Renderer renderer, Batch batch);

void create_quad(Batch *batch, Quad quad, SPRITE_ID sprite_id, glm::vec4 color);
void create_color_triangle(Batch *batch, Triangle triangle, glm::vec4 color);

void print_gles_errors();

void init_main_shader_attribs(Renderer *renderer);

float sign (simple_vertex p1, simple_vertex p2, simple_vertex p3);
bool is_over_triangle (Triangle triangle, int x, int y);

#endif
