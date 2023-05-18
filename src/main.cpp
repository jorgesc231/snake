#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include <unistd.h>
#include <limits.h>

#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <SDL_image.h>
//#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <emscripten.h>

#else

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#endif

//#include <SDL2/SDL_opengles2.h>
#include <GLES2/gl2.h>          // Use GL ES 2

// Imgui stuff
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "assets_loader.h"
#include "renderer.h"

#define MAX_SND_CHANNELS 8

//#define IMGUI_IMPL_OPENGL_DEBUG

#define OPENGL_ERROR { GLenum err = glGetError(); IM_ASSERT(err == GL_NO_ERROR); }
void print_gles_errors();


// TODO: Por ahora imgui no funciona en la raspberry pi 1 B+
#if _RPI1
#define ACTIVATE_IMGUI 0
#else
#define ACTIVATE_IMGUI 1
#endif


// TODO: Ahora respeta el limite, pero el doble...
#define FPS 60
#define FRAME_TARGET_TIME (1000.0f / FPS)


#define SNAKE_LENGTH 256
#define SQUARE_X 17
#define SQUARE_Y 15
#define SQUARE_SIZE 40

#define DEFAULT_TIMESTEP 0.15f

// Colores
#define DEFAULT_CELL_COLOR1 glm::vec3(0.67f, 0.84f, 0.32f)
#define DEFAULT_CELL_COLOR2 glm::vec3(0.64f, 0.82f, 0.29f)


uint32_t DISP_WIDTH = 1280;
uint32_t DISP_HEIGHT = 720;

enum GAME_STATUS {
    PLAY,
    PAUSED,
    LOST,
};

enum
{
    CH_ANY = -1,
    CH_EAT,
    CH_MOVE,
    CH_GAME_OVER
};

enum
{
    SND_SNAKE_EAT,
    SND_SNAKE_MOVE,
    SND_SNAKE_DIE,
    SND_MAX
};

struct v2 {
    int32_t x, y;
};

struct Snake {
    v2 position;
    v2 direction;
    SPRITE_ID sprite;
};

struct Game_state {
    char *base_path = NULL;
    char assets_path_buffer[PATH_MAX];

    SDL_Window* window = NULL;
    SDL_GLContext context = NULL;

    Camera camara;
    batch_renderer renderer;

    bool game_over = false;
    bool quit = false;

    bool audio_loaded = false;
    bool audio_enabled = true;

    float time_step = DEFAULT_TIMESTEP;
    int32_t score = 0;
    GAME_STATUS status = PAUSED;
};

char cwd[PATH_MAX];

Snake snake[SNAKE_LENGTH] = {0};
int32_t tail_counter = 0;


v2 food_pos = {10, 7};
bool food_active = false;

Mix_Chunk* sounds[SND_MAX];

Vertex vertices[4000];

uint32_t prev_time = SDL_GetTicks();

// debug gui state
bool show_demo_window = false;
bool show_another_window = false;
bool show_debug_overlay = true;    

glm::vec4 clear_color = glm::vec4(0.34f, 0.54f, 0.20f, 1.0f);
glm::vec3 cell_color1 = DEFAULT_CELL_COLOR1;
glm::vec3 cell_color2 = DEFAULT_CELL_COLOR2;


uint32_t init_engine(Game_state *state);
void do_main_loop();
void shutdown_app(Game_state *state);
uint32_t init_game(Game_state *state);
void update_snake(Snake *snake, Game_state *state);
void game_render(Game_state *state);
void draw_debug_overlay();
void draw_debug_window(Game_state *state);


// Estado del juego
Game_state state;


int main(int argc, char* args[])
{
    init_engine(&state);
    
#ifndef __EMSCRIPTEN__

    while (!state.quit) {
        do_main_loop();
    }
    
    shutdown_app(&state);
#endif


#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(do_main_loop, 0, 0);
#endif


    return EXIT_SUCCESS;
}


