#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include <SDL2/SDL_opengles2.h>

#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#define _IMPLEMENT_TRANSLATE_
#include "translate.h"

#include "snake.h"
#include "engine.h"
#include "gui.h"

// Emscripten necesita una funcion especial para pasarle parametros al main loop
Engine engine;
Game_state state;

int main(int argc, char* args[])
{
	log_init(&state);

    init_engine(&engine);

    // TODO: Implement a better way of manage errors
    state.quit = !init_game(&engine, &state);

#if !defined(__EMSCRIPTEN__)

    while (!state.quit)
    {
        main_loop();
    }
    
    shutdown_game(&engine);
#endif


#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(main_loop, 0, 0);
#endif


    return EXIT_SUCCESS;
}

float prueba_time = 0.0f;

void main_loop()
{    
    // The code calculates the elapsedTime in seconds since the last time this code was executed.
    uint64_t current_time = SDL_GetTicks64();
    float elapsed_time = (float)(current_time - engine.prev_time) / 1000.0f;
    engine.prev_time = current_time;  // Prepare for the next frame
    

    // Process Events
    process_events(&engine, &state);

    update_game(&state, elapsed_time);

    render_game(&engine, &state);
        
    print_gles_errors();

    draw_gui(&engine, &state);

    SDL_GL_SwapWindow(engine.window);

    //SDL_LogDebug(CATEGORY_GAME_SNAKE, "Current FPS: %f", 1.0f / elapsed_time);
}

int32_t init_game(Engine *engine, Game_state *state)
{
	get_system_language(state);

    if (engine->device_type == ENGINE_PHONE)
    {
        state->controller_type = CONTROLLER_TOUCH;
        state->selected_level = LEVEL_PHONE;

        if (engine->DISP_WIDTH > engine->DISP_HEIGHT)
        {
            state->score_bar_size = PC_SCORE_BAR_SIZE;
            engine->scale_factor = 1;
            engine->font_large_size = PC_LARGE_FONT_SIZE;
            engine->font_main_size = PC_MAIN_FONT_SIZE;
            engine->font_title_size = PC_TITLE_FONT_SIZE;
        }
        else
        {
            state->score_bar_size = PHONE_SCORE_BAR_SIZE;
            engine->scale_factor = 2;
            engine->font_large_size = PHONE_LARGE_FONT_SIZE;
            engine->font_main_size = PHONE_MAIN_FONT_SIZE;
            engine->font_title_size = PHONE_TITLE_FONT_SIZE;
        }
    }
    else
    {
        state->controller_type = CONTROLLER_KEYBOARD;
        state->selected_level = LEVEL_DEFAULT;

        state->score_bar_size = PC_SCORE_BAR_SIZE;
        engine->scale_factor = 1;
        engine->font_large_size = PC_LARGE_FONT_SIZE;
        engine->font_main_size = PC_MAIN_FONT_SIZE;
        engine->font_title_size = PC_TITLE_FONT_SIZE;
    }

    init_levels (state);

    // TODO: solucion temporal...
#if !defined(__EMSCRIPTEN__)
    load_state_file(engine, CONFIG_FILE_NAME, state);
#endif

    change_level(state, state->selected_level);

    calculate_gui(state, engine->DISP_WIDTH, engine->DISP_HEIGHT);

    print_gles_errors();

    // Load Texture and Shaders
    engine->textures[0] = texture_load(&engine->assets, "graphics/atlas.png");

    print_gles_errors();

    engine->shaders[TEXTURE_SHADER] = shader_prog_load(&engine->assets, "shaders/texture.vertex", "shaders/texture.fragment");

    print_gles_errors();

    // TODO: No se ejecutan estos asserts a pesar de estar definido NDEBUG...
#if defined(NDEBUG)
    assert(engine->textures[0]);
    assert(engine->shaders[TEXTURE_SHADER]);
#endif

    if (!engine->textures[0] || !engine->shaders[TEXTURE_SHADER])
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could't load game assets.", NULL);
        return 0;
    }

    glUseProgram(engine->shaders[TEXTURE_SHADER]);

    init_main_shader_attribs(engine);

    // Get the uniform location
    engine->AttribLocationProjMtx = glGetUniformLocation(engine->shaders[TEXTURE_SHADER], "u_Projection");
    if (engine->AttribLocationProjMtx < 0)
    {
        SDL_LogError(CATEGORY_GAME_SNAKE, "ERROR: Couldn't get \"MVP\" uniform location\n");
        assert(0);
    }

    glBindTexture(GL_TEXTURE_2D, engine->textures[0]);
    
    GLint tex_sampler_uniform_loc = glGetUniformLocation(engine->shaders[TEXTURE_SHADER], "texSampler");
    if (tex_sampler_uniform_loc < 0)
    {
        SDL_LogError(CATEGORY_GAME_SNAKE, "ERROR: Couldn't get \"texSampler\" uniform location\n");
        assert(0);
    }
    
    // Set Uniform to texture unit 0
    glUniform1i(tex_sampler_uniform_loc, 0);
    
    // Inicializa el audio
    if (init_audio(state->sounds, SND_MAX))
    {
        // TODO: Reportar errores si no se puede cargar el audio...
        state->sounds[SND_SNAKE_EAT] = Mix_LoadWAV(get_data_path(&engine->assets, "sound/food.mp3"));
        state->sounds[SND_INTERACTION] = Mix_LoadWAV(get_data_path(&engine->assets, "sound/move.mp3"));
        state->sounds[SND_SNAKE_DIE] = Mix_LoadWAV(get_data_path(&engine->assets, "sound/gameover_short.mp3"));

        state->audio_loaded = state->sounds[SND_SNAKE_EAT] && state->sounds[SND_INTERACTION] && state->sounds[SND_SNAKE_DIE];
    }
    else
    {
        state->audio_loaded = false;
        state->audio_enabled = false;
        SDL_LogError(CATEGORY_GAME_SNAKE, "Unable to load audio");
    }

    
    // Load the fonts
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();

    // IMPORTANT: AddFontFromMemoryTTF() by default transfer ownership of the data buffer to the font atlas, which will attempt to free it on destruction.
    void *font_data = NULL;
    int32_t font_data_size = 0;
    font_data = load_font(&engine->assets, "graphics/Bungee-Regular.ttf", font_data, &font_data_size);

    if (!font_data)
    {
    	SDL_LogError(CATEGORY_GAME_SNAKE, "Couldn't load font");
    	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Couldn't load font", NULL);
    	return 0;
    }

    print_gles_errors();

    // If you want to keep ownership of the data and free it yourself, you need to clear the FontDataOwnedByAtlas field
    // NOTE: La memoria de la fuente no es liberada porque solo se carga una vez y al cerrar es limpiada por el OS.
    // NOTE: Permitir que AddFontFromMemoryTTF() transfiera el ownership al font atlas hace que no se cierre de forma inmediata por estar liberando la memoria.
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
#if defined(NDEBUG)
    engine->font_debug = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 18.0f, &font_cfg);
    IM_ASSERT(engine->font_debug != NULL);
