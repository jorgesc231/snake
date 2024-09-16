#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#define _IMPLEMENT_TRANSLATE_
#include "translate.h"

#include "snake.h"
#include "assets_loader.h"
#include "renderer.h"
#include "gui.h"

Mix_Chunk* sounds[SND_MAX];

Renderer renderer;
AssetManager assets;
Game_state state;

uint64_t prev_time = SDL_GetTicks64();

int main(int argc, char* args[])
{
	log_init(&state);

    init_renderer(&renderer);
    init_asset_manager(&assets);

    // TODO: Implement a better way of manage errors
    state.quit = !init_game(&renderer, &state);

#if !defined(__EMSCRIPTEN__)

    while (!state.quit)
    {
        main_loop();
    }
    
    shutdown_game(&renderer);
#endif


#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(main_loop, 0, 0);
#endif


    return EXIT_SUCCESS;
}


void main_loop()
{    
    // The code calculates the elapsedTime in seconds since the last time this code was executed.
    uint64_t current_time = SDL_GetTicks64();
    float elapsed_time = (float)(current_time - prev_time) / 1000.0f;
    prev_time = current_time;  // Prepare for the next frame

    // Process Events
    process_events(&renderer, &state);

    update_game(&state, elapsed_time);

    render_game(&renderer, &state);
        
    print_gles_errors();

    draw_gui(&renderer, &state);

    SDL_GL_SwapWindow(renderer.window);

    //SDL_LogDebug(CATEGORY_GAME_SNAKE, "Current FPS: %f", 1.0f / elapsed_time);
}

int32_t init_game(Renderer *renderer, Game_state *state)
{
	get_system_language(state);

    if (renderer->device_type == PHONE)
    {
        state->controller_type = CONTROLLER_TOUCH;
        state->selected_level = LEVEL_PHONE;

        if (renderer->DISP_WIDTH > renderer->DISP_HEIGHT)
        {
            state->score_bar_size = PC_SCORE_BAR_SIZE;
            renderer->scale_factor = 1;
            renderer->font_large_size = PC_LARGE_FONT_SIZE;
            renderer->font_main_size = PC_MAIN_FONT_SIZE;
            renderer->font_title_size = PC_TITLE_FONT_SIZE;
        }
        else
        {
            state->score_bar_size = PHONE_SCORE_BAR_SIZE;
            renderer->scale_factor = 2;
            renderer->font_large_size = PHONE_LARGE_FONT_SIZE;
            renderer->font_main_size = PHONE_MAIN_FONT_SIZE;
            renderer->font_title_size = PHONE_TITLE_FONT_SIZE;
        }
    }
    else
    {
        state->controller_type = CONTROLLER_KEYBOARD;
        state->selected_level = LEVEL_DEFAULT;

        state->score_bar_size = PC_SCORE_BAR_SIZE;
        renderer->scale_factor = 1;
        renderer->font_large_size = PC_LARGE_FONT_SIZE;
        renderer->font_main_size = PC_MAIN_FONT_SIZE;
        renderer->font_title_size = PC_TITLE_FONT_SIZE;
    }

    init_levels (state);

    // TODO: solucion temporal...
#if !defined(__EMSCRIPTEN__)
    load_state_file(&assets, CONFIG_FILE_NAME, state);
#endif

    change_level(state, state->selected_level);

    calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);

    print_gles_errors();

    // Load Texture and Shaders
    renderer->textures[0] = texture_load(&assets, "graphics/atlas.png");

    print_gles_errors();

    renderer->shaders[TEXTURE_SHADER] = shader_prog_load(&assets, "shaders/texture.vertex", "shaders/texture.fragment");

    print_gles_errors();

    // TODO: No se ejecutan estos asserts a pesar de estar definido NDEBUG...
#if defined(NDEBUG)
    assert(renderer->textures[0]);
    assert(renderer->shaders[TEXTURE_SHADER]);