uint32_t init_engine(Game_state *state)
{
    // Inicializa el generador de numeros aleatorios
    srand(time(0));
    
    bool success = false;

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        SDL_Log("Current working dir: %s\n", cwd);
    } else {
        SDL_Log("getcwd() error");
        return 1;
    }

    // Contruir el path absoluto hacia los assets
    if ((state->base_path = SDL_GetBasePath())) {
        SDL_Log("base path: %s\n", state->base_path);

    } else {
        SDL_Log("No se pudo obtener el Base Path\n");
        // TODO: Esto deberia ser un error fatal. 
    }


    // NOTE: Necesario para que SDL2 funcione con ANGLE
    #if !defined(__EMSCRIPTEN__) || !defined(_RPI1)
    SDL_SetHint("SDL_OPENGL_ES_DRIVER", "1");
    //SDL_SetHintWithPriority("SDL_OPENGL_ES_DRIVER", "1", SDL_HINT_OVERRIDE);
    #endif

    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }


    // Request OpenGL ES 2.0
    // Por alguna razon me da un contexto 3.0 con ANGLE
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    // Want double-buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);


    // Create the window
    state->window = SDL_CreateWindow("Snake2D - SDL2 + GLES2", SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED, DISP_WIDTH, DISP_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!state->window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could't create the main window.", NULL);
        return EXIT_FAILURE;
    }

    // Create OpenGL context
    state->context = SDL_GL_CreateContext(state->window);
    
    if (!state->context)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could't create the OpenGL context.", NULL);
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[SDL] GL context creation failed!");
        return EXIT_FAILURE;
    }

    SDL_GL_MakeCurrent(state->window, state->context);

    SDL_GL_SetSwapInterval(0);


#if ACTIVATE_IMGUI
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls


    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(state->window, state->context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    //#endif
#endif

    
    SDL_Log("GL_VERSION = %s\n",  glGetString(GL_VERSION));
    SDL_Log("GL_VENDOR = %s\n",  glGetString(GL_VENDOR));
    SDL_Log("GL_RENDERER = %s\n",  glGetString(GL_RENDERER));

    // Permite redimensionar la ventana
    SDL_SetWindowResizable(state->window, SDL_TRUE);
    SDL_SetWindowMinimumSize(state->window, 960, 540);

    glViewport(0, 0, DISP_WIDTH, DISP_HEIGHT);

#ifdef _RPI1
    if (SDL_ShowCursor(SDL_DISABLE));
#endif


    // Initialize PNG loading
    uint32_t imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        
        assert(false);
        
        return false;
    }

    // Inicializa SDL_Mixer
    uint32_t flags = MIX_INIT_MP3;
    uint32_t initted = Mix_Init (flags);
    if ((initted & flags) != flags)
    {
        fprintf(stderr, "Error al inicializar SDL_Mix: %s\n", Mix_GetError());
        
    } else {

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
        {
            printf("Couldn't initialize SDL Mixer\n");
            exit(1);
        }

        Mix_AllocateChannels(MAX_SND_CHANNELS);

        memset(sounds, 0, sizeof(Mix_Chunk*) * SND_MAX);

        // TODO: Reportar errores si no se puede cargar el audio...
        //sounds[SND_SNAKE_EAT] = Mix_LoadWAV("../assets/sound/food.mp3");
        sounds[SND_SNAKE_EAT] = Mix_LoadWAV(data_path(state->assets_path_buffer, state->base_path, "sound/food.mp3"));
        sounds[SND_SNAKE_MOVE] = Mix_LoadWAV(data_path(state->assets_path_buffer, state->base_path, "sound/move.mp3"));
        sounds[SND_SNAKE_DIE] = Mix_LoadWAV(data_path(state->assets_path_buffer, state->base_path, "sound/gameover.mp3"));

        state->audio_loaded = sounds[SND_SNAKE_EAT] && sounds[SND_SNAKE_MOVE] && sounds[SND_SNAKE_DIE];
    }



    // Inicializa el renderer
    init_camera_2d(&state->camara, 1280, 720, glm::vec2(0, 0));
    init_batch_renderer(&state->renderer, state->assets_path_buffer, state->base_path);

    print_gles_errors();


    // Inicializa el juego
    // TODO: Deberia estar separado de la inicializacion del engine
    init_game(state);

    print_gles_errors();
    
    update_snake(snake, state);

    return success;
}