#endif

    engine->font_main = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, engine->font_main_size, &font_cfg);
    engine->font_large = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, engine->font_large_size, &font_cfg);
    engine->font_title = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, engine->font_title_size, &font_cfg);


    IM_ASSERT(engine->font_main != NULL);
    IM_ASSERT(engine->font_large != NULL);
    IM_ASSERT(engine->font_title != NULL);
    
    set_game_state(state);
    update_snake(state);

    return 1;
}

void process_events(Engine *engine, Game_state *state)
{
    SDL_Event event;
        
    while(SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
        {
            state->quit = true;
        }

        if (event.type == SDL_APP_WILLENTERBACKGROUND)
        {
        	if (state->status != LOST && state->status != INIT && state->status != PAUSED)
        	{
            	state->status = PAUSED;
                state->accept_input = false;
        	}
#if !defined(__EMSCRIPTEN__)
        	write_state_file(engine, CONFIG_FILE_NAME, state);
#endif
        }

        if (event.type == SDL_APP_DIDENTERBACKGROUND)
        {
        	// TODO: implement
        }

        if (event.type == SDL_APP_WILLENTERFOREGROUND)
        {
        	// TODO: implement
        }

        if (event.type == SDL_APP_DIDENTERFOREGROUND)
        {
        	// TODO: implement
        }

        if (event.type == SDL_APP_TERMINATING)
        {
#if !defined(__EMSCRIPTEN__)
        	write_state_file(engine, CONFIG_FILE_NAME, state);
#endif
        }

        // Eventos de la ventana
        if (event.type == SDL_WINDOWEVENT)
        {
            switch(event.window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED:
                {
                    SDL_LogDebug(CATEGORY_GAME_SNAKE, "Window %d size changed to %dx%d\n", event.window.windowID, event.window.data1, event.window.data2);

                    if (event.window.data1 != engine->DISP_WIDTH || event.window.data2 != engine->DISP_HEIGHT)
                    {
                        engine->DISP_WIDTH = event.window.data1;
                        engine->DISP_HEIGHT = event.window.data2;

                        glViewport(0, 0, engine->DISP_WIDTH, engine->DISP_HEIGHT);

                        init_camera_2d(&engine->camera, engine->DISP_WIDTH, engine->DISP_HEIGHT, glm::vec2(0, 0));

                        calculate_gui(state, engine->DISP_WIDTH, engine->DISP_HEIGHT);
                    }
                } break;

                case SDL_WINDOWEVENT_CLOSE:
                {
                    state->quit = true;
                } break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {

                } break;

                case SDL_WINDOWEVENT_RESTORED:
                {

                } break;

                case SDL_WINDOWEVENT_FOCUS_LOST:
                {

                } break;
            }
        }

			// TODO: Tiene que haber una mejor forma
			// TOUCH3 is active
			bool control3_active = state->controller_type == CONTROLLER_TOUCH3;

			// Mouse and Touch Input
			if ((event.type == SDL_MOUSEBUTTONDOWN  && (event.button.button & SDL_BUTTON_LEFT)) || event.type == SDL_FINGERDOWN)
			{
                int32_t x = event.button.x;
                int32_t y = event.button.y;

                // la entrada tactil va de 0.0 a 1.0.
                if (event.type == SDL_FINGERDOWN)
                {
                	x = (int32_t)(event.tfinger.x * engine->DISP_WIDTH);
                	y = (int32_t)(event.tfinger.y * engine->DISP_HEIGHT);
                	SDL_LogDebug(CATEGORY_GAME_SNAKE, "normalized %f x %f \t window %dx%d\n", event.tfinger.x, event.tfinger.y, x, y);
                }

				if (is_over(state->touch_input_rects[TOUCH_INPUT_UP], x, y) ||
						(control3_active && is_over_triangle(state->touch_input_triangles[TOUCH_INPUT_UP], x, y)))
				{
					add_input(state, DIR_UP);
				}

				if (is_over(state->touch_input_rects[TOUCH_INPUT_DOWN], x, y) ||
						(control3_active && is_over_triangle(state->touch_input_triangles[TOUCH_INPUT_DOWN], x, y)))
				{
					add_input(state, DIR_DOWN);
				}

				if (is_over(state->touch_input_rects[TOUCH_INPUT_RIGHT], x, y) ||
						(control3_active && is_over_triangle(state->touch_input_triangles[TOUCH_INPUT_RIGHT], x, y)))
				{
					add_input(state, DIR_RIGHT);
				}

				if (is_over(state->touch_input_rects[TOUCH_INPUT_LEFT], x, y) ||
						(control3_active && is_over_triangle(state->touch_input_triangles[TOUCH_INPUT_LEFT], x, y)))
				{
					add_input(state, DIR_LEFT);
				}
			}

        if ((event.type == SDL_MOUSEBUTTONUP  && (event.button.button & SDL_BUTTON_LEFT)) || event.type == SDL_FINGERUP)
        {
            state->input_dir = DIR_NONE;
        }

        if (event.type == SDL_KEYDOWN)
        {
            // Input del teclado
            if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) && !event.key.repeat)
            {
               add_input(state, DIR_UP);
            }

            if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) && !event.key.repeat)
            {
                add_input(state, DIR_DOWN);
            }

            if ((event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) && !event.key.repeat)
            {
                add_input(state, DIR_RIGHT);
            }

            if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) && !event.key.repeat)
            {
                add_input(state, DIR_LEFT);
            }

            // Pausa con escape
            if (event.key.keysym.sym == SDLK_ESCAPE && !event.key.repeat && state->status != LOST)
            {
              	if (state->audio_enabled) Mix_PlayChannel(CH_INTERACTION, state->sounds[SND_INTERACTION], 0);

               	if (state->status != PAUSED && state->status != INIT)
               	{
                    state->status = PAUSED;
                    state->accept_input = false;
                    state->show_menu = false;
              	}
               	else
               	{
                    state->status = PLAY;
                    state->accept_input = true;
              	}

            }