#endif

    if (!renderer->textures[0] || !renderer->shaders[TEXTURE_SHADER])
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Could't load game assets.", NULL);
        return 0;
    }

    glUseProgram(renderer->shaders[TEXTURE_SHADER]);

    init_main_shader_attribs(renderer);

    // Get the uniform location
    renderer->AttribLocationProjMtx = glGetUniformLocation(renderer->shaders[TEXTURE_SHADER], "u_Projection");
    if (renderer->AttribLocationProjMtx < 0)
    {
        SDL_LogError(CATEGORY_GAME_SNAKE, "ERROR: Couldn't get \"MVP\" uniform location\n");
        assert(0);
    }

    glBindTexture(GL_TEXTURE_2D, renderer->textures[0]);
    
    GLint tex_sampler_uniform_loc = glGetUniformLocation(renderer->shaders[TEXTURE_SHADER], "texSampler");
    if (tex_sampler_uniform_loc < 0)
    {
        SDL_LogError(CATEGORY_GAME_SNAKE, "ERROR: Couldn't get \"texSampler\" uniform location\n");
        assert(0);
    }
    
    // Set Uniform to texture unit 0
    glUniform1i(tex_sampler_uniform_loc, 0);
    
    // Inicializa el audio
    if (init_audio(sounds, SND_MAX))
    {
        // TODO: Reportar errores si no se puede cargar el audio...
        sounds[SND_SNAKE_EAT] = Mix_LoadWAV(get_data_path(&assets, "sound/food.mp3"));
        sounds[SND_INTERACTION] = Mix_LoadWAV(get_data_path(&assets, "sound/move.mp3"));
        sounds[SND_SNAKE_DIE] = Mix_LoadWAV(get_data_path(&assets, "sound/gameover_short.mp3"));

        state->audio_loaded = sounds[SND_SNAKE_EAT] && sounds[SND_INTERACTION] && sounds[SND_SNAKE_DIE];
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
    font_data = load_font(&assets, "graphics/Bungee-Regular.ttf", font_data, &font_data_size);

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
    renderer->font_debug = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 18.0f, &font_cfg);
    IM_ASSERT(renderer->font_debug != NULL);
#endif

    renderer->font_main = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, renderer->font_main_size, &font_cfg);
    renderer->font_large = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, renderer->font_large_size, &font_cfg);
    renderer->font_title = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, renderer->font_title_size, &font_cfg);


    IM_ASSERT(renderer->font_main != NULL);
    IM_ASSERT(renderer->font_large != NULL);
    IM_ASSERT(renderer->font_title != NULL);
    
    set_game_state(state);
    update_snake(state);

    return 1;
}