void do_main_loop()
{    
        SDL_Event event;
        
        while(SDL_PollEvent(&event)) {

#if ACTIVATE_IMGUI
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif

            if (event.type == SDL_QUIT) {
                state.quit = true;
            }

            // Eventos de la ventana
            if (event.type == SDL_WINDOWEVENT) {
                switch(event.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
                    {
                        SDL_Log("SDL_WINDOWEVENT_SHOW");
                    } break;

                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        SDL_Log("Window %d resize to %dx%d\n", event.window.windowID,
                                event.window.data1, event.window.data2);
                    } break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        SDL_Log("Window %d size changed to %dx%d\n", event.window.windowID, event.window.data1, event.window.data2);
                        glViewport(0, 0, event.window.data1, event.window.data2);

                    } break;
                }
            }


            if (event.type == SDL_KEYDOWN ) {
                if (event.key.keysym.sym == SDLK_UP && !event.key.repeat) {
                    if (snake[0].direction.x != 0 && snake[0].direction.y != 1) {
                        snake[0].direction = (v2){0, -1};

                        if (state.audio_enabled) Mix_PlayChannel(CH_MOVE, sounds[SND_SNAKE_MOVE], 0);
                    }

                }
                if (event.key.keysym.sym == SDLK_DOWN && !event.key.repeat) {
                    if (snake[0].direction.x != 0 && snake[0].direction.y != -1) {
                        snake[0].direction = (v2){0, 1};

                        if (state.audio_enabled) Mix_PlayChannel(CH_MOVE, sounds[SND_SNAKE_MOVE], 0);
                    }
                }
                if (event.key.keysym.sym == SDLK_RIGHT && !event.key.repeat) {
                    if (snake[0].direction.x != -1 && snake[0].direction.y != 0) {
                        snake[0].direction = (v2){1, 0};

                        if (state.audio_enabled) Mix_PlayChannel(CH_MOVE, sounds[SND_SNAKE_MOVE], 0);
                    }

                }
                if (event.key.keysym.sym == SDLK_LEFT && !event.key.repeat) {
                    if (snake[0].direction.x != 1 && snake[0].direction.y != 0) {
                        snake[0].direction = (v2){-1, 0};

                        if (state.audio_enabled) Mix_PlayChannel(CH_MOVE, sounds[SND_SNAKE_MOVE], 0);
                    }
                }


                if (event.key.keysym.sym == SDLK_ESCAPE && !event.key.repeat && state.status != LOST) state.status = PAUSED;
                
                if (event.key.keysym.sym == SDLK_SPACE && !event.key.repeat ) {
                    
                    if (state.status == LOST) {
                        init_game(&state);            
                    }                

                    state.status = PLAY;
                }


                if (event.key.keysym.sym == SDLK_F1 && !event.key.repeat) {
                    show_debug_overlay = !show_debug_overlay;
                }

                #ifndef __EMSCRIPTEN__
                if (event.key.keysym.sym == SDLK_F5 && !event.key.repeat) {
                    state.quit = true;
                }
                #endif
            }
        }


        
        // The code calculates the elapsedTime in seconds since the last time this code was executed.
        uint32_t current_time = SDL_GetTicks();
        float time_to_wait = FRAME_TARGET_TIME - (current_time - prev_time);
        
        // NOTE: Esto no es necesario en WEB
        #ifndef __EMSCRIPTEN__
        if (time_to_wait > 0.0f && time_to_wait < FRAME_TARGET_TIME) {
            SDL_Delay(time_to_wait);
            //SDL_Log("%f", time_to_wait);
        }
        #endif

        float elapsed_time = (float)(current_time - prev_time) / 1000.0f;

        prev_time = current_time;  // Prepare for the next frame
        

        static float tiempo = 0;
        tiempo += elapsed_time;


        // Actualizacion de estado del juego

        if (tiempo > state.time_step && state.status == PLAY) {    

            update_snake(snake, &state);

            // Food Collision
            if ((snake[0].position.x == food_pos.x && snake[0].position.y == food_pos.y))
            {
                tail_counter += 1;
                state.score += 1;
                food_active = false;

                if (state.audio_enabled) Mix_PlayChannel(CH_EAT, sounds[SND_SNAKE_EAT], 0);
            }


            tiempo = 0;
        }


        // Food position calculation
        if (!food_active)
        {
            food_active = true;

            int loop_counter = 0;

            food_pos.x = rand() % SQUARE_X;
            food_pos.y = rand() % SQUARE_Y;

            for (int i = 0; i < tail_counter; i++)
            {
                while ((food_pos.x == snake[i].position.x) && (food_pos.y == snake[i].position.y))
                {
                    food_pos.x = rand() % SQUARE_X;
                    food_pos.y = rand() % SQUARE_Y;
                    
                    i = 0;
                    loop_counter++;

#ifdef NDEBUG
                    SDL_Log("snake: x = %d - y = %d", snake[i].position.x, snake[i].position.y);
                    SDL_Log("food:  x = %d - y = %d", food_pos.x, food_pos.y);
#endif

                }
            }

#ifdef NDEBUG
            SDL_Log("loop counter: %d", loop_counter);
#endif

        }


        if (state.status == LOST && state.game_over) {
            
            if (state.audio_enabled) Mix_PlayChannel(CH_GAME_OVER, sounds[SND_SNAKE_DIE], 0);

            state.game_over = false;
        }


        // Renderizar
        game_render(&state);
        print_gles_errors();