#if defined(NDEBUG)
            // Muestra el overlay de depuracion con F1
            if (event.key.keysym.sym == SDLK_F1 && !event.key.repeat)
            {
                state->show_debug_overlay = !state->show_debug_overlay;
            }

            // Oculta la pantalla de status con F2
            if (event.key.keysym.sym == SDLK_F2 && !event.key.repeat)
            {
                state->show_status_screen = !state->show_status_screen;
            }

            // Cierra el juego con F5
            #ifndef __EMSCRIPTEN__
            if (event.key.keysym.sym == SDLK_F5 && !event.key.repeat)
            {
                state->quit = true;
            }
            #endif
#endif

            // Si el teclado tiene boton de mute o stop, desactiva el audio.
            if ((event.key.keysym.sym == SDLK_AUDIOMUTE || event.key.keysym.sym == SDLK_AUDIOSTOP) && !event.key.repeat)
            {
                state->audio_enabled = !state->audio_enabled;
            }
        }

        // TODO: Inplementar swipes
#if 0
        if (event.type == SDL_FINGERMOTION)
        {
            SDL_Log("Finger Motion: x = %f y = %f dx = %f dy = %f", event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy);

            float adx = abs(event.tfinger.dx);
            float ady = abs(event.tfinger.dy);

            //float TOUCH_THRESHOLD_X = 20 * (1 / (float)renderer->DISP_WIDTH);
            //float TOUCH_THRESHOLD_Y = 20 * (1 / (float)renderer->DISP_HEIGHT);

            float TOUCH_THRESHOLD_X = event.tfinger.dx / 4;
            float TOUCH_THRESHOLD_Y = event.tfinger.dy / 4;

            // swite horizontal
            if (adx > ady && adx > TOUCH_THRESHOLD_X)
            {
                if (0 > event.tfinger.dx) // swipe right to left
                {
                    SDL_Log("Swipe Left");
                    add_input(state, DIR_LEFT);

                }
                else  // swipe left to right
                {
                    SDL_Log("Swipe Right");
                    add_input(state, DIR_RIGHT);
                }
            } else if (ady > adx && ady > TOUCH_THRESHOLD_Y)
            {
                if (0 > event.tfinger.dy) // swipe bottom to top
                {
                    SDL_Log("Swipe Top");
                    add_input(state, DIR_UP);
                }
                else // swipe top to bottom
                {
                    SDL_Log("Swipe Bottom");
                    add_input(state, DIR_DOWN);
                }
            }


        }
#endif
    }

    // Save state before quit
    if (state->quit)
    {
#if !defined(__EMSCRIPTEN__)
    	write_state_file(engine, CONFIG_FILE_NAME, state);
#endif
    }
}

void set_game_state(Game_state *state)
{
    state->score = 0;
    state->status = INIT;
    state->time_out_completed = false;
    state->accept_input = false;

    switch (state->difficulty)
    {
        case DIFFICULTY_SLOW:
            state->time_step = DEFAULT_TIMESTEP;
            break;
        case DIFFICULTY_NORMAL:
            state->time_step = DEFAULT_NORMAL_TIMESTEP;
            break;
        case DIFFICULTY_FAST:
            state->time_step = DEFAULT_FAST_TIMESTEP;
            break;
        case DIFFICULTY_PROGRESSIVE:
            state->time_step = DEFAULT_TIMESTEP;
            break;
        default:
            state->time_step = DEFAULT_NORMAL_TIMESTEP;
            break;
    }

    state->direction_buffer[0] = DIR_NONE;
    state->direction_buffer[1] = DIR_NONE;

    for (int32_t i = 0; i < MAX_SNAKE_LENGTH; i++) {
        state->snake[i].position = (v2){0, 0};
        state->snake[i].direction = (v2){0, 0};
    }

    // Add Snake
    state->tail_counter = INITIAL_TAIL_COUNTER;
    state->snake[0].position = (v2){state->columns / 3 - 1, state->rows / 2 - 1};
    state->snake[0].direction = (v2){ 1, 0 };

    for (int32_t i = 1; i < state->tail_counter; i++) {
        state->snake[i].position.x = state->snake[i - 1].position.x - 1;
        state->snake[i].position.y = state->snake[i - 1].position.y;
        state->snake[i].direction = (v2){ 1, 0 };
    }

    // Add Food
    state->food_pos.x = state->columns / 3 * 2;
    state->food_pos.y = state->snake[0].position.y;
    state->food_active = true;
}

