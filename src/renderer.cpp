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
    
    target->r = 1.0f;
    target->g = 1.0f;
    target->b = 1.0f;
    target->a = 1.0f;
    
    target->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target++;
    
    // Segundo vertice
    target->x = x + size;
    target->y = y;
    
    target->r = 1.0f;
    target->g = 1.0f;
    target->b = 1.0f;
    target->a = 1.0f;
    
    target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    target++;
    
    // Tercer vertice
    target->x = x + size;
    target->y = y + size;
    
    target->r = 1.0f;
    target->g = 1.0f;
    target->b = 1.0f;
    target->a = 1.0f;
    
    target->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    target->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    target++;
    
    // Cuarto vertice
    target->x = x;
    target->y = y + size;
    
    target->r = 1.0f;
    target->g = 1.0f;
    target->b = 1.0f;
    target->a = 1.0f;
    
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
    //renderer->shaders[TEXT_SHADER] = shaderProgLoad(data_path(path_buffer, base_path, "shaders/text.vertex"), data_path(temporal_path_buffer, base_path, "shaders/text.fragment"));

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


    init_main_shader_attribs(renderer);
    

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


// TODO: Podria replicar el comportamiento de los VAO usando funciones...
void init_main_shader_attribs(batch_renderer *renderer) {
    // TODO: Esto podria almacenarlo en alguna parte para no tener que llamarlo cada frame
    GLint posLoc = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_pos");
    GLint colorLoc = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_color");
    GLint texLoc = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_texCoord");


    // NOTA: Este estado se eliminan cuando se usa glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(posLoc);
    glEnableVertexAttribArray(colorLoc);
    glEnableVertexAttribArray(texLoc);
    
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
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