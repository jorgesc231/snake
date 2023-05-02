// Todo el trabajo de renderizar

#include <stdio.h>
#include "renderer.h"

#include "assets_loader.h"

void init_camera_2d(Camera *camera, float width, float height, glm::vec2 camera_pos)
{
    camera->position = glm::vec3(camera_pos, 0.0f);
    
    // Our camera is going to be a stationary camera that will always be looking toward the center of the world coordinates; 
    // the up vector will always be pointing toward the positive y-axis.
    camera->view = glm::translate(glm::mat4(1.0f), camera->position);
    camera->projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
}


void init_camera_3d(Camera *camera, float FOV, float width, float height, float nearPlane, float farPlane, glm::vec3 camera_pos)
{
    camera->position = camera_pos;
    
    glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    camera->view = glm::lookAt(camera->position, camera_front, camera_up);
    
    camera->projection = glm::perspective(FOV, width / height, nearPlane, farPlane);
}


Vertex *create_color_quad(Vertex *target, glm::vec2 position, glm::vec3 color)
{
    float size = 1.0f;  // Quad de 1 x 1

    AtlasSprite sprite = DescAtlas[NO_TEXTURE];

    target->x = position.x;
    target->y = position.y;

    target->r = color.r;
    target->g = color.g;
    target->b = color.b;
    target->a = 255;

    target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target++;

    // Segundo vertice
    target->x = position.x + size;
    target->y = position.y;

    target->r = color.r;
    target->g = color.g;
    target->b = color.b;
    target->a = 255;

    target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target++;

    // Tercer vertice
    target->x = position.x + size;
    target->y = position.y + size;

    target->r = color.r;
    target->g = color.g;
    target->b = color.b;
    target->a = 255;

    target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target++;

    // Cuarto vertice
    target->x = position.x;
    target->y = position.y + size;

    target->r = color.r;
    target->g = color.g;
    target->b = color.b;
    target->a = 255;

    target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target++;


    return target;
}

Vertex *create_texture_quad(Vertex* target, float x, float y, SPRITE_ID sprite_id)
{
    float size = 1.0f;  // Quad de 1 x 1
    
    AtlasSprite sprite = DescAtlas[sprite_id];

    target->x = x;
    target->y = y;
    
    target->r = 255;
    target->g = 255;
    target->b = 255;
    target->a = 255;
    
    //target->u = 0.0f;
    //target->v = 0.0f;
    //target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    //target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target++;
    
    // Segundo vertice
    target->x = x + size;
    target->y = y;
    
    target->r = 255;
    target->g = 255;
    target->b = 255;
    target->a = 255;
    
    //target->u = 1.0f;
    //target->v = 0.0f;
    //target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    //target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target++;
    
    // Tercer vertice
    target->x = x + size;
    target->y = y + size;
    
    target->r = 255;
    target->g = 255;
    target->b = 255;
    target->a = 255;
    
    //target->u = 1.0f;
    //target->v = 1.0f;
    //target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    //target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target++;
    
    // Cuarto vertice
    target->x = x;
    target->y = y + size;
    
    target->r = 255;
    target->g = 255;
    target->b = 255;
    target->a = 255;
    
    //target->u = 0.0f;
    //target->v = 1.0f;
    //target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    //target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target++;
    
    
    return target;
}



int init_batch_renderer(batch_renderer *renderer, char* path_buffer, char* base_path)
{
    // FIX: Esto esta mal...
    char temporal_path_buffer[PATH_MAX];

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_BLEND);

    // TODO: Esto no deberia estar aqui!!!!
    // Load Texture and Shaders
    renderer->textures[0] = texture_load(data_path(path_buffer, base_path, "graphics/snake_atlas.png"));
    renderer->shaders[TEXTURE_SHADER] = shaderProgLoad(data_path(path_buffer, base_path, "shaders/texture.vertex"), data_path(temporal_path_buffer, base_path, "shaders/texture.fragment"));
    //renderer->textures[0] = texture_load("../assets/graphics/snake_atlas.png");
    //renderer->shaders[TEXTURE_SHADER] = shaderProgLoad("../assets/shaders/texture.vertex", "../assets/shaders/texture.fragment");

    assert(renderer->textures[0]);
    assert(renderer->shaders[TEXTURE_SHADER]);

    glUseProgram(renderer->shaders[TEXTURE_SHADER]);

    glBindTexture(GL_TEXTURE_2D, renderer->textures[0]);
    
    GLint texSamplerUniformLoc = glGetUniformLocation(renderer->shaders[TEXTURE_SHADER], "texSampler");
    if (texSamplerUniformLoc < 0) {
        SDL_Log("ERROR: No se pudo obtener la ubicacion de texSampler\n");
    }
    
    glUniform1i(texSamplerUniformLoc, 0);

    

    // Sirve para saber el tamano del vertex buffer e index buffer de antemano
    const size_t MaxQuadCount = 1000;   // Cantidad maxima de quad que quiero dibujar por drawcall
    const size_t MaxVertexCount = MaxQuadCount * 4;
    const size_t MaxIndexCount = MaxQuadCount * 6;
    
    // Crea los indices de forma automatica (GLES 2.0 solo soporta shorts...)
    GLushort indices[MaxIndexCount];
    GLushort offset = 0;
    
    for (size_t i = 0; i < MaxIndexCount; i += 6)
    {
        indices[i + 0] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 2 + offset;
        
        indices[i + 3] = 2 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 0 + offset;
        
        offset += 4;
    }


    // Inicializa un vertex buffer dinamico
    glGenBuffers(1, &renderer->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

    GLint posLoc = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_pos");
    GLint colorLoc = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_color");
    GLint texLoc = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_texCoord");

    glEnableVertexAttribArray(posLoc);
    glEnableVertexAttribArray(colorLoc);
    glEnableVertexAttribArray(texLoc);
    
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    

    // Inicializa el index buffer
    glGenBuffers(1, &renderer->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndexCount * sizeof(unsigned short), indices, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);


    // TODO: deberia validar errores
    return 1;
}