void update_game (Game_state *state, float elapsed_time) {

    state->time += elapsed_time;
    
    if (state->status != PAUSED && state->status != LOST)
    {
        prueba_time += elapsed_time;
    }
    
    SDL_LogDebug(CATEGORY_GAME_SNAKE, "time = %f - elapsed_time = %f", prueba_time, elapsed_time);

    // Actualizacion de estado del juego
    // NOTA: Importante el >= para que no parpadeen cosas
    if (state->time >= state->time_step && state->status == PLAY)
    {
        update_snake(state);
        state->time = 0;

        // IMPORTANTE: Le restamos los 150ms al temporizador en lugar de igualarlo a 0.
        // Esto evita que el juego se ralentice si un frame dura un poco más de lo normal.
        //prueba_time -= state->time_step;
        prueba_time = 0;
    }

    // TODO: Manejar el caso en el que el mapa este totalmente ocupado por la serpiente y
    // no quede espacio libre para colocar la comida, por ahora probablemente crash...o ciclo infinito
    // Food position calculation
    if (!state->food_active)
    {
        state->food_active = true;

        state->food_pos.x = rand() % state->columns;
        state->food_pos.y = rand() % state->rows;

        for (int i = 0; i < state->tail_counter; i++)
        {
            while ((state->food_pos.x == state->snake[i].position.x) && (state->food_pos.y == state->snake[i].position.y))
            {
                state->food_pos.x = rand() % state->columns;
                state->food_pos.y = rand() % state->rows;
                    
                i = 0;

                SDL_LogDebug(CATEGORY_GAME_SNAKE, "snake: x = %d - y = %d", state->snake[i].position.x, state->snake[i].position.y);
                SDL_LogDebug(CATEGORY_GAME_SNAKE, "food:  x = %d - y = %d", state->food_pos.x, state->food_pos.y);
            }
        }
    }

    if (state->status == LOST)
    {
        if (state->game_over)
        {
            if (state->audio_enabled)   Mix_PlayChannel(CH_GAME_OVER, state->sounds[SND_SNAKE_DIE], 0);
            state->game_over = false;
            state->show_status_screen = false;
            state->time = 0;
        }

        // Delay after lose, temporal implementation...
        if (state->time > state->time_out && !state->time_out_completed)
        {
            state->show_status_screen = true;
            state->time_out_completed = true;
        }
    }
}


// TODO: Optimizar
void update_snake(Game_state *state)
{
    // if the snake collides with his own body the head will be rotated,
    // so we restore the old direction
    v2 old_dir = state->snake[0].direction;

    switch(get_input(state))
    {
        case DIR_UP:
        {
            state->snake[0].direction = (v2){0, -1};
        } break;

        case DIR_DOWN:
        {
            state->snake[0].direction = (v2){0, 1};
        } break;

        case DIR_RIGHT:
        {
            state->snake[0].direction = (v2){1, 0};
        } break;

        case DIR_LEFT:
        {
            state->snake[0].direction = (v2){-1, 0};
        } break;

        case DIR_NONE:
        default:
        {

        } break;
    }

    v2 future_pos = {state->snake[0].position.x + state->snake[0].direction.x, state->snake[0].position.y + state->snake[0].direction.y};

    // Food Collision
    if ((future_pos.x == state->food_pos.x && future_pos.y == state->food_pos.y))
    {
        // Grow the snake
        state->tail_counter += 1;
        state->snake[state->tail_counter - 1].position = state->snake[state->tail_counter - 2].position;
        state->snake[state->tail_counter - 1].direction = state->snake[state->tail_counter - 2].direction;
        
        // Increase score
        state->score += 1;
        state->food_active = false;

        if (state->audio_enabled) Mix_PlayChannel(CH_EAT, state->sounds[SND_SNAKE_EAT], 0);

        // TODO: Mejorar!!!
        int32_t* highest_score = &state->levels[state->selected_level].hight_score_by_difficulty[state->difficulty];
        if (state->score > *highest_score) *highest_score = state->score;

        // Aumento de velocidad
        if (state->difficulty == DIFFICULTY_PROGRESSIVE)
        {
            if (state->score >= 5 && state->score <= 10) state->time_step -= 0.002f;
            else if (state->score > 10) state->time_step -= 0.001f;
        }

    }

    // Head with Body Collision
    for (int i = 1; i < state->tail_counter; i++)
    {
        if (future_pos.x == state->snake[i].position.x && future_pos.y == state->snake[i].position.y
                && i != state->tail_counter - 1)
        {
                state->status = LOST;
                state->accept_input = false;
                state->game_over = true;
                state->snake[0].direction = old_dir;
                return;
        }
    }

    // Head with map limits collisions
    if (future_pos.x >= 0 && future_pos.x <= state->columns - 1 && future_pos.y >= 0 && future_pos.y <= state->rows - 1)
    {
        for (int i = state->tail_counter - 1; i > 0; i--) {

            state->snake_old[i].position = state->snake[i].position;
            state->snake_old[i].direction = state->snake[i].direction;

            // TODO: en el caso de la cola lee memoria anterior
            state->snake[i].position = state->snake[i - 1].position;
            state->snake[i].direction = state->snake[i - 1].direction;
        }

        state->snake_old[0].position = state->snake[0].position;

        state->snake[0].position.x += state->snake[0].direction.x;
        state->snake[0].position.y += state->snake[0].direction.y;

    } else {
        state->status = LOST;
        state->accept_input = false;
        state->game_over = true;
        state->snake[0].direction = old_dir;
        return;
    }
}

// De los dos valores toma el mas pequeño
#define Min(a,b) (((a)<(b))?(a):(b))
// De los dos valores toma el mas grande
#define Max(a,b) (((a)>(b))?(a):(b))