#if ACTIVATE_IMGUI
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();


        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);


        if (show_debug_overlay) {
            draw_debug_window(&state);
            draw_debug_overlay();
        }


        print_gles_errors();
        

        // Rendering
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        //glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif


        print_gles_errors();


        // Actualiza la ventana
        SDL_GL_SwapWindow(state.window);

        //SDL_Log("Current FPS: %f", 1.0f / elapsed_time);
}


// TODO: Falta...
void shutdown_app(Game_state *my_state)
{
#if ACTIVATE_IMGUI
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

#endif

    SDL_GL_DeleteContext(my_state->context);
    SDL_DestroyWindow(my_state->window);
    SDL_Quit();
}

uint32_t init_game(Game_state *state)
{
    state->score = 0;
    state->status = PAUSED;

    for (int i = 0; i < SNAKE_LENGTH; i++) {
        snake[i].position = (v2){0, 0};
        snake[i].direction = (v2){0, 0};
        snake[i].sprite = NO_TEXTURE;
    }

    tail_counter = 3;
    snake[0].position = (v2){4, 7};
    snake[0].direction = (v2){ 1, 0 };

    for (int i = 1; i < tail_counter; i++) {
        snake[i].position.x = snake[i - 1].position.x - 1;
        snake[i].position.y = snake[i - 1].position.y;
        snake[i].direction = (v2){ 1, 0 };
    }

    food_pos.x = 11;
    food_pos.y = 7;
    food_active = true;

    return 1;
}


