// Todo el trabajo de renderizar

#include "renderer.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "snake.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

uint32_t init_renderer(Renderer *renderer)
{
    // Init the random number generator
    // TODO: Change to a better random number generator
    srand(time(0));
    
    bool success = false;

    // NOTE: Necesario para que SDL2 funcione con ANGLE
    #if !defined(__EMSCRIPTEN__) || !defined(_RPI1) || !defined(__ANDROID__)
    SDL_SetHint("SDL_OPENGL_ES_DRIVER", "1");
    //SDL_SetHintWithPriority("SDL_OPENGL_ES_DRIVER", "1", SDL_HINT_OVERRIDE);
    #endif

    // Block to portrait mode on Android
    // TODO: Finish Android landscape implementation
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");

    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        SDL_LogCritical(CATEGORY_ENGINE_SNAKE, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_DisplayMode mode;

    if (SDL_GetDesktopDisplayMode(0, &mode) < 0)
    {
        SDL_LogCritical(CATEGORY_ENGINE_SNAKE, "Display Mode could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "Display Mode: %dx%d %dhz", mode.w, mode.h, mode.refresh_rate);

    if (SDL_GetCurrentDisplayMode(0, &mode) < 0)
    {
        SDL_LogCritical(CATEGORY_ENGINE_SNAKE, "Current Display Mode could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "Current Display Mode: %dx%d %dhz", mode.w, mode.h, mode.refresh_rate);

#if defined(__EMSCRIPTEN__)
    // Get display resolution and device type (Phone or desktop) for emscripten
    renderer->DISP_WIDTH = EM_ASM_INT(return window.innerWidth;);
    renderer->DISP_HEIGHT = EM_ASM_INT(return window.innerHeight;);

    renderer->device_type = (DEVICE_TYPE) EM_ASM_INT(return device_type;);
#endif

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    

    // Request OpenGL ES 2.0 context on Raspberry Pi 1 (ANGLE provides 3.0 context anyway)
#if defined(_RPI1)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Want double-buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Create the window
#if defined(__ANDROID__) || defined(_RPI1)
    // NOTE: Allow window resize (important for Android)
    int32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN;
#else
    int32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
#endif

    renderer->window = SDL_CreateWindow("Snake2D", SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED, renderer->DISP_WIDTH, renderer->DISP_HEIGHT, window_flags);

    if (!renderer->window)
    {
        SDL_LogCritical(CATEGORY_ENGINE_SNAKE, "[SDL] Could't create the main window.");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could't create the main window.", NULL);
        return EXIT_FAILURE;
    }

    // Get the provided window size
    SDL_GetWindowSize(renderer->window, &renderer->DISP_WIDTH, &renderer->DISP_HEIGHT);

    // Create OpenGL context
    renderer->context = SDL_GL_CreateContext(renderer->window);
    
    if (!renderer->context)
    {
        SDL_LogCritical(CATEGORY_ENGINE_SNAKE, "[SDL] GL context creation failed!");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could't create the OpenGL context.", NULL);
        return EXIT_FAILURE;
    }

    SDL_GL_MakeCurrent(renderer->window, renderer->context);

    // TODO: Adaptar a los DPI de la plataforma.
    SDL_GetDisplayDPI(0, &renderer->dpi, 0, 0);

    int drawable_width, drawable_height;
    SDL_GL_GetDrawableSize(renderer->window, &drawable_width, &drawable_height);

    SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "Window Size: (%d, %d) Drawable Area Size: (%d, %d) DPI: %f", renderer->DISP_WIDTH, renderer->DISP_WIDTH, drawable_width, drawable_height, renderer->dpi);

    // NOTE: No VSync with EMSCRIPTEN
	#if !defined(__EMSCRIPTEN__)
    // -1 for adaptive vsync
    // If an application requests adaptive vsync and the system does not support it, this function will fail and return -1.
    if (SDL_GL_SetSwapInterval(-1) == -1)
    {
        //In such a case, you should probably retry the call with 1 for the interval.
        SDL_LogWarn(CATEGORY_ENGINE_SNAKE, "adaptive vsync not supported, using normal vsync...");
        SDL_GL_SetSwapInterval(1);
    }
    #endif

#if defined(_RPI1)
    SDL_ShowCursor(SDL_DISABLE);
#endif

    //SDL_SetWindowResizable(renderer->window, SDL_TRUE);
    SDL_SetWindowMinimumSize(renderer->window, 960, 540);

    glViewport(0, 0, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);

    renderer->disp_orientation = SDL_GetDisplayOrientation(0);

    //
    // Show version number of stuff
    //

    SDL_version sdl_version;
    SDL_GetVersion(&sdl_version);

    const SDL_version *image_version = IMG_Linked_Version();
    const SDL_version *mixer_version = Mix_Linked_Version();
    //const SDL_version *ttf_version = TTF_Linked_Version();

    SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "SDL_VERSION = %d.%d.%d\n", sdl_version.major, sdl_version.minor, sdl_version.patch);
    SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "SDL_IMAGE_VERSION = %d.%d.%d\n", image_version->major, image_version->minor, image_version->patch);
    SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "SDL_MIXER_VERSION = %d.%d.%d\n", mixer_version->major, mixer_version->minor, mixer_version->patch);
    SDL_LogInfo(CATEGORY_ENGINE_SNAKE,"GL_VENDOR = %s\n",  glGetString(GL_VENDOR));
    SDL_LogInfo(CATEGORY_ENGINE_SNAKE,"GL_RENDERER = %s\n",  glGetString(GL_RENDERER));
    SDL_LogInfo(CATEGORY_ENGINE_SNAKE,"GL_VERSION = %s\n",  glGetString(GL_VERSION));

    const char* glsl_version = "#version 100";
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(renderer->window, renderer->context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    

    //#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    //#endif

    
    // Initialize PNG loading
    int32_t imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        SDL_LogCritical(CATEGORY_ENGINE_SNAKE, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        assert(false);
        
        return false;
    }

    init_camera_2d(&renderer->camera, renderer->DISP_WIDTH, renderer->DISP_HEIGHT, glm::vec2(0, 0));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_BLEND);   

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


    // Create a dynamic vertex buffer
    glGenBuffers(1, &renderer->main_batch.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->main_batch.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);


    // Create the index buffer
    glGenBuffers(1, &renderer->main_batch.index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->main_batch.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndexCount * sizeof(unsigned short), indices, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    print_gles_errors();

   
    return success;
}

void init_camera_2d(Camera *camera, float width, float height, glm::vec2 camera_pos)
{
    camera->position = glm::vec3(camera_pos, 0.0f);
    
    // Our camera is going to be a stationary camera that will always be looking toward the center of the world coordinates; 
    // the up vector will always be pointing toward the positive y-axis.
    camera->view = glm::translate(glm::mat4(1.0f), camera->position);
    camera->projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
}

void create_quad(Batch *batch, Quad quad, SPRITE_ID sprite_id, glm::vec4 color)
{
    AtlasSprite sprite = DescAtlas[sprite_id];

    batch->vertices_ptr->x = quad.x;
    batch->vertices_ptr->y = quad.y;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    // Segundo vertice
    batch->vertices_ptr->x = quad.x + quad.width;
    batch->vertices_ptr->y = quad.y;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    // Tercer vertice
    batch->vertices_ptr->x = quad.x + quad.width;
    batch->vertices_ptr->y = quad.y + quad.height;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    // Cuarto vertice
    batch->vertices_ptr->x = quad.x;
    batch->vertices_ptr->y = quad.y + quad.height;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    batch->index_count += 6;
    batch->vertex_count += 4;
}

void create_color_triangle(Batch *batch, Triangle triangle, glm::vec4 color)
{
    AtlasSprite sprite = DescAtlas[NO_TEXTURE];

    batch->vertices_ptr->x = triangle.a.x;
    batch->vertices_ptr->y = triangle.a.y;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((float)(sprite.positionX) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((float)(sprite.positionY) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    // Segundo vertice
    batch->vertices_ptr->x = triangle.b.x;
    batch->vertices_ptr->y = triangle.b.y;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    // Tercer vertice
    batch->vertices_ptr->x = triangle.c.x;
    batch->vertices_ptr->y = triangle.c.y;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX + sprite.sourceWidth) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    // Cuarto vertice
    batch->vertices_ptr->x = triangle.a.x;
    batch->vertices_ptr->y = triangle.a.y;

    batch->vertices_ptr->r = color.r;
    batch->vertices_ptr->g = color.g;
    batch->vertices_ptr->b = color.b;
    batch->vertices_ptr->a = color.a;

    batch->vertices_ptr->u = ((sprite.positionX) / (float)ATLAS_WIDTH);
    batch->vertices_ptr->v = ((sprite.positionY + sprite.sourceHeight) / (float)ATLAS_HEIGHT);
    batch->vertices_ptr++;

    batch->index_count += 6;
    batch->vertex_count += 4;
}

void begin_batch(Batch *batch)
{
    batch->vertices_ptr = batch->vertices;
    batch->vertex_count = 0;
    batch->index_count = 0;
}

// TODO: Podria replicar el comportamiento de los VAO usando funciones...
void init_main_shader_attribs(Renderer *renderer) {

    if (!renderer->attribs_enabled)
    {
        renderer->AttribLocationVtxPos = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_pos");
        renderer->AttribLocationVtxUV = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_texCoord");
        renderer->AttribLocationVtxColor = glGetAttribLocation(renderer->shaders[TEXTURE_SHADER], "a_color");;

        renderer->attribs_enabled = true;
    }

    glBindBuffer(GL_ARRAY_BUFFER, renderer->main_batch.vertex_buffer);

    // NOTA: Este estado se eliminan cuando se usa glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(renderer->AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(renderer->AttribLocationVtxPos);

    glVertexAttribPointer(renderer->AttribLocationVtxColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(renderer->AttribLocationVtxColor);

    glVertexAttribPointer(renderer->AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(renderer->AttribLocationVtxUV);

    print_gles_errors();
}


void print_gles_errors()
{
#if defined(NDEBUG)
    GLuint err = glGetError();
    if (err != GL_NO_ERROR) {
        // Failed
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "OpenGL ERROR code %u\n", err);
        //assert(false);
    }
#endif
}


void shutdown_renderer(Renderer *renderer) 
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(renderer->context);
    SDL_DestroyWindow(renderer->window);
    SDL_Quit();
}


float sign (simple_vertex p1, simple_vertex p2, simple_vertex p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

//bool PointInTriangle (simple_vertex pt, simple_vertex v1, simple_vertex v2, simple_vertex v3)
bool is_over_triangle (Triangle triangle, int x, int y)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign((simple_vertex){x, y}, triangle.a, triangle.b);
    d2 = sign((simple_vertex){x, y}, triangle.b, triangle.c);
    d3 = sign((simple_vertex){x, y}, triangle.c, triangle.a);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}