// Primero toma el mayor entre el valor y el suelo (0.0)
// Luego toma el menor entre el resultado y el techo (1.0)
float Clamp(float valor, float minimo, float maximo)  {
    return Max(minimo, Min(valor, maximo));
}

// TODO: Experimentando con lerp deberia estar en otro archivo
float lerp(float a, float t, float b) {
    float x = a + (b - a) * t;
    return (x);
}

float prueba_x = 0.0f;
float prueba_y = 0.0f;

void render_game(Engine *engine, Game_state *state)
{
    // Init the main batch
    begin_batch(&engine->main_batch);

    // add operations to the batch

    // Draw the score bar background
    create_quad(&engine->main_batch, state->score_rect, NO_TEXTURE, glm::vec4(0.07f, 0.09f, 0.07f, 1.0f));

    // Draw the map
    for (int32_t y = 0; y < state->rows; y++)
    {
        for (int32_t x = 0; x < state->columns; x++)
        {
            Quad map_quad = (Quad) {x * state->cell_size + state->screen_rect.x, y * state->cell_size + state->screen_rect.y, state->cell_size, state->cell_size};

            if (state->game_skin == SKIN_DEFAULT)
            {
                if (y % 2 == 0)
                {
                    if (x % 2 == 0) {
                        create_quad(&engine->main_batch, map_quad, NO_TEXTURE, engine->cell_color2);
                    } else {
                        create_quad(&engine->main_batch, map_quad, NO_TEXTURE, engine->cell_color1);
                    }
                } else {
                    if (x % 2 == 0) {
                        create_quad(&engine->main_batch, map_quad, NO_TEXTURE, engine->cell_color1);
                    } else {
                        create_quad(&engine->main_batch, map_quad, NO_TEXTURE, engine->cell_color2);
                    }
                }
            }
            else if (state->game_skin == SKIN_RARE)
            {
                if (y % 2 == 0)
                {
                    if (x % 2 == 0) {
                        create_quad(&engine->main_batch, map_quad, RARE_GRASS_CLEAR, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    } else {
                    	create_quad(&engine->main_batch, map_quad, RARE_GRASS_DARK, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    }
                } else {
                    if (x % 2 == 0) {
                    	create_quad(&engine->main_batch, map_quad, RARE_GRASS_DARK, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    } else {
                    	create_quad(&engine->main_batch, map_quad, RARE_GRASS_CLEAR, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    }
                }
            }
            else if (state->game_skin == SKIN_MINIMAL)
            {
            	create_quad(&engine->main_batch, map_quad, NO_TEXTURE, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }
            else if (state->game_skin == SKIN_RETRO)
            {
                create_quad(&engine->main_batch, map_quad, RETRO_CELL, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            else if (state->game_skin == SKIN_CLASSIC)
            {
            	create_quad(&engine->main_batch, map_quad, CLASSIC_BODY, glm::vec4(0.25f, 0.25f, 0.25f, 0.1f));
            }
        }
    }

    // Draw the border for the classic skin
    if (state->game_skin == SKIN_CLASSIC)
    {
        int32_t border_size = state->border_size;
        int32_t border_height = state->cell_size * state->rows;

        // Left Border
        Quad border_quad = (Quad) {state->screen_rect.x - border_size, state->screen_rect.y, border_size, border_height + border_size};
        create_quad(&engine->main_batch, border_quad, NO_TEXTURE, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));

        // Right Border
        border_quad = (Quad) {state->screen_rect.x + state->screen_rect.width, state->screen_rect.y, border_size, border_height + border_size};
        create_quad(&engine->main_batch, border_quad, NO_TEXTURE, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));

        // Bottom Border
        border_quad = (Quad) {state->screen_rect.x - border_size, state->screen_rect.y + border_height, state->screen_rect.width + border_size, border_size};
        create_quad(&engine->main_batch, border_quad, NO_TEXTURE, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
    }

    // Animar de forma lineal el decrecimiento y creciemiento de un segmento de la serpiente
    int shrinking_size = 0;
    int expanding_size = 0;

    float time_between = prueba_time / state->time_step;

    // Limitamos time_between a 1.0 para que no se pase del objetivo
    time_between = Min(time_between, 1.0f);

    // Draw the Snake
    for (int32_t i = state->tail_counter - 1; i >= 0; i--)
    {
        //Quad snake_quad = (Quad) {state->snake[i].position.x * state->cell_size + state->screen_rect.x, state->snake[i].position.y * state->cell_size + state->screen_rect.y, state->cell_size, state->cell_size};
        Quad snake_quad = {0};
        
        v2 snake_pos = game_to_screen_pos(state->snake[i].position, state);

        if (state->lerp) {
            int old_snake_x = state->snake_old[i].position.x * state->cell_size + state->screen_rect.x;
            int old_snake_y = state->snake_old[i].position.y * state->cell_size + state->screen_rect.y;
    
            if (prueba_time < state->time_step) {  
                prueba_x = lerp(old_snake_x, time_between, snake_pos.x);
                prueba_y = lerp(old_snake_y, time_between, snake_pos.y);
                shrinking_size = lerp(state->cell_size, time_between, 0);
                expanding_size = lerp(0.0f, time_between, state->cell_size);
            } else {
                prueba_x = snake_pos.x;
                prueba_y = snake_pos.y;
            }
        
    
           #if 0
            if (i == 0) {
                SDL_LogDebug(CATEGORY_GAME_SNAKE, "LERP: old_x = %d - target_x = %d - lerp_x = %f", old_snake_x, snake_pos.x, prueba_x);
                SDL_LogDebug(CATEGORY_GAME_SNAKE, "TIME: time_step = %f - elapsed_time = %f", state->time_step, prueba_time);
                SDL_LogDebug(CATEGORY_GAME_SNAKE, "TIME: time between = %f", time_between);
            }
            #endif
         
            snake_quad = (Quad) {prueba_x, prueba_y, state->cell_size, state->cell_size};
        } else {
            snake_quad = (Quad) {snake_pos.x, snake_pos.y, state->cell_size, state->cell_size};
        }
    
        SPRITE_ID sprite = NO_TEXTURE;

            if (state->game_skin == SKIN_MINIMAL)
            {
                if (i != 0) create_quad(&engine->main_batch, snake_quad, NO_TEXTURE, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
                else create_quad(&engine->main_batch, snake_quad, NO_TEXTURE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            }
            else if (state->game_skin == SKIN_CLASSIC)
            {
            	if (i != 0) create_quad(&engine->main_batch, snake_quad, CLASSIC_BODY, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
            	else create_quad(&engine->main_batch, snake_quad, CLASSIC_HEAD, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
            }
            else if (state->game_skin == SKIN_RETRO)
            {
                create_quad(&engine->main_batch, snake_quad, RETRO_BODY, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            else
            {
                // Cabeza
                if (i == 0)
                {
                    if (state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0)
                    {
                        sprite = state->game_skin == SKIN_DEFAULT ? HEAD_RIGHT : RARE_HEAD_RIGHT;
                    }

                    if (state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0)
                    {
                        sprite = state->game_skin == SKIN_DEFAULT ? HEAD_LEFT : RARE_HEAD_LEFT;
                    }

                    if (state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1)
                    {
                        sprite = state->game_skin == SKIN_DEFAULT ? HEAD_DOWN : RARE_HEAD_DOWN;
                    }

                    if (state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1)
                    {
                        sprite = state->game_skin == SKIN_DEFAULT ? HEAD_UP : RARE_HEAD_UP;
                    }
                }
                else
                {
                    // Cola
                    if (i == state->tail_counter - 1)
                    {
                        Quad relleno = snake_quad;
                        SPRITE_ID relleno_sprite;


                        // TODO: En vez de ir achicando la cola deberia simplemente moverla a la vez que se recorta el sprite
                        //       asi evito que la punta de la cola se ensanche...
 
                        // Dibuja la cola cuando viene doblando
                        if (state->snake[i].direction.x != state->snake_old[i].direction.x || state->snake[i].direction.y != state->snake_old[i].direction.y) 
                        {
                            relleno.x = state->snake[i].position.x * state->cell_size + state->screen_rect.x;
                            relleno.y = state->snake[i].position.y * state->cell_size + state->screen_rect.y;

                            // TODO: No se si es necesario
                            // Crea un limite hasta donde se puede mover el quad cuando le cambio el tamaño
                            v2 quad_limit = game_to_screen_pos(state->snake[i].position, state);
                            quad_limit.x += state->cell_size;
                            quad_limit.y += state->cell_size;

                            // Top Right (Tail Right)
                            if (state->snake[i].direction.y == -1 && state->snake_old[i].direction.x == -1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPRIGHT : RARE_BODY_TOPRIGHT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_RIGHT : RARE_TAIL_RIGHT;

                                snake_quad.width = shrinking_size;
                                if (snake_quad.x < quad_limit.x) snake_quad.x = quad_limit.x;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }

                            // Bottom Right (Tail Right)
                            if (state->snake[i].direction.y == 1 && state->snake_old[i].direction.x == -1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMRIGHT : RARE_BODY_BOTTOMRIGHT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_RIGHT : RARE_TAIL_RIGHT;

                                snake_quad.width = shrinking_size;
                                if (snake_quad.x < quad_limit.x) snake_quad.x = quad_limit.x;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }

                            // Top Left (Tail Left)
                            if (state->snake[i].direction.y == -1 && state->snake_old[i].direction.x == 1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPLEFT : RARE_BODY_TOPLEFT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_LEFT : RARE_TAIL_LEFT;

                                snake_quad.width = shrinking_size;
                                snake_quad.x += 5;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }

                            // Bottom Left (Tail Left)
                            if (state->snake[i].direction.y == 1 && state->snake_old[i].direction.x == 1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMLEFT : RARE_BODY_BOTTOMLEFT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_LEFT : RARE_TAIL_LEFT;
                                snake_quad.width = shrinking_size;
                                snake_quad.x += 5;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }
                             
                            // Bottom Left (Tail Down)
                            if (state->snake[i].direction.x == -1 && state->snake_old[i].direction.y == -1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMLEFT : RARE_BODY_BOTTOMLEFT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_DOWN : RARE_TAIL_DOWN;

                                snake_quad.height = shrinking_size;
                                if (snake_quad.y < quad_limit.y) snake_quad.y = quad_limit.y;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }

                            // Bottom Right (Tail Down)
                            if (state->snake[i].direction.x == 1 && state->snake_old[i].direction.y == -1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMRIGHT : RARE_BODY_BOTTOMRIGHT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_DOWN : RARE_TAIL_DOWN;

                                snake_quad.height = shrinking_size;
                                if (snake_quad.y < quad_limit.y) snake_quad.y = quad_limit.y;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }

                            // Top Left (Tail Up)
                            if (state->snake[i].direction.x == -1 && state->snake_old[i].direction.y == 1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPLEFT : RARE_BODY_TOPLEFT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_UP : RARE_TAIL_UP;
                                snake_quad.height = shrinking_size;
                                snake_quad.y += 5;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }

                            // Top Right (Tail Up)
                            if (state->snake[i].direction.x == 1 && state->snake_old[i].direction.y == 1)
                            {
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPRIGHT : RARE_BODY_TOPRIGHT;
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_UP : RARE_TAIL_UP;
                                snake_quad.height = shrinking_size;
                                snake_quad.y += 5;

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }


                            
                        } else {

                            // Dibuja la cola cuando la serpiente viene recto

                            #if 1
                            // Dibuja la cola en horizontal hacia la izquierda
                            if (state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0)
                            {
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_LEFT : RARE_TAIL_LEFT;
                                relleno.x += snake_quad.width;
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_HORIZONTAL : RARE_BODY_HORIZONTAL;

                                if (state->snake[i - 1].direction.x == 0) {
                                    relleno.width = shrinking_size;
                                }

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                                
                            }
                            #endif

                            #if 1
                            // Dibuja la cola en horizontal hacia la derecha
                            if (state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0)
                            {
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_RIGHT : RARE_TAIL_RIGHT;
                                relleno.x -= snake_quad.width;
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_HORIZONTAL : RARE_BODY_HORIZONTAL;

                                if (state->snake[i - 1].direction.x == 0) {
                                    relleno.width = shrinking_size;
                                    relleno.x = state->snake[i - 1].position.x * state->cell_size + state->screen_rect.x + state->cell_size;
                                }

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }
                            #endif

                            #if 1
                            // Dibuja la cola en vertical hacia arriba
                            if (state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1)
                            {
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_UP : RARE_TAIL_UP;
                                relleno.y += snake_quad.height;
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_VERTICAL : RARE_BODY_VERTICAL;

                                if (state->snake[i - 1].direction.y == 0) {
                                    relleno.height = shrinking_size;
                                }

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }
                            #endif

                            #if 1
                            // Dibuja la cola en vertical hacia abajo
                            if (state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1)
                            {
                                sprite = state->game_skin == SKIN_DEFAULT ? TAIL_DOWN : RARE_TAIL_DOWN;
                                relleno.y -= snake_quad.height;
                                relleno_sprite = state->game_skin == SKIN_DEFAULT ? BODY_VERTICAL : RARE_BODY_VERTICAL;

                                if (state->snake[i - 1].direction.y == 0) {
                                    relleno.height = shrinking_size;
                                    relleno.y = state->snake[i - 1].position.y * state->cell_size + state->screen_rect.y + state->cell_size;
                                }

                                create_quad(&engine->main_batch, relleno, relleno_sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                            }
                            #endif
                        }
                    }
                    else  // Cuerpo
                    {
                        // Se mueve en vertical
                        if (state->snake[i].direction.x == 0 && (state->snake[i].direction.y == -1 || state->snake[i].direction.y == 1))
                        {                          
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_VERTICAL : RARE_BODY_VERTICAL;
                            
                            if (i == 1 && !(state->snake[i].direction.y != state->snake[i + 1].direction.y)) {
                                
                                if (state->snake[i].direction.y == 1 && state->lerp)
                                {
                                    snake_quad.y = state->snake[i].position.y * state->cell_size + state->screen_rect.y;
                                } 

                                snake_quad.height = expanding_size;                               
                            } else {
                                snake_quad.x = snake_pos.x;
                                snake_quad.y = snake_pos.y;
                            }
                        }

                        // Se mueve en horizontal
                        if (state->snake[i].direction.y == 0 && (state->snake[i].direction.x == -1 || state->snake[i].direction.x == 1))
                        {                          
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_HORIZONTAL : RARE_BODY_HORIZONTAL;
                            
                            if (i == 1 && state->snake[i].direction.x == state->snake[i + 1].direction.x) {

                                if (state->snake[i].direction.x == 1 && state->lerp)
                                {
                                    snake_quad.x = state->snake[i].position.x * state->cell_size + state->screen_rect.x;
                                }

                                snake_quad.width = expanding_size;
                            }
                            else {
                                snake_quad.x = snake_pos.x;
                                snake_quad.y = snake_pos.y;
                            }
                        }

                        // Movimientos en giro
                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1) && (state->snake[i + 1].direction.x == 1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMLEFT : RARE_BODY_BOTTOMLEFT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }

                        if ((state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == 1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPRIGHT : RARE_BODY_TOPRIGHT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }

                        #if 1
                        if ((state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == 1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPLEFT : RARE_BODY_TOPLEFT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }
                        #endif
#if 1
                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1) && (state->snake[i + 1].direction.x == 1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPLEFT : RARE_BODY_TOPLEFT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }
#endif
                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1) && (state->snake[i + 1].direction.x == -1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPRIGHT : RARE_BODY_TOPRIGHT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }

                        if ((state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == -1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMRIGHT : RARE_BODY_BOTTOMRIGHT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }

                        if ((state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == -1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMLEFT : RARE_BODY_BOTTOMLEFT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }

                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1) && (state->snake[i + 1].direction.x == -1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMRIGHT : RARE_BODY_BOTTOMRIGHT;
                            snake_quad.x = snake_pos.x;
                            snake_quad.y = snake_pos.y;
                        }

                    }
                }

                create_quad(&engine->main_batch, snake_quad, sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
    }


     // Draw the Food
    Quad food_quad = (Quad) {state->food_pos.x * state->cell_size + state->screen_rect.x, state->food_pos.y * state->cell_size + state->screen_rect.y, state->cell_size, state->cell_size};
    if (state->game_skin == SKIN_DEFAULT)
    {
        create_quad(&engine->main_batch, food_quad, APPLE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    }
    else if (state->game_skin == SKIN_RARE)
    {
        create_quad(&engine->main_batch, food_quad, FRUIT, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    else if (state->game_skin == SKIN_MINIMAL)
    {
        create_quad(&engine->main_batch, food_quad, NO_TEXTURE, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
    else if (state->game_skin == SKIN_CLASSIC)
    {
        create_quad(&engine->main_batch, food_quad, CLASSIC_FOOD, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
    }
    else if (state->game_skin == SKIN_RETRO)
    {
        create_quad(&engine->main_batch, food_quad, RETRO_FOOD, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    draw_touch_controls(state, engine);

    
    // Set dynamic vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, engine->main_batch.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, engine->main_batch.vertex_count * sizeof(Vertex), engine->main_batch.vertices);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine->main_batch.index_buffer);
    
    glUseProgram(engine->shaders[TEXTURE_SHADER]);

    glBindTexture(GL_TEXTURE_2D, engine->textures[0]);

    init_main_shader_attribs(engine);

    // Copy the projection matrix to the GPU
    glUniformMatrix4fv(engine->AttribLocationProjMtx, 1, GL_FALSE, &engine->camera.projection[0][0]);

    // TODO: Esto no deberia ser asi...
    if (state->game_skin == SKIN_RETRO)
    {
        glClearColor(0.35f, 0.37f, 0.26f, 1.0f);
    }
    else if (state->game_skin == SKIN_MINIMAL)
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }
    else if (state->game_skin == SKIN_CLASSIC)
    {
        glClearColor(0.7f, 0.74f, 0.03f, 1.0f);
    }
    else
    {
        glClearColor(engine->clear_color.r, engine->clear_color.g, engine->clear_color.b, engine->clear_color.a);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    print_gles_errors();
    
    // NOTA: OpenGL ES 2.0 solo soporta indices de tipo SHORT, 3.0 soporta int.
    glDrawElements(GL_TRIANGLES, engine->main_batch.index_count, GL_UNSIGNED_SHORT, NULL);

    print_gles_errors();


// TODO: It's not necessary to reset the state every frame
#if 0
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

}

DIRECTION direction_vector_enum(v2 dir)
{
    if (dir.x == 0 && dir.y == -1)  return DIR_UP;
    if (dir.x == 0 && dir.y == 1)   return DIR_DOWN;
    if (dir.x == -1 && dir.y == 0)  return DIR_LEFT;
    if (dir.x == 1 && dir.y == 0)   return DIR_RIGHT;
    return DIR_NONE;
}

v2 direction_enum_vector(DIRECTION dir)
{
    if (dir == DIR_UP)  return (v2){0, -1};
    if (dir == DIR_DOWN) return (v2){0, 1};
    if (dir == DIR_LEFT) return (v2){-1, 0};
    if (dir == DIR_RIGHT) return (v2){1, 0};
    if (dir == DIR_NONE) return (v2){0, 0};

    return (v2){0, 0};
}

// Convierte la posicion en el grid del juego a una coordenada en pixeles de la pantalla
inline v2 game_to_screen_pos(v2 game_pos, Game_state *state) 
{
    return (v2) { game_pos.x * state->cell_size + state->screen_rect.x, game_pos.y * state->cell_size + state->screen_rect.y};
}

// TODO: Implementar esto bien...
void add_input(Game_state* state, DIRECTION dir)
{
    DIRECTION last_dir = DIR_NONE;

    // TODO: @test iluminar los botones que se presionan
    if (state->accept_input) state->input_dir = dir;

    if (state->direction_buffer[0] == DIR_NONE && state->direction_buffer[1] == DIR_NONE)
    {
        last_dir = direction_vector_enum(state->snake[0].direction);
    }
    else {
        last_dir = state->direction_buffer[1];
    }

    if (state->accept_input && dir != last_dir && (state->direction_buffer[0] == DIR_NONE || state->direction_buffer[1] == DIR_NONE))
    {
        v2 last_dir_opposite = direction_enum_vector(last_dir);
        v2 new_dir = direction_enum_vector(dir);

        if (new_dir.x != -last_dir_opposite.x && new_dir.y != -last_dir_opposite.y)
        {
            if (state->direction_buffer[1] == DIR_NONE)
            {
                state->direction_buffer[1] = dir;
            }
            else {
                state->direction_buffer[0] = state->direction_buffer[1];
                state->direction_buffer[1] = dir;
            }
        }
    }
}

// TODO: Codigo bastante feo
DIRECTION get_input(Game_state* state)
{
    DIRECTION dir = DIR_NONE;

    if (state->direction_buffer[0] != DIR_NONE && state->direction_buffer[1] != DIR_NONE)
    {
        dir = state->direction_buffer[0];
        state->direction_buffer[0] = state->direction_buffer[1];
        state->direction_buffer[1] = DIR_NONE;

        return dir;
    }

    if (state->direction_buffer[0] == DIR_NONE && state->direction_buffer[1] != DIR_NONE)
    {
        dir = state->direction_buffer[1];
        state->direction_buffer[1] = DIR_NONE;

        return dir;
    }

    if (state->direction_buffer[0] != DIR_NONE && state->direction_buffer[1] == DIR_NONE)
    {
        dir = state->direction_buffer[0];
        state->direction_buffer[0] = DIR_NONE;

        return dir;
    }

    return dir;
}

bool is_over(Rect rect, int32_t x, int32_t y)
{
    bool over = true;

    // Posicion del punto con respecto al rectangulo
    if (x < rect.x ||                // el mouse esta a la izquierda
        x > rect.x + rect.width ||      // a la derecha
        y < rect.y ||                // arriba
        y > rect.y + rect.height)        // abajo
        over = false;

    // Si no esta en ninguna de esas posiciones, solo puede estar sobre el boton.
    return over;
}

int log_init(Game_state *state)
{
	// Set the log priority to show debug logs, change later only for errors?
#if defined(NDEBUG)
    SDL_LogSetPriority(CATEGORY_GAME_SNAKE, SDL_LOG_PRIORITY_DEBUG);
    SDL_LogSetPriority(CATEGORY_ENGINE_SNAKE, SDL_LOG_PRIORITY_DEBUG);
#else
	SDL_LogSetPriority(CATEGORY_GAME_SNAKE, SDL_LOG_PRIORITY_ERROR);
	SDL_LogSetPriority(CATEGORY_ENGINE_SNAKE, SDL_LOG_PRIORITY_ERROR);
#endif

    return 1;
}

void shutdown_game(Engine *engine)
{
	shutdown_engine(engine);
	Mix_CloseAudio();
    return;
}