void update_snake(Snake *snake, Game_state *state) 
{
    // Deteccion de colisiones
    if (snake[0].position.x >= 0 && snake[0].position.y >= 0 && snake[0].position.x <= SQUARE_X - 1 && snake[0].position.y <= SQUARE_Y - 1) {

        for (int i = tail_counter - 1; i >= 0; i--) {

            // Cabeza
            if (i == 0) {

                if (snake[i].direction.x == 1 && snake[i].direction.y == 0) {
                    snake[i].sprite = HEAD_RIGHT;
                }

                if (snake[i].direction.x == -1 && snake[i].direction.y == 0) {
                    snake[i].sprite = HEAD_LEFT;
                }

                if (snake[i].direction.x == 0 && snake[i].direction.y == 1) {
                    snake[i].sprite = HEAD_DOWN;
                }

                if (snake[i].direction.x == 0 && snake[i].direction.y == -1) {
                    snake[i].sprite = HEAD_UP;
                }
            }
            else {

                // TODO: en el caso de la cola lee memoria anterior
                snake[i].position = snake[i - 1].position;
                snake[i].direction = snake[i - 1].direction;

                // Cola
                if (i == tail_counter - 1) {
                
                    if (snake[i].direction.x == 0 && snake[i].direction.y == -1) {
                       snake[i].sprite = TAIL_DOWN;
                    }

                    if (snake[i].direction.x == 0 && snake[i].direction.y == 1) {
                       snake[i].sprite = TAIL_UP;
                    }

                    if (snake[i].direction.x == 1 && snake[i].direction.y == 0) {
                       snake[i].sprite = TAIL_LEFT;
                    }

                    if (snake[i].direction.x == -1 && snake[i].direction.y == 0) {
                       snake[i].sprite = TAIL_RIGHT;
                    }

                // Cuerpo
                } else {

                    if (snake[i].direction.x == 0 && (snake[i].direction.y == -1 || snake[i].direction.y == 1)) {
                       snake[i].sprite = BODY_VERTICAL;
                    }

                    if (snake[i].direction.y == 0 && (snake[i].direction.x == -1 || snake[i].direction.x == 1)) {
                       snake[i].sprite = BODY_HORIZONTAL;
                    }

                    if ((snake[i].direction.x == 0 && snake[i].direction.y == 1) && (snake[i + 1].direction.x == 1 && snake[i + 1].direction.y == 0)) {
                        snake[i].sprite = BODY_BOTTOMLEFT;   
                    }

                    if ((snake[i].direction.x == 1 && snake[i].direction.y == 0) && (snake[i + 1].direction.x == 0 && snake[i + 1].direction.y == 1)) {
                        snake[i].sprite = BODY_TOPRIGHT;   
                    }

                    if ((snake[i].direction.x == -1 && snake[i].direction.y == 0) && (snake[i + 1].direction.x == 0 && snake[i + 1].direction.y == 1)) {
                        snake[i].sprite = BODY_TOPLEFT;   
                    }

                    if ((snake[i].direction.x == 0 && snake[i].direction.y == -1) && (snake[i + 1].direction.x == 1 && snake[i + 1].direction.y == 0)) {
                        snake[i].sprite = BODY_TOPLEFT;   
                    }

                    if ((snake[i].direction.x == 0 && snake[i].direction.y == -1) && (snake[i + 1].direction.x == -1 && snake[i + 1].direction.y == 0)) {
                        snake[i].sprite = BODY_TOPRIGHT;   
                    }

                    if ((snake[i].direction.x == 1 && snake[i].direction.y == 0) && (snake[i + 1].direction.x == 0 && snake[i + 1].direction.y == -1)) {
                        snake[i].sprite = BODY_BOTTOMRIGHT;   
                    }

                    if ((snake[i].direction.x == -1 && snake[i].direction.y == 0) && (snake[i + 1].direction.x == 0 && snake[i + 1].direction.y == -1)) {
                        snake[i].sprite = BODY_BOTTOMLEFT;   
                    }

                    if ((snake[i].direction.x == 0 && snake[i].direction.y == 1) && (snake[i + 1].direction.x == -1 && snake[i + 1].direction.y == 0)) {
                        snake[i].sprite = BODY_BOTTOMRIGHT;   
                    }

                }
            }

        }

        snake[0].position.x += snake[0].direction.x;
        snake[0].position.y += snake[0].direction.y;

        for (int i = 1; i < tail_counter; i++) {
            if (snake[0].position.x == snake[i].position.x && snake[0].position.y == snake[i].position.y) {
                
    #if NDEBUG
                SDL_Log("Colision con el seg: %d", i);
    #endif

                state->status = LOST;
                state->game_over = true;
            }
        }
    } else {
        state->status = LOST; 
        state->game_over = true;
    }
}