void process_events(Renderer *renderer, Game_state *state)
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
        	write_state_file(&assets, CONFIG_FILE_NAME, state);
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
        	write_state_file(&assets, CONFIG_FILE_NAME, state);
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

                    if (event.window.data1 != renderer->DISP_WIDTH || event.window.data2 != renderer->DISP_HEIGHT)
                    {
                        renderer->DISP_WIDTH = event.window.data1;
                        renderer->DISP_HEIGHT = event.window.data2;

                        glViewport(0, 0, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);

                        init_camera_2d(&renderer->camera, renderer->DISP_WIDTH, renderer->DISP_HEIGHT, glm::vec2(0, 0));

                        calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
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
                	x = (int32_t)(event.tfinger.x * renderer->DISP_WIDTH);
                	y = (int32_t)(event.tfinger.y * renderer->DISP_HEIGHT);
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
              	if (state->audio_enabled) Mix_PlayChannel(CH_INTERACTION, sounds[SND_INTERACTION], 0);

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
    	write_state_file(&assets, CONFIG_FILE_NAME, state);
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

    // Actualizacion de estado del juego

    if (state->time > state->time_step && state->status == PLAY)
    {
        update_snake(state);
        state->time = 0;
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
            if (state->audio_enabled)   Mix_PlayChannel(CH_GAME_OVER, sounds[SND_SNAKE_DIE], 0);
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
        state->tail_counter += 1;
        state->score += 1;
        state->food_active = false;

        // TODO: Mejorar!!!
        int32_t* highest_score = &state->levels[state->selected_level].hight_score_by_difficulty[state->difficulty];
        if (state->score > *highest_score) *highest_score = state->score;

        if (state->audio_enabled) Mix_PlayChannel(CH_EAT, sounds[SND_SNAKE_EAT], 0);

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

            // TODO: en el caso de la cola lee memoria anterior
            state->snake[i].position = state->snake[i - 1].position;
            state->snake[i].direction = state->snake[i - 1].direction;
        }

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

void render_game(Renderer *renderer, Game_state *state)
{
    // Init the main batch
    begin_batch(&renderer->main_batch);

    // add operations to the batch

    // Draw the score bar background
    create_quad(&renderer->main_batch, state->score_rect, NO_TEXTURE, glm::vec4(0.07f, 0.09f, 0.07f, 1.0f));

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
                        create_quad(&renderer->main_batch, map_quad, NO_TEXTURE, renderer->cell_color2);
                    } else {
                        create_quad(&renderer->main_batch, map_quad, NO_TEXTURE, renderer->cell_color1);
                    }
                } else {
                    if (x % 2 == 0) {
                        create_quad(&renderer->main_batch, map_quad, NO_TEXTURE, renderer->cell_color1);
                    } else {
                        create_quad(&renderer->main_batch, map_quad, NO_TEXTURE, renderer->cell_color2);
                    }
                }
            }
            else if (state->game_skin == SKIN_RARE)
            {
                if (y % 2 == 0)
                {
                    if (x % 2 == 0) {
                        create_quad(&renderer->main_batch, map_quad, RARE_GRASS_CLEAR, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    } else {
                    	create_quad(&renderer->main_batch, map_quad, RARE_GRASS_DARK, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    }
                } else {
                    if (x % 2 == 0) {
                    	create_quad(&renderer->main_batch, map_quad, RARE_GRASS_DARK, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    } else {
                    	create_quad(&renderer->main_batch, map_quad, RARE_GRASS_CLEAR, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    }
                }
            }
            else if (state->game_skin == SKIN_MINIMAL)
            {
            	create_quad(&renderer->main_batch, map_quad, NO_TEXTURE, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }
            else if (state->game_skin == SKIN_RETRO)
            {
                create_quad(&renderer->main_batch, map_quad, RETRO_CELL, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
#if 0
            else if (state->game_skin == SKIN_CLASSIC)
            {
            	create_quad(&renderer->main_batch, map_quad, NO_TEXTURE, glm::vec4(0.77f, 0.81f, 0.64f, 1.0f));
            }
#endif
        }
    }

    // Draw the Snake
    for (int32_t i = 0; i < state->tail_counter; i++)
    {
        Quad snake_quad = (Quad) {state->snake[i].position.x * state->cell_size + state->screen_rect.x, state->snake[i].position.y * state->cell_size + state->screen_rect.y, state->cell_size, state->cell_size};
        SPRITE_ID sprite = NO_TEXTURE;

            if (state->game_skin == SKIN_MINIMAL)
            {
                if (i != 0) create_quad(&renderer->main_batch, snake_quad, NO_TEXTURE, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
                else create_quad(&renderer->main_batch, snake_quad, NO_TEXTURE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            }
#if 0
            else if (state->game_skin == SKIN_CLASSIC)
            {
                create_quad(&renderer->main_batch, snake_quad, NO_TEXTURE, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
            }
#endif
            else if (state->game_skin == SKIN_RETRO)
            {
                create_quad(&renderer->main_batch, snake_quad, RETRO_BODY, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
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

                        if (state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1)
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? TAIL_DOWN : RARE_TAIL_DOWN;
                        }

                        if (state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1)
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? TAIL_UP : RARE_TAIL_UP;
                        }

                        if (state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0)
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? TAIL_LEFT : RARE_TAIL_LEFT;
                        }

                        if (state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0)
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? TAIL_RIGHT : RARE_TAIL_RIGHT;
                        }
                    }
                    else  // Cuerpo
                    {
                        if (state->snake[i].direction.x == 0 && (state->snake[i].direction.y == -1 || state->snake[i].direction.y == 1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_VERTICAL : RARE_BODY_VERTICAL;
                        }

                        if (state->snake[i].direction.y == 0 && (state->snake[i].direction.x == -1 || state->snake[i].direction.x == 1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_HORIZONTAL : RARE_BODY_HORIZONTAL;
                        }

                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1) && (state->snake[i + 1].direction.x == 1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMLEFT : RARE_BODY_BOTTOMLEFT;
                        }

                        if ((state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == 1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPRIGHT : RARE_BODY_TOPRIGHT;
                        }

                        if ((state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == 1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPLEFT : RARE_BODY_TOPLEFT;
                        }

                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1) && (state->snake[i + 1].direction.x == 1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPLEFT : RARE_BODY_TOPLEFT;
                        }

                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == -1) && (state->snake[i + 1].direction.x == -1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_TOPRIGHT : RARE_BODY_TOPRIGHT;
                        }

                        if ((state->snake[i].direction.x == 1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == -1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMRIGHT : RARE_BODY_BOTTOMRIGHT;
                        }

                        if ((state->snake[i].direction.x == -1 && state->snake[i].direction.y == 0) && (state->snake[i + 1].direction.x == 0 && state->snake[i + 1].direction.y == -1))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMLEFT : RARE_BODY_BOTTOMLEFT;
                        }

                        if ((state->snake[i].direction.x == 0 && state->snake[i].direction.y == 1) && (state->snake[i + 1].direction.x == -1 && state->snake[i + 1].direction.y == 0))
                        {
                            sprite = state->game_skin == SKIN_DEFAULT ? BODY_BOTTOMRIGHT : RARE_BODY_BOTTOMRIGHT;
                        }

                    }
                }

                create_quad(&renderer->main_batch, snake_quad, sprite, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
    }


     // Draw the Food
    Quad food_quad = (Quad) {state->food_pos.x * state->cell_size + state->screen_rect.x, state->food_pos.y * state->cell_size + state->screen_rect.y, state->cell_size, state->cell_size};
    if (state->game_skin == SKIN_DEFAULT)
    {
        create_quad(&renderer->main_batch, food_quad, APPLE, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    }
    else if (state->game_skin == SKIN_RARE)
    {
        create_quad(&renderer->main_batch, food_quad, FRUIT, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    else if (state->game_skin == SKIN_MINIMAL)
    {
        create_quad(&renderer->main_batch, food_quad, NO_TEXTURE, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
#if 0
    else if (state->game_skin == SKIN_CLASSIC)
    {
        create_quad(&renderer->main_batch, food_quad, NO_TEXTURE, glm::vec4(0.60f, 0.60f, 0.60f, 1.0f));

        Quad food_center_quad = (Quad){(int32_t)(food_quad.x + food_quad.width * 0.10f), (int32_t)(food_quad.y + food_quad.height * 0.10f), (int32_t)(food_quad.width * 0.8f), (int32_t)(food_quad.height * 0.8f)};
        create_quad(&renderer->main_batch, food_center_quad, NO_TEXTURE, glm::vec4(0.87f, 0.87f, 0.87f, 1.0f));

    }
#endif
    else if (state->game_skin == SKIN_RETRO)
    {
        create_quad(&renderer->main_batch, food_quad, RETRO_FOOD, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    draw_touch_controls(state, renderer);

    
    // Set dynamic vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderer->main_batch.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->main_batch.vertex_count * sizeof(Vertex), renderer->main_batch.vertices);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->main_batch.index_buffer);
    
    glUseProgram(renderer->shaders[TEXTURE_SHADER]);

    glBindTexture(GL_TEXTURE_2D, renderer->textures[0]);

    init_main_shader_attribs(renderer);

    // Copy the projection matrix to the GPU
    glUniformMatrix4fv(renderer->AttribLocationProjMtx, 1, GL_FALSE, &renderer->camera.projection[0][0]);

    // TODO: Esto no deberia ser asi...
    if (state->game_skin == SKIN_RETRO)
    {
        glClearColor(0.35f, 0.37f, 0.26f, 1.0f);
    }
    else if (state->game_skin == SKIN_MINIMAL)
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }
    else
    {
        glClearColor(renderer->clear_color.r, renderer->clear_color.g, renderer->clear_color.b, renderer->clear_color.a);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    print_gles_errors();
    
    // NOTA: OpenGL ES 2.0 solo soporta indices de tipo SHORT, 3.0 soporta int.
    glDrawElements(GL_TRIANGLES, renderer->main_batch.index_count, GL_UNSIGNED_SHORT, NULL);

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

// TODO: Uses the global state and renderer @Fix
void shutdown_game(Renderer *renderer)
{
	shutdown_renderer(renderer);
	Mix_CloseAudio();
    return;
}