void print_gles_errors()
{

    GLuint err = glGetError();
    if (err != GL_NO_ERROR) {
        // Failed
        printf("ERROR, code %u\n", err);
        //assert(false);
    }
}

#if 0
Vertex *create_quad(Vertex *target, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color);
#endif

#if 0
//glUseProgram(renderer->shaders[TEXTURE_SHADER]);


// prepare transformations
glm::mat4 model = glm::mat4(1.0f);

model = glm::translate(model, glm::vec3(position, 0.0f));
model = glm::translate(model, glm::vec3(0.5*size.x, 0.5*size.y, 0.0));
model = glm::rotate(model, glm::radians(rotate),
                    glm::vec3(0.0, 0.0, 1.0));
model = glm::translate(model, glm::vec3(-0.5*size.x, -0.5*size.y, 0.0));
model = glm::scale(model, glm::vec3(size, 1.0f));

//shader.SetMatrix4("model", model);
//shader.SetVector3f("spriteColor", color);

glActiveTexture(GL_TEXTURE0);
//glBindTexture(GL_TEXTURE_2D, state->textures[0]);


//glBindVertexArray(quadVAO);
//glDrawArrays(GL_TRIANGLES, 0, 6);
//glBindVertexArray(0);
#endif

#if 0

// Renderizar
static int setup = 0;

if (!setup) {
    setup = 1;
    
    
    glUseProgram(state.shaders[TEXTURE_SHADER]);
    
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindTexture(GL_TEXTURE_2D, state.textures[0]);
    
    GLint texSamplerUniformLoc = glGetUniformLocation(state.shaders[TEXTURE_SHADER], "texSampler");
    if (texSamplerUniformLoc < 0) {
        SDL_Log("ERROR: No se pudo obtener la ubicacion de texSampler\n");
    }
    
    glUniform1i(texSamplerUniformLoc, 0);
    
    glClearColor(0.392156863f, 0.584313725f, 0.929411765f, 1.f);
    
    assert(glGetError() == GL_NO_ERROR);
}

Vertex vtx[] =
{
    {.x = -0.5f, .y = -0.5, .r = 255, .u = 0.0f, .v = 0.0f},
    {.x = 0.5f, .y = -0.5f, .g = 255, .u = 1.0f, .v = 0.0f},
    {.x = 0.5f, .y = 0.5f, .b = 255, .u = 1.0f, .v = 1.0f},
    
    {.x = 0.5f, .y = 0.5f, .r = 255, .u = 1.0f, .v = 1.0f},
    {.x = -0.5f, .y = 0.5f, .r = 255, .u = 0.0f, .v = 1.0f},
    {.x = -0.5f, .y = -0.5f, .r = 255, .u = 0.0f, .v = 0.0f},
};

glClear(GL_COLOR_BUFFER_BIT);
glBufferData(GL_ARRAY_BUFFER, sizeof(vtx), vtx, GL_STATIC_DRAW);

glDrawArrays(GL_TRIANGLES, 0, 6);

assert(glGetError() == GL_NO_ERROR); 

// Update the window

SDL_GL_SwapWindow(window);


// Wait for the user to quit

bool quit = false;

while (!quit)
{
    SDL_Event event;
    
    if (SDL_WaitEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        
        // Salir ante cualquier pulsacion de tecla!
        if (event.type == SDL_KEYDOWN) {
            quit = true;
        }
    }
}

#endif