void game_render(Game_state *state)
{
    //uint32_t indexCount = 0;
    GLushort indexCount = 0;
    uint32_t vertexCount = 0;
    
    Vertex* buffer = vertices;

    // Draw the cells    
    bool cambiar = false;

    for (int y = 0; y < SQUARE_Y; y++)
    {
        for (int x = 0; x < SQUARE_X; x++)
        {
            if (cambiar) {
                buffer = create_color_quad(buffer, glm::vec2(x, y), cell_color2);
                cambiar = false;
            } else {
                buffer = create_color_quad(buffer, glm::vec2(x, y), cell_color1);
                cambiar = true;
            }

            indexCount += 6;
            vertexCount += 4;
        }
    }


    // Draw the Snake
    for (int i = 0; i < tail_counter; i++) {

        if (snake[i].sprite != NO_TEXTURE) {
           buffer = create_texture_quad(buffer, snake[i].position.x, snake[i].position.y, snake[i].sprite);

            indexCount += 6;
            vertexCount += 4;
        }

    }


    // Draw the Food
    buffer = create_texture_quad(buffer, food_pos.x, food_pos.y, APPLE);
    indexCount += 6;
    vertexCount += 4;

    
    // Set dynamic vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, state->renderer.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(Vertex), vertices);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->renderer.index_buffer);
    
    glUseProgram(state->renderer.shaders[TEXTURE_SHADER]);
    glBindTexture(GL_TEXTURE_2D, state->renderer.textures[0]);
    

    //  Because multiplying matrices occurs from right to left,
    // we transform the matrix in reverse order: translate, rotate, and then scale.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3((DISP_WIDTH / 2) - (SQUARE_X * SQUARE_SIZE / 2), (DISP_HEIGHT / 2) - (SQUARE_Y * SQUARE_SIZE / 2), 0.0f));
    model = glm::scale(model, glm::vec3(SQUARE_SIZE, SQUARE_SIZE, 0.0f));

    glm::mat4 mvp = state->camara.projection * state->camara.view * model;
    
    
    GLint MVPUniformLoc = glGetUniformLocation(state->renderer.shaders[TEXTURE_SHADER], "u_MVP");
    if (MVPUniformLoc < 0) {
        SDL_Log("ERROR: No se pudo obtener la ubicacion de MVP\n");
    }
    
    glUniformMatrix4fv(MVPUniformLoc, 1, GL_FALSE, &mvp[0][0]);

    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);


    print_gles_errors();
    
    // NOTA: OpenGL ES 2.0 solo soporta indices de tipo SHORT, 3.0 soporta int.
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, NULL);

    print_gles_errors();

#if 0
    // TODO: Da error, investigar
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}


#if ACTIVATE_IMGUI
void draw_debug_window(Game_state *state)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    
    //ImGui::SetNextWindowBgAlpha(0.80f); // Transparent background
    ImGui::Begin("Estado del juego", &show_debug_overlay, window_flags);

    ImGui::Text("Estado:");
    ImGui::SameLine(0, 20);


    if (state->status == PLAY)  ImGui::TextColored(ImVec4(0, 255, 0, 1), "PLAY");
    if (state->status == PAUSED) ImGui::TextColored(ImVec4(255, 0, 255, 1), "PAUSED");
    if (state->status == LOST)  ImGui::TextColored(ImVec4(255, 0, 0, 1), "LOST");

    //if (item_disabled)
    //    ImGui::BeginDisabled(true);
    //
    //if (item_disabled)
    //        ImGui::EndDisabled();

    ImGui::SameLine(0, 40);
    if (ImGui::Button("PLAY", ImVec2(50, 20))) {
        if (state->status == LOST) {
            init_game(state);
            update_snake(snake, state);
        }

        state->status = PLAY;
    }
    ImGui::SameLine();

    if (ImGui::Button("PAUSE", ImVec2(50, 20))) {
        state->status = PAUSED;
    }

    ImGui::SameLine();

    if (ImGui::Button("LOST", ImVec2(50, 20))) {
        state->status = LOST;
    }

    ImGui::Separator();

    ImGui::SliderFloat("Time Step", &state->time_step, 0.1f, 1.0f);

    ImGui::Checkbox("Audio", &state->audio_enabled);

    
    //ImGui::SameLine();
    //ImGui::Checkbox("Colision cuerpo", &audio_enabled);
    //ImGui::SameLine();
    //ImGui::Checkbox("Colision paredes", &audio_enabled);
    //ImGui::SameLine();
    //ImGui::Checkbox("Wrap", &audio_enabled);



    ImGui::Separator();

    ImGui::InputInt("Score", &state->score, 1);

    ImGui::Separator();

    ImGui::InputInt("Food x", &food_pos.x);
    ImGui::InputInt("Food y", &food_pos.y);
    if (food_pos.x > SQUARE_X - 1) food_pos.x = SQUARE_X - 1;
    if (food_pos.x < 0) food_pos.x = 0;
    if (food_pos.y > SQUARE_Y - 1) food_pos.y = SQUARE_Y - 1;
    if (food_pos.y < 0) food_pos.y = 0;

    ImGui::Separator();

    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg;

    if (ImGui::BeginTable("body_table", 5, flags))
    {
        // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Position");
        ImGui::TableSetupColumn("Direction");
        ImGui::TableSetupColumn("Sprite");
        ImGui::TableHeadersRow();

        for (int row = 0; row < tail_counter; row++)
        {
            ImGui::TableNextRow();
            for (int column = 0; column < 5; column++)
            {
                ImGui::TableSetColumnIndex(column);
                //ImGui::Text("Hello %d,%d", column, row);

                switch(column) {
                    case 0:
                    {
                        ImGui::Text("%d", row);
                    } break;

                    case 1:
                    {
                        if (row == 0)   ImGui::Text("HEAD");
                        else if (row == tail_counter - 1) ImGui::Text("TAIL");
                        else ImGui::Text("BODY");

                    } break;

                    case 2:
                    {
                        ImGui::Text("(%d, %d)", snake[row].position.x, snake[row].position.y);
                    } break;

                    case 3:
                    {
                        ImGui::Text("(%d, %d)", snake[row].direction.x, snake[row].direction.y);
                    } break;

                    case 4:
                    {
                       char const *sprite_names[ATLAS_SPRITE_COUNT + 1] =  {
                            "NO_TEXTURE",
                            "APPLE",
                            "TAIL_RIGHT",
                            "TAIL_UP",
                            "TAIL_DOWN",
                            "TAIL_LEFT",
                            "BODY_BOTTOMLEFT",
                            "BODY_BOTTOMRIGHT",
                            "BODY_HORIZONTAL",
                            "BODY_TOPLEFT",
                            "BODY_TOPRIGHT",
                            "BODY_VERTICAL",
                            "HEAD_DOWN",
                            "HEAD_LEFT",
                            "HEAD_RIGHT",
                            "HEAD_UP",
                            "ATLAS_SPRITE_COUNT"
                        };

                        ImGui::Text("%s", sprite_names[snake[row].sprite]);
                    } break;
                }
            }
         }

        ImGui::EndTable();
    }


    ImGui::End();
}

void draw_debug_overlay()
{
    const float DISTANCE = 10.0f;
    static int corner = 3;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
    {
        window_flags |= ImGuiWindowFlags_NoMove;
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Renderer Debug overlay", &show_debug_overlay, window_flags))
    {
        ImGui::Text("Renderer Debug Overlay");
        ImGui::Separator();


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");


        ImGui::Separator();

        ImGui::ColorEdit3("Background##2f", &clear_color[0], ImGuiColorEditFlags_Float);
        ImGui::ColorEdit3("Cells 1##2f", &cell_color1[0], ImGuiColorEditFlags_Float);
        ImGui::ColorEdit3("Cells 2##2f", &cell_color2[0], ImGuiColorEditFlags_Float);

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (show_debug_overlay && ImGui::MenuItem("Close")) show_debug_overlay = false;
            ImGui::EndPopup();
        }
     }
     ImGui::End();
}


#endif


