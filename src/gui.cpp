#include "gui.h"
#include "assets_loader.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "translate.h"

// ImGuiCond_FirstUseEver = if window has not data in .ini file
// ImGuiCond_Once = once per session

void calculate_gui(Game_state *state, int32_t screen_width, int32_t screen_height)
{
    state->screen_rect = (Rect) {0, 0, screen_width, screen_height};

    state->score_rect = cut_top(&state->screen_rect, state->score_bar_size);

    float target_aspect_ratio = state->columns / (float)state->rows;

    int32_t aspectWidth = state->screen_rect.width;
    int32_t aspectHeight = aspectWidth / target_aspect_ratio;

    if (aspectHeight > state->screen_rect.height)
    {
        // We must switch to pillarbox mode (barras a los lados)
        aspectHeight = state->screen_rect.height;
        aspectWidth = aspectHeight * target_aspect_ratio;
    }

    state->cell_size = aspectHeight / state->rows;

    create_touch_controls(state);

    int32_t diff =  screen_width - state->cell_size * state->columns;

    // Center the score bar and the map
    cut_left(&state->screen_rect, diff * 0.5f);
    cut_right(&state->screen_rect, diff * 0.5f);

    // Limit the minimum size of the score bar
    if (diff > screen_width * 0.5f) diff = screen_width * 0.5f;
    cut_left(&state->score_rect, diff * 0.5f);
    cut_right(&state->score_rect, diff * 0.5f);
}

void create_touch_controls(Game_state *state)
{
    int screen_controller_size = 0;

    Rect *up_button = &state->touch_input_rects[TOUCH_INPUT_UP];
    Rect *down_button = &state->touch_input_rects[TOUCH_INPUT_DOWN];
    Rect *left_button = &state->touch_input_rects[TOUCH_INPUT_LEFT];
    Rect *right_button = &state->touch_input_rects[TOUCH_INPUT_RIGHT];

    int32_t width = state->screen_rect.width;
    int32_t height = state->screen_rect.height;

    screen_controller_size = width - state->cell_size * state->columns;

    if (state->controller_type == CONTROLLER_TOUCH)
    {
        // Landscape
        if (width >= height)
        {
            if (screen_controller_size < width * 0.4) screen_controller_size = width * 0.4;

            Rect left_pad = get_left(&state->screen_rect, screen_controller_size * 0.5f);
            Rect right_pad = get_right(&state->screen_rect, screen_controller_size * 0.5f);

            int margin = (left_pad.height * 0.25);
            cut_top(&left_pad, margin);
            cut_bottom(&left_pad, margin);

            cut_top(&right_pad, margin);
            cut_bottom(&right_pad, margin);

            *up_button = get_top(&left_pad, left_pad.height * 0.49f);
            *down_button = get_bottom(&left_pad, left_pad.height * 0.49f);
            *left_button = get_left(&right_pad, right_pad.width * 0.49f);
            *right_button = get_right(&right_pad, right_pad.width * 0.49f);
        }
        else // Portrait
        {
            screen_controller_size = height * 0.5;
            Rect controller_rect = get_bottom(&state->screen_rect, screen_controller_size);

            //int margin = controller_rect.width * 0.02;
            int margin = controller_rect.width * 0.1;
            cut_left(&controller_rect, margin);
            cut_right(&controller_rect, margin);

            margin = controller_rect.height * 0.1;
            cut_bottom(&controller_rect, margin);
            cut_bottom(&controller_rect, margin);

            Rect left_pad = get_left(&controller_rect, controller_rect.width * 0.49f);
            Rect right_pad = get_right(&controller_rect, controller_rect.width * 0.49f);

            *up_button = get_top(&left_pad, left_pad.height * 0.49f);
            *down_button = get_bottom(&left_pad, left_pad.height * 0.49f);
            *left_button = get_left(&right_pad, right_pad.width * 0.49f);
            *right_button = get_right(&right_pad, right_pad.width * 0.49f);
        }
    }

    if (state->controller_type == CONTROLLER_TOUCH2)
    {
        // Landscape
        if (width >= height)
        {
            if (screen_controller_size < width * 0.4) screen_controller_size = width * 0.4;

            Rect left_pad = get_left(&state->screen_rect, screen_controller_size * 0.5f);
            Rect right_pad = get_right(&state->screen_rect, screen_controller_size * 0.5f);

            int margin = (left_pad.height * 0.25f);
            cut_top(&left_pad, margin);
            cut_bottom(&left_pad, margin);

            cut_top(&right_pad, margin);
            cut_bottom(&right_pad, margin);

            *up_button = get_top(&left_pad, left_pad.height * 0.49f);
            *left_button = get_bottom(&left_pad, left_pad.height * 0.49f);
            *down_button = get_top(&right_pad, right_pad.height * 0.49f);
            *right_button = get_bottom(&right_pad, right_pad.height * 0.49f);
        }
        else // Portrait
        {
            screen_controller_size = height * 0.45;
            Rect controller_rect = get_bottom(&state->screen_rect, screen_controller_size);

            int margin = controller_rect.height * 0.1f;
            cut_bottom(&controller_rect, margin);
            cut_right(&controller_rect, margin);
            cut_left(&controller_rect, margin);

            Rect left_pad = cut_left(&controller_rect, controller_rect.width * 0.33f);
            Rect middle_pad = cut_left(&controller_rect, controller_rect.width * 0.5f);
            Rect right_pad = controller_rect;

            *up_button = cut_top(&middle_pad, middle_pad.height * 0.5f);
            cut_bottom(up_button, 5);
            cut_top(&middle_pad, 5);
            *down_button = middle_pad;

            *left_button = left_pad;
            cut_right(left_button, 10);
            *right_button = right_pad;
            cut_left(right_button, 10);
        }
    }

    if (state->controller_type == CONTROLLER_TOUCH3)
    {
        *up_button = (Rect) {0,0, 0, 0};
        *down_button = (Rect) {0,0, 0, 0};
        *left_button = (Rect) {0,0, 0, 0};
        *right_button = (Rect) {0,0, 0, 0};

        // Landscape
        if (width >= height)
        {
            if (screen_controller_size < width * 0.4) screen_controller_size = width * 0.4;

            Rect left_pad = get_left(&state->screen_rect, screen_controller_size * 0.5f);
            Rect right_pad = get_right(&state->screen_rect, screen_controller_size * 0.5f);

            int margin = (left_pad.height * 0.25f);
            cut_top(&left_pad, margin);
            cut_bottom(&left_pad, margin);

            cut_top(&right_pad, margin);
            cut_bottom(&right_pad, margin);

            calculate_pad_triangles(state, left_pad);
        }
        else
        {
            screen_controller_size = height * 0.45;
            Rect controller_rect = get_bottom(&state->screen_rect, screen_controller_size);

            int margin = controller_rect.width * 0.05f;
            cut_left(&controller_rect, margin);
            cut_right(&controller_rect, margin);

            margin = controller_rect.height * 0.15f;
            cut_bottom(&controller_rect, margin);

            calculate_pad_triangles(state, controller_rect);
        }
    }
}

// Show touch controls on the screen
void draw_touch_controls(Game_state *state, Renderer *renderer)
{
    float opacity = 0.25f;
    float opacity_pressed = 0.4f;

    glm::vec4 color_pressed = glm::vec4(0.22f, 0.22f, 0.22f, opacity_pressed);
    glm::vec4 color_released = glm::vec4(0.22f, 0.22f, 0.22f, opacity);

    if (state->controller_type == CONTROLLER_TOUCH || state->controller_type == CONTROLLER_TOUCH2)
    {
        Rect *up_button = &state->touch_input_rects[TOUCH_INPUT_UP];
        Rect *down_button = &state->touch_input_rects[TOUCH_INPUT_DOWN];
        Rect *left_button = &state->touch_input_rects[TOUCH_INPUT_LEFT];
        Rect *right_button = &state->touch_input_rects[TOUCH_INPUT_RIGHT];

        // TODO: Hardcoded
        int arrow_size = 64 * renderer->scale_factor;
        Rect tex_rect = *up_button;
        cut_top(&tex_rect, (up_button->height - arrow_size) * 0.5f);
        cut_bottom(&tex_rect, (up_button->height - arrow_size) * 0.5f);
        cut_left(&tex_rect, (up_button->width - arrow_size) * 0.5f);
        cut_right(&tex_rect, (up_button->width - arrow_size) * 0.5f);

        // Coloca la textura de las flechas en los botones
        if (state->input_dir == DIR_UP && state->accept_input)
        {
            create_quad(&renderer->main_batch, *up_button, NO_TEXTURE, color_pressed);
            create_quad(&renderer->main_batch, tex_rect, UI_UP_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity_pressed));
        }
        else
        {
            create_quad(&renderer->main_batch, *up_button, NO_TEXTURE, color_released);
            create_quad(&renderer->main_batch, tex_rect, UI_UP_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity));
        }

        tex_rect = *down_button;
        cut_top(&tex_rect, (down_button->height - arrow_size) * 0.5f);
        cut_bottom(&tex_rect, (down_button->height - arrow_size) * 0.5f);
        cut_left(&tex_rect, (down_button->width - arrow_size) * 0.5f);
        cut_right(&tex_rect, (down_button->width - arrow_size) * 0.5f);

        if (state->input_dir == DIR_DOWN && state->accept_input)
        {
            create_quad(&renderer->main_batch, *down_button, NO_TEXTURE, glm::vec4(0.22f, 0.22f, 0.22f, opacity_pressed));
            create_quad(&renderer->main_batch, tex_rect, UI_DOWN_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity_pressed));
        }
        else
        {
            create_quad(&renderer->main_batch, *down_button, NO_TEXTURE, color_released);
            create_quad(&renderer->main_batch, tex_rect, UI_DOWN_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity));
        }

        tex_rect = *left_button;
        cut_top(&tex_rect, (left_button->height - arrow_size) * 0.5f);
        cut_bottom(&tex_rect, (left_button->height - arrow_size) * 0.5f);
        cut_left(&tex_rect, (left_button->width - arrow_size) * 0.5f);
        cut_right(&tex_rect, (left_button->width - arrow_size) * 0.5f);

        if (state->input_dir == DIR_LEFT && state->accept_input)
        {
            create_quad(&renderer->main_batch, *left_button, NO_TEXTURE, color_pressed);
            create_quad(&renderer->main_batch, tex_rect, UI_LEFT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity_pressed));
        }
        else
        {
            create_quad(&renderer->main_batch, *left_button, NO_TEXTURE, color_released);
            create_quad(&renderer->main_batch, tex_rect, UI_LEFT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity));
        }

        tex_rect = *right_button;
        cut_top(&tex_rect, (right_button->height - arrow_size) * 0.5f);
        cut_bottom(&tex_rect, (right_button->height - arrow_size) * 0.5f);
        cut_left(&tex_rect, (right_button->width - arrow_size) * 0.5f);
        cut_right(&tex_rect, (right_button->width - arrow_size) * 0.5f);

        if (state->input_dir == DIR_RIGHT && state->accept_input)
        {
            create_quad(&renderer->main_batch, *right_button, NO_TEXTURE, color_pressed);
            create_quad(&renderer->main_batch, tex_rect, UI_RIGHT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity_pressed));
        }
        else
        {
            create_quad(&renderer->main_batch, *right_button, NO_TEXTURE, color_released);
            create_quad(&renderer->main_batch, tex_rect, UI_RIGHT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, opacity));
        }
    }

    if (state->controller_type == CONTROLLER_TOUCH3)
    {
        draw_pad_triangles(state, &renderer->main_batch, glm::vec4(0.22f, 0.22f, 0.22f, opacity), glm::vec4(0.22f, 0.22f, 0.22f, opacity_pressed));
    }
}

void draw_gui(Renderer *renderer, Game_state *state) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();


    draw_score(renderer, state);
        
    if (state->status != PLAY && state->show_status_screen) draw_status(renderer, state);
        
#if defined(NDEBUG)
    if (state->show_demo_window) ImGui::ShowDemoWindow((bool *)&state->show_demo_window);

    if (state->show_debug_overlay)
    {
        draw_debug_overlay(renderer, state);
        draw_debug_window(renderer, state);
    }
#endif

    print_gles_errors();
        

    // Rendering de gui
    ImGui::Render();
    //ImGuiIO& io = ImGui::GetIO();
    //glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    //glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    print_gles_errors();
}

void draw_score(Renderer *renderer, Game_state *state)
{
    AtlasSprite apple_sprite = DescAtlas[APPLE];
    AtlasSprite trophy_sprite = DescAtlas[UI_TROPHY];
    AtlasSprite pause_sprite = DescAtlas[UI_PAUSE];
    AtlasSprite sound_sprite = DescAtlas[UI_AUDIO];
    AtlasSprite mute_sprite = DescAtlas[UI_NO_AUDIO];

    ImGui::PushFont(renderer->font_main);

    //ImGui::GetTextLineHeight();
    int32_t spacing = 10;
    ImVec2 text_size = ImGui::CalcTextSize("100");
    int32_t trophy_size = text_size.y;
    int32_t apple_size = text_size.y;

    switch (state->game_skin)
    {
    	case SKIN_RARE:
    		apple_sprite = DescAtlas[FRUIT];
            apple_size = text_size.y * 1.50f;
    		break;
    	case SKIN_RETRO:
    		apple_sprite = DescAtlas[RETRO_FOOD];
    		break;
    	default:
    		apple_sprite = DescAtlas[APPLE];
            apple_size = text_size.y * 1.50f;
    		break;
    }

    ImGui::SetNextWindowPos(ImVec2(state->score_rect.x, state->score_rect.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(state->score_rect.width, state->score_rect.height));

    // ImGuiWindowFlags_NoInputs
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings;
    
    //ImGui::SetNextWindowBgAlpha(0.4f); // Transparent background
    ImGui::Begin("Score", NULL, window_flags);


    // TODO: Temporal
    Rect score_rect = state->score_rect;

    int margin = score_rect.height - text_size.y;
    cut_top(&score_rect, margin * 0.5f);
    cut_bottom(&score_rect, margin * 0.5f);

    Rect highest_point = cut_left(&score_rect, state->score_rect.width * 0.25f);
    Rect buttons = cut_right(&score_rect, state->score_rect.width * 0.3f);


    int32_t diff = highest_point.width - (int32_t )text_size.x + trophy_size;
    cut_left(&highest_point, diff * 0.5f);
    cut_right(&highest_point, diff * 0.5f);

    diff = score_rect.width - ((int32_t )text_size.x + apple_size + spacing);
    cut_left(&score_rect, diff * 0.5f);
    cut_right(&score_rect, diff * 0.5f);

    ImGui::SetCursorScreenPos(ImVec2(highest_point.x, highest_point.y));

    // Dibuja el sprite del trofeo a lado del puntaje mas alto
    ImGui::Image((void*)(intptr_t)renderer->textures[0],
        ImVec2(trophy_size, trophy_size),
        ImVec2(((trophy_sprite.positionX) / (float)ATLAS_WIDTH), ((trophy_sprite.positionY) / (float)ATLAS_HEIGHT)),
        ImVec2(((trophy_sprite.positionX + trophy_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((trophy_sprite.positionY + trophy_sprite.sourceHeight) / (float)ATLAS_HEIGHT)));

    ImGui::SameLine(0, 20);
    ImGui::Text("%d", state->levels[state->selected_level].hight_score_by_difficulty[state->difficulty]);  // TODO: ?

    // Draw the apple
    ImGui::SetCursorScreenPos(ImVec2(score_rect.x, score_rect.y + score_rect.height * 0.5f - apple_size * 0.5f));

    // Dibuja el sprite de la manzada a lado del puntaje
    ImGui::Image((void*)(intptr_t)renderer->textures[0],
                 ImVec2(apple_size, apple_size),
                 ImVec2(((apple_sprite.positionX) / (float)ATLAS_WIDTH), ((apple_sprite.positionY) / (float)ATLAS_HEIGHT)),
                 ImVec2(((apple_sprite.positionX + apple_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((apple_sprite.positionY + apple_sprite.sourceHeight) / (float)ATLAS_HEIGHT)));


    ImGui::SameLine(0, spacing);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(pos.x, score_rect.y));

    ImGui::Text("%d", state->score);
    //ImGui::PopStyleColor();

    ImGui::PopFont();


    // Colocar botones de opciones
    buttons.height = state->score_rect.height;
    //buttons.y = state->score_rect.y + buttons.height * 0.05f;
    buttons.y = state->score_rect.y;

    ImGui::SetCursorScreenPos(ImVec2(buttons.x, buttons.y));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.09f, 0.07f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    if (state->audio_enabled)
    {
        if (ImGui::ImageButton("Sound", (void*)(intptr_t)renderer->textures[0],
        		ImVec2(buttons.height * 0.9f, buttons.height * 0.9f),
        		ImVec2(((sound_sprite.positionX) / (float)ATLAS_WIDTH), ((sound_sprite.positionY) / (float)ATLAS_HEIGHT)),
    			ImVec2(((sound_sprite.positionX + sound_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((sound_sprite.positionY + sound_sprite.sourceHeight) / (float)ATLAS_HEIGHT)),
    			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
    			) && state->audio_loaded)
        {
        	state->audio_enabled = false;
        }
    }
    else
    {

        if (ImGui::ImageButton("Sound", (void*)(intptr_t)renderer->textures[0],
        		ImVec2(buttons.height * 0.9f, buttons.height * 0.9f),
        		ImVec2(((mute_sprite.positionX) / (float)ATLAS_WIDTH), ((mute_sprite.positionY) / (float)ATLAS_HEIGHT)),
    			ImVec2(((mute_sprite.positionX + mute_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((mute_sprite.positionY + mute_sprite.sourceHeight) / (float)ATLAS_HEIGHT)),
    			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
    			) && state->audio_loaded)
        {
        	state->audio_enabled = true;
        }
    }

    ImGui::SameLine(0, 20);

    if (ImGui::ImageButton("Pause", (void*)(intptr_t)renderer->textures[0],
    		ImVec2(buttons.height * 0.9f, buttons.height * 0.9f),
    		ImVec2(((pause_sprite.positionX) / (float)ATLAS_WIDTH), ((pause_sprite.positionY) / (float)ATLAS_HEIGHT)),
			ImVec2(((pause_sprite.positionX + pause_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((pause_sprite.positionY + pause_sprite.sourceHeight) / (float)ATLAS_HEIGHT)),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
			)
    		)
    {
        if (state->accept_input) state->status = PAUSED;
    }


    ImGui::PopStyleColor(3);

    ImGui::End();
}

void draw_status(Renderer *renderer, Game_state *state) {

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));

    //ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs;
    ImGuiWindowFlags window_flags =  ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings;
    ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
    
    ImGui::Begin("Status", NULL, window_flags);

    // No Acepta entrada al juego en el menu
    state->accept_input = false;

    ImGui::PushFont(renderer->font_large);
    
    if (!state->show_menu)
    {
        ImGui::PushFont(renderer->font_title);

        if (state->status == INIT)
        {
            ImGui::SetCursorPosX( (ImGui::GetWindowWidth() - ImGui::CalcTextSize(get_text(state->selected_language, STR_GAME_TITLE)).x) * 0.5f);
            ImGui::SetCursorPosY(io.DisplaySize.y * 0.25f);
            ImGui::Text("%s", get_text(state->selected_language, STR_GAME_TITLE));
        }

        if (state->status == PAUSED)
        {
            ImGui::SetCursorPosX( (ImGui::GetWindowWidth() - ImGui::CalcTextSize(get_text(state->selected_language, STR_PAUSE)).x) * 0.5f);
            ImGui::SetCursorPosY(io.DisplaySize.y * 0.25f);
            ImGui::Text("%s", get_text(state->selected_language, STR_PAUSE));
        }
        
        if (state->status == LOST)
        {
            ImGui::SetCursorPosX( (ImGui::GetWindowWidth() - ImGui::CalcTextSize(get_text(state->selected_language, STR_LOST)).x) * 0.5f);
            ImGui::SetCursorPosY(io.DisplaySize.y * 0.25f);
            ImGui::Text("%s", get_text(state->selected_language, STR_LOST));
        }

        ImGui::PopFont();

        // TODO: Deberia centralizar el tema...
        // You may modify the ImGui::GetStyle() main instance during initialization and before NewFrame().
        // During the frame, use ImGui::PushStyleVar(ImGuiStyleVar_XXXX)/PopStyleVar() to alter the main style values,
        // and ImGui::PushStyleColor(ImGuiCol_XXX)/PopStyleColor() for colors.
        if (renderer->device_type == PHONE)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(60, 60));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 50));
        }
        else
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(30, 20));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
        }

        ImVec2 text_button_size = ImGui::CalcTextSize(get_text(state->selected_language, STR_BTN_PLAY));
        ImVec2 frame_padding = ImGui::GetStyle().FramePadding;

        text_button_size = ImVec2(text_button_size.x + frame_padding.x * 2, text_button_size.y + frame_padding.y * 2);
        ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - text_button_size.x * 0.5f, ImGui::GetWindowHeight() * 0.55f - text_button_size.y * 0.5f));

        //ImGui::SetFocusID(ImGui::GetID(("Jugar")), ImGui::GetCurrentWindow());
        // Highlight
        //ImGuiContext& g = *ImGui::GetCurrentContext();
        //g.NavDisableHighlight = false;

        if (ImGui::Button(get_text(state->selected_language, STR_BTN_PLAY)))
        {
            if (state->status == LOST)
            {
                set_game_state(state);
                update_snake(state);
            }

            state->status = PLAY;
            state->accept_input = true;
        }

        text_button_size = ImGui::CalcTextSize(get_text(state->selected_language, STR_BTN_OPTIONS));
        text_button_size = ImVec2(text_button_size.x + frame_padding.x * 2, text_button_size.y + frame_padding.y * 2);
        ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - text_button_size.x * 0.5f, ImGui::GetCursorPosY()));

        if (ImGui::Button(get_text(state->selected_language, STR_BTN_OPTIONS)))
        {
            state->show_menu = true;
        }

        ImGui::PopStyleVar(3);

        AtlasSprite sound_sprite = DescAtlas[UI_AUDIO];
        AtlasSprite mute_sprite = DescAtlas[UI_NO_AUDIO];

        // TODO: Temporal
        Rect buttons = get_right(&state->score_rect, state->score_rect.width * 0.3f);
        ImGui::SetCursorPosX(buttons.x);
        ImGui::SetCursorPosY(state->score_rect.y);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.09f, 0.07f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (state->audio_enabled)
        {
            if (ImGui::ImageButton("Sound", (void*)(intptr_t)renderer->textures[0],
            		ImVec2(state->score_rect.height * 0.9f, state->score_rect.height * 0.9f),
            		ImVec2((sound_sprite.positionX / (float)ATLAS_WIDTH), (sound_sprite.positionY / (float)ATLAS_HEIGHT)),
        			ImVec2(((sound_sprite.positionX + sound_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((sound_sprite.positionY + sound_sprite.sourceHeight) / (float)ATLAS_HEIGHT)),
        			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
        			) && state->audio_loaded)
            {
            	state->audio_enabled = false;
            }
        }
        else
        {

            if (ImGui::ImageButton("Sound", (void*)(intptr_t)renderer->textures[0],
            		ImVec2(state->score_rect.height * 0.9f, state->score_rect.height * 0.9f),
            		ImVec2(((mute_sprite.positionX) / (float)ATLAS_WIDTH), ((mute_sprite.positionY) / (float)ATLAS_HEIGHT)),
        			ImVec2(((mute_sprite.positionX + mute_sprite.sourceWidth) / (float)ATLAS_WIDTH), ((mute_sprite.positionY + mute_sprite.sourceHeight) / (float)ATLAS_HEIGHT)),
        			ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
        			) && state->audio_loaded)
            {
            	state->audio_enabled = true;
            }
        }

        ImGui::PopStyleColor(3);

    }
    else
    {
        state->accept_input = false;

        ImVec2 options_window_size;

        // TODO: Deberia buscar otra forma...
        if (renderer->device_type == PHONE)
        {
            options_window_size = ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

            ImGui::SetNextWindowBgAlpha(0.4f);
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() * 0.5f, ImGui::GetWindowHeight() * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 80.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 10.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(40, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 40));
        }
        else
        {
            options_window_size = ImVec2(ImGui::GetWindowWidth() * 0.70f, ImGui::GetWindowHeight() * 0.80f);

            ImGui::SetNextWindowBgAlpha(0.4f);
            // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2.0f, ImGui::GetWindowHeight() / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.46f));

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 10.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 40));
        }

        // ImGuiWindowFlags_AlwaysVerticalScrollbar
        ImGui::BeginChild("Options", options_window_size, ImGuiChildFlags_Border, ImGuiWindowFlags_NoSavedSettings);

        if (renderer->device_type == PHONE)
        {
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
        }

        float spacing = ImGui::GetStyle().ItemInnerSpacing.x + 20;

        {
            // Cambiar skin del juego (Por mejorar...)
            const char* item_names[] = {
            		get_text(state->selected_language, STR_SKIN_DEFAULT),
					get_text(state->selected_language, STR_SKIN_RETRO),
					//get_text(state->selected_language, STR_SKIN_CLASSIC),
					get_text(state->selected_language, STR_SKIN_MINIMAL),
					get_text(state->selected_language, STR_SKIN_RARE),
					get_text(state->selected_language, STR_SKIN_3D),
            };
            int current_item = state->game_skin;

            ImGui::SeparatorText(get_text(state->selected_language, STR_CAT_SKIN));

            int text_sz = ImGui::CalcTextSize(("ABCDEFGHY")).x;
            int total_sz = spacing + text_sz + spacing + ImGui::GetFrameHeight() * 2;
            //int position = ImGui::GetContentRegionAvail().x / 2 - total_sz / 2;
            int position = ImGui::GetWindowSize().x * 0.5f - total_sz * 0.5f;

            ImGui::SetCursorPosX(position);

            if (ImGui::ArrowButton("##left_arrow_skin", ImGuiDir_Left))
            {
                current_item--;

                if (current_item < 0) current_item = GAME_SKIN_COUNT - 1;
            }

            ImGui::SameLine();

            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing + (text_sz * 0.5f - ImGui::CalcTextSize(item_names[current_item]).x * 0.5f));
            ImGui::TextUnformatted(item_names[current_item]);

            ImGui::SameLine();
            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing  + text_sz + spacing );

            if (ImGui::ArrowButton("##right_arrow_skin", ImGuiDir_Right))
            {
                current_item++;

                if (current_item >= GAME_SKIN_COUNT) current_item = 0;
            }

            state->game_skin = (GAME_SKIN)current_item;

			if (renderer->device_type == PHONE) ImGui::Spacing();
        }

        ImGui::SeparatorText(get_text(state->selected_language, STR_CAT_LEVEL));
        {
            const char* item_names[] = {
            		get_text(state->selected_language, STR_LEVEL_DEFAULT),
					get_text(state->selected_language, STR_LEVEL_SMALL),
					get_text(state->selected_language, STR_LEVEL_BIG),
                    get_text(state->selected_language, STR_LEVEL_PHONE)
            };
            int _item = state->selected_level;

            int text_sz = ImGui::CalcTextSize(("ABCDEFGHY")).x;
            int total_sz = spacing + text_sz + spacing + ImGui::GetFrameHeight() * 2;
            int position = ImGui::GetWindowSize().x * 0.5f - total_sz * 0.5f;

            ImGui::SetCursorPosX(position);

            if (ImGui::ArrowButton("##left_arrow_size", ImGuiDir_Left))
            {
                _item--;
                if ( _item < LEVEL_DEFAULT) _item = ((int) LEVEL_COUNT) - 1;

                change_level(state, (GAME_LEVEL)_item);
                update_snake(state);
                // Need to recalculate the layout
                calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
            }

            ImGui::SameLine();

            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing + (text_sz / 2 - ImGui::CalcTextSize(item_names[_item]).x / 2));
            ImGui::TextUnformatted(item_names[_item ]);

            ImGui::SameLine();
            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing  + text_sz + spacing );

            if (ImGui::ArrowButton("##right_arrow_size", ImGuiDir_Right))
            {
                _item++;
                if (_item > LEVEL_COUNT - 1) _item = LEVEL_DEFAULT;

                change_level(state, (GAME_LEVEL)_item);
                update_snake(state);
                // Need to recalculate the layout
                calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
            }

            if (renderer->device_type == PHONE) ImGui::Spacing();
        }

        ImGui::SeparatorText(get_text(state->selected_language, STR_CAT_VELOCITY));

        {
            const char* item_names[] = {
            		get_text(state->selected_language, STR_VEL_SLOW),
					get_text(state->selected_language, STR_VEL_NORMAL),
					get_text(state->selected_language, STR_VEL_FAST),
                    get_text(state->selected_language, STR_VEL_PROGRESSIVE)
            };

            int _item = state->difficulty;

            int text_sz = ImGui::CalcTextSize("ABCDEFGHY").x;
            int total_sz = spacing + text_sz + spacing + ImGui::GetFrameHeight() * 2;
            int position = ImGui::GetWindowSize().x * 0.5f - total_sz * 0.5f;

            ImGui::SetCursorPosX(position);

            if (ImGui::ArrowButton("##left_arrow_timestep", ImGuiDir_Left))
            {
                _item--;

                if ( _item < DIFFICULTY_SLOW) _item = ((int) DIFFICULTY_COUNT) - 1;

                set_game_state(state);
                update_snake(state);
            }

            ImGui::SameLine();

            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing + (text_sz * 0.5f - ImGui::CalcTextSize(item_names[_item]).x * 0.5f));
            ImGui::TextUnformatted(item_names[_item]);

            ImGui::SameLine();
            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing  + text_sz + spacing );

            if (ImGui::ArrowButton("##right_arrow_timestep", ImGuiDir_Right))
            {
                _item++;

                if (_item > DIFFICULTY_COUNT - 1) _item = DIFFICULTY_SLOW;

                set_game_state(state);
                update_snake(state);
            }

            state->difficulty = (DIFFICULTY)_item;

            if (_item == DIFFICULTY_SLOW) state->time_step = DEFAULT_TIMESTEP;
            else if (_item == DIFFICULTY_NORMAL) state->time_step = DEFAULT_NORMAL_TIMESTEP;
            else if (_item == DIFFICULTY_FAST) state->time_step = DEFAULT_FAST_TIMESTEP;
            else if (_item == DIFFICULTY_PROGRESSIVE) state->time_step = DEFAULT_NORMAL_TIMESTEP;

            if (renderer->device_type == PHONE) ImGui::Spacing();
        }


        ImGui::SeparatorText(get_text(state->selected_language, STR_CAT_INPUT));

        {
            const char* item_names[] = {
            		get_text(state->selected_language, STR_INPUT_KEYBOARD),
					get_text(state->selected_language, STR_INPUT_DUALPAD),
					get_text(state->selected_language, STR_INPUT_HPAD),
					get_text(state->selected_language, STR_INPUT_TRIANGLEPAD),
            };
            int _item = state->controller_type;

            int text_sz = ImGui::CalcTextSize("ABCDEFGHY").x;
            int total_sz = spacing + text_sz + spacing + ImGui::GetFrameHeight() * 2;
            int position = ImGui::GetWindowSize().x * 0.5f - total_sz * 0.5f;

            ImGui::SetCursorPosX(position);

            if (ImGui::ArrowButton("##left_arrow_control", ImGuiDir_Left))
            {
                _item--;

                if (_item < 0) _item = CONTROLLER_TYPE_COUNT - 1;

                state->controller_type = (CONTROLLER_TYPE)_item;

                calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
            }

            ImGui::SameLine();

            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing + (text_sz / 2 - ImGui::CalcTextSize(item_names[_item]).x / 2));
            ImGui::TextUnformatted(item_names[_item]);

            ImGui::SameLine();
            ImGui::SetCursorPosX(position + ImGui::GetFrameHeight() + spacing  + text_sz + spacing );

            if (ImGui::ArrowButton("##right_arrow_control", ImGuiDir_Right))
            {
                _item++;

                if (_item > CONTROLLER_TYPE_COUNT - 1) _item = 0;

                state->controller_type = (CONTROLLER_TYPE)_item;

                calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
            }
        }


        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Indent();

        if (ImGui::Button(get_text(state->selected_language, STR_BTN_CLOSE)))
        {
            state->show_menu = false;
        }

        // Closes the options window if click outside the options window
        ImVec2 mouse_pos = ImGui::GetIO().MouseClickedPos[0];
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 window_pos = ImGui::GetWindowPos();

        if (!is_over((Rect){(int)window_pos.x, (int)window_pos.y, (int)window_size.x, (int)window_size.y}, mouse_pos.x, mouse_pos.y))
        {
        	state->show_menu = false;
        }

        ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
        ScrollWhenDraggingOnVoid(ImVec2(0.0f, -mouse_delta.y), ImGuiMouseButton_Left);

        ImGui::EndChild();

        ImGui::PopStyleVar(7);
    }

    ImGui::PopFont();

    ImGui::End();
}

#if defined(NDEBUG)
void draw_debug_window(Renderer *renderer, Game_state *state)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::PushFont(renderer->font_debug);

    //ImGui::SetNextWindowBgAlpha(0.80f); // Transparent background
    ImGui::Begin("Estado del juego", (bool*)&state->show_debug_overlay, window_flags);

    if (ImGui::IsWindowAppearing())
        ImGui::SetWindowFocus();

    // Cambiar idioma
    {
        const char* items[] = {
        		get_text(state->selected_language, STR_SPANISH),
				get_text(state->selected_language, STR_ENGLISH),
        };
        LANGUAGES current_item = state->selected_language;

		if (ImGui::BeginCombo("Language", items[current_item])) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
					state->selected_language = current_item = (LANGUAGES)n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}

			ImGui::EndCombo();
		}
    }

	// Cambiar skin del juego (Por mejorar...)
    {
        const char* items[] = {
        		get_text(state->selected_language, STR_SKIN_DEFAULT),
				get_text(state->selected_language, STR_SKIN_RETRO),
				get_text(state->selected_language, STR_SKIN_CLASSIC),
				get_text(state->selected_language, STR_SKIN_MINIMAL),
				get_text(state->selected_language, STR_SKIN_RARE),
				get_text(state->selected_language, STR_SKIN_3D),
        };
		GAME_SKIN current_item = state->game_skin;

		if (ImGui::BeginCombo("Skin", items[current_item])) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
				{
					current_item = (GAME_SKIN)n;
					state->game_skin = current_item;
				}


				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}

			ImGui::EndCombo();
		}
    }

	// Cambiar velocidad
    {
        const char* items[] = {
        		get_text(state->selected_language, STR_VEL_SLOW),
				get_text(state->selected_language, STR_VEL_NORMAL),
				get_text(state->selected_language, STR_VEL_FAST),
				get_text(state->selected_language, STR_VEL_PROGRESSIVE)
        };
		DIFFICULTY current_item = state->difficulty;

		if (ImGui::BeginCombo("Difficulty", items[current_item])) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
				{
					state->difficulty = current_item = (DIFFICULTY)n;

		            if (current_item == DIFFICULTY_SLOW) state->time_step = DEFAULT_TIMESTEP;
		            else if (current_item == DIFFICULTY_NORMAL) state->time_step = DEFAULT_NORMAL_TIMESTEP;
		            else if (current_item == DIFFICULTY_FAST) state->time_step = DEFAULT_FAST_TIMESTEP;
		            else if (current_item == DIFFICULTY_PROGRESSIVE) state->time_step = DEFAULT_NORMAL_TIMESTEP;
				}


				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}
    }

    // Cambiar Nivel
    {
        const char* items[] = {
        		get_text(state->selected_language, STR_LEVEL_DEFAULT),
				get_text(state->selected_language, STR_LEVEL_SMALL),
				get_text(state->selected_language, STR_LEVEL_BIG),
                get_text(state->selected_language, STR_LEVEL_PHONE),
        };
		GAME_LEVEL current_item = state->selected_level;

		if (ImGui::BeginCombo("Size", items[current_item])) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
				{
					current_item = (GAME_LEVEL)n;
					change_level(state, current_item);
					calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
				}


				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}



			ImGui::EndCombo();
		}
    }

    // Cambiar controles
    {
        const char* items[] = {
        		get_text(state->selected_language, STR_INPUT_KEYBOARD),
				get_text(state->selected_language, STR_INPUT_DUALPAD),
				get_text(state->selected_language, STR_INPUT_HPAD),
                get_text(state->selected_language, STR_INPUT_TRIANGLEPAD),
        };
		CONTROLLER_TYPE current_item = state->controller_type;

		if (ImGui::BeginCombo("Controller", items[current_item])) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
				{
					current_item = (CONTROLLER_TYPE)n;
					state->controller_type = current_item;
					calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}

			ImGui::EndCombo();
		}
    }

    ImGui::Separator();

    ImGui::Text("Game Status:");
    ImGui::SameLine(0, 20);

    if (state->status == INIT)  ImGui::TextColored(ImVec4(0, 255, 0, 1), "INIT");
    if (state->status == PLAY)  ImGui::TextColored(ImVec4(0, 255, 0, 1), "PLAY");
    if (state->status == PAUSED) ImGui::TextColored(ImVec4(255, 0, 255, 1), "PAUSED");
    if (state->status == LOST)  ImGui::TextColored(ImVec4(255, 0, 0, 1), "LOST");

    //if (item_disabled)
    //    ImGui::BeginDisabled(true);
    //
    //if (item_disabled)
    //        ImGui::EndDisabled();

    ImGui::SameLine(0, 40);
    if (ImGui::Button("PLAY", ImVec2(80, 30)))
    {
        if (state->status == LOST) {
            set_game_state(state);
            update_snake(state);
        }

        state->status = PLAY;
        state->accept_input = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("PAUSE", ImVec2(80, 30)))
    {
        state->status = PAUSED;
    }

    ImGui::SameLine();

    if (ImGui::Button("LOST", ImVec2(80, 30)))
    {
        state->status = LOST;
    }

    ImGui::Separator();
    
    ImGui::SliderFloat("Time Step", &state->time_step, 0.1f, 2.0f);

    ImGui::Checkbox("Audio", (bool*)&state->audio_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Status Screen", (bool *)&state->show_status_screen);

    ImGui::Separator();

    ImGui::InputInt("Score", &state->score, 1);

    ImGui::Separator();

    if (ImGui::InputInt("Rows", &state->rows) || ImGui::InputInt("Columns", &state->columns))
    {
    	if (state->rows >= 30) state->rows = 30;
    	if (state->columns >= 30) state->columns = 30;
    	if (state->rows < 10) state->rows = 10;
    	if (state->columns < 10) state->columns = 10;

    	calculate_gui(state, renderer->DISP_WIDTH, renderer->DISP_HEIGHT);
    }

    ImGui::Separator();

    ImGui::InputInt("Food x", &state->food_pos.x);
    ImGui::InputInt("Food y", &state->food_pos.y);

    if (state->food_pos.x >= state->columns) state->food_pos.x = 0;
    if (state->food_pos.x < 0) state->food_pos.x = state->columns - 1;
    if (state->food_pos.y >= state->rows) state->food_pos.y = 0;
    if (state->food_pos.y < 0) state->food_pos.y = state->rows - 1;

    ImGui::Separator();

    ImGui::Text("Input Buffer:");

    static ImGuiTableFlags dir_table_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg;

    if (ImGui::BeginTable("input_buffer_table", 2, dir_table_flags))
    {
        // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 2; row++)
        {
            ImGui::TableNextRow();
            for (int column = 0; column < 2; column++)
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
                        char const *dir_names[5] =  {
                                "DIR_NONE",
                                "DIR_UP",
                                "DIR_DOWN",
                                "DIR_RIGHT",
                                "DIR_LEFT",
                         };

                         ImGui::Text("%s", dir_names[state->direction_buffer[row]]);

                    } break;
                }
            }
         }

        ImGui::EndTable();
    }

    ImGui::Separator();

    ImGui::Text("Snake Buffer:");

    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg;

    if (ImGui::BeginTable("body_table", 4, flags))
    {
        // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Position");
        ImGui::TableSetupColumn("Direction");
        ImGui::TableHeadersRow();

        for (int row = 0; row < state->tail_counter; row++)
        {
            ImGui::TableNextRow();
            for (int column = 0; column < 4; column++)
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
                        else if (row == state->tail_counter - 1) ImGui::Text("TAIL");
                        else ImGui::Text("BODY");

                    } break;

                    case 2:
                    {
                        ImGui::Text("(%d, %d)", state->snake[row].position.x, state->snake[row].position.y);
                    } break;

                    case 3:
                    {
                        ImGui::Text("(%d, %d)", state->snake[row].direction.x, state->snake[row].direction.y);
                    } break;
                }
            }
         }

        ImGui::EndTable();
    }

    ImGui::PopFont();

    ImGui::End();
}

void draw_debug_overlay(Renderer *renderer, Game_state *state)
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
    if (ImGui::Begin("Renderer Debug overlay", (bool*)&state->show_debug_overlay, window_flags))
    {
        if (ImGui::IsWindowAppearing())
            ImGui::SetWindowFocus();

        ImGui::Text("Renderer Debug Overlay");
        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");

        ImGui::Text("Windows Size: (%d, %d) - DPI: %0.1f", renderer->DISP_WIDTH, renderer->DISP_HEIGHT, renderer->dpi);
        //ImGui::Text("Viewport Pos: (x = %d, y = %d) \nViewport Size: (w = %d, h = %d)", state->arena_rect.x, state->arena_rect.y, state->arena_rect.width, state->arena_rect.height);

        ImGui::Separator();

        ImGui::ColorEdit3("Background##2f", &renderer->clear_color[0], ImGuiColorEditFlags_Float);
        ImGui::ColorEdit3("Cells 1##2f", &renderer->cell_color1[0], ImGuiColorEditFlags_Float);
        ImGui::ColorEdit3("Cells 2##2f", &renderer->cell_color2[0], ImGuiColorEditFlags_Float);

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (state->show_debug_overlay && ImGui::MenuItem("Close")) state->show_debug_overlay = false;
            ImGui::EndPopup();
        }
     }
     ImGui::End();
}
#endif


void calculate_pad_triangles (Game_state *state, Rect rect)
{
    int mid_x = rect.x + rect.width * 0.5f;
    int mid_y = rect.y + rect.height * 0.5f;
    int spacing = 15;

    // LEFT
    simple_vertex v1 = (simple_vertex){ rect.x, rect.y + spacing * 2 };
    simple_vertex v2 = (simple_vertex){ rect.x, rect.y + rect.height - spacing * 2 };
    simple_vertex v3 = (simple_vertex){ mid_x - spacing, mid_y };

    Triangle tri = (Triangle){v1, v2, v3};
    state->touch_input_triangles[TOUCH_INPUT_LEFT] = tri;

    // UP
    v1 = (simple_vertex) { rect.x + rect.width, rect.y + spacing };
    v2 = (simple_vertex) { rect.x, rect.y + spacing };
    v3 = (simple_vertex) { mid_x, mid_y - spacing };

    Triangle tri2 = (Triangle){v1, v2, v3};
    state->touch_input_triangles[TOUCH_INPUT_UP] = tri2;

    // DOWN
    v1 = (simple_vertex) { rect.x + rect.width, rect.y + rect.height - spacing };
    v2 = (simple_vertex) { rect.x, rect.y + rect.height - spacing };
    v3 = (simple_vertex) { mid_x, mid_y + spacing };

    Triangle tri3 = (Triangle){v1, v2, v3};
    state->touch_input_triangles[TOUCH_INPUT_DOWN] = tri3;

    // RIGHT
    v1 = (simple_vertex){ rect.x + rect.width, rect.y + spacing * 2};
    v2 = (simple_vertex){ rect.x + rect.width, rect.y + rect.height - spacing * 2 };
    v3 = (simple_vertex){ mid_x + spacing, mid_y };

    Triangle tri4 = (Triangle) {v1, v2, v3};
    state->touch_input_triangles[TOUCH_INPUT_RIGHT] = tri4;
}

void draw_pad_triangles (Game_state *state, Batch *batch, glm::vec4 color, glm::vec4 color_pressed)
{
    Rect texture_rect;
    //int32_t icon_size = v3.y - v1.y;
    // TODO: @Hack
#if defined(__ANDROID__)
    int32_t icon_size = 128;
#else
    int32_t icon_size = 64;
#endif

    // Left
    Triangle *triangle = &state->touch_input_triangles[TOUCH_INPUT_LEFT];

    texture_rect.x = triangle->c.x - (triangle->c.x - triangle->a.x) * 0.5f - icon_size;
    texture_rect.y = triangle->c.y - icon_size * 0.5f;
    texture_rect.width = icon_size;
    texture_rect.height = icon_size;

    if (state->input_dir == DIR_LEFT && state->accept_input)
    {
        create_color_triangle(batch, *triangle, color_pressed);
        create_quad(batch, texture_rect, UI_LEFT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color_pressed.a));
    }
    else
    {
        create_color_triangle(batch, *triangle, color);
        create_quad(batch, texture_rect, UI_LEFT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color.a));
    }

    // Up
    triangle = &state->touch_input_triangles[TOUCH_INPUT_UP];

    texture_rect.x = triangle->c.x - icon_size * 0.5f;
    texture_rect.y = triangle->a.y + (triangle->c.y - triangle->a.y) * 0.5f - icon_size * 0.5f;

    if (state->input_dir == DIR_UP && state->accept_input)
    {
        create_color_triangle(batch, *triangle, color_pressed);
        create_quad(batch, texture_rect, UI_UP_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color_pressed.a));
    }
    else
    {
        create_color_triangle(batch, *triangle, color);
        create_quad(batch, texture_rect, UI_UP_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color.a));
    }

    // DOWN
    triangle = &state->touch_input_triangles[TOUCH_INPUT_DOWN];
    texture_rect.x = triangle->c.x - icon_size * 0.5f;
    texture_rect.y = triangle->a.y - (triangle->a.y - triangle->c.y) * 0.5f - icon_size * 0.5f;

    if (state->input_dir == DIR_DOWN && state->accept_input)
    {
        create_color_triangle(batch, *triangle, color_pressed);
        create_quad(batch, texture_rect, UI_DOWN_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color_pressed.a));
    }
    else
    {
        create_color_triangle(batch, *triangle, color);
        create_quad(batch, texture_rect, UI_DOWN_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color.a));
    }

    // Right
    triangle = &state->touch_input_triangles[TOUCH_INPUT_RIGHT];
    texture_rect.x = triangle->c.x + (triangle->a.x - triangle->c.x) * 0.5f;
    texture_rect.y = triangle->c.y - icon_size * 0.5f;

    if (state->input_dir == DIR_RIGHT && state->accept_input)
    {
        create_color_triangle(batch, *triangle, color_pressed);
        create_quad(batch, texture_rect, UI_RIGHT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color_pressed.a));
    }
    else
    {
        create_color_triangle(batch, *triangle, color);
        create_quad(batch, texture_rect, UI_RIGHT_ARROW, glm::vec4(1.0f, 1.0f, 1.0f, color.a));
    }
}

// COPYPASTA from: https://github.com/ocornut/imgui/issues/3379#issuecomment-1678718752
void ScrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;
    bool hovered = false;
    bool held = false;
    ImGuiID id = window->GetID("##scrolldraggingoverlay");
    ImGui::KeepAliveID(id);
    ImGuiButtonFlags button_flags = (mouse_button == 0) ? ImGuiButtonFlags_MouseButtonLeft : (mouse_button == 1) ? ImGuiButtonFlags_MouseButtonRight : ImGuiButtonFlags_MouseButtonMiddle;
    if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
        ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
    if (held && delta.x != 0.0f)
        ImGui::SetScrollX(window, window->Scroll.x + delta.x);
    if (held && delta.y != 0.0f)
        ImGui::SetScrollY(window, window->Scroll.y + delta.y);
}

// NOTE: Revisar esto: https://dustri.org/b/min-and-max-macro-considered-harmful.html
inline int rect_min (int a, int b)
{
    if (a < b) return a;
    else if (b < a) return b;
    else return a;
}

inline int rect_max (int a, int b)
{
    if (a > b) return a;
    else if (b > a) return b;
    else return a;
}

// Manejo de layout con CutRect

Rect cut_left (Rect *rect, int32_t a)
 {
     int32_t x = rect->x;
     rect->x = rect_min (rect->x + rect->width, rect->x + a);
     rect->width -= a;

     Rect result = {x, rect->y, a, rect->height};

     return result;
 }

Rect cut_right (Rect *rect, int32_t a)
{
     //rect->w = max (rect->x, rect->w - a);
     rect->width = rect->width - a;

     Rect result = {rect->x + rect->width, rect->y, a, rect->height};

     return result;
}

 Rect cut_top (Rect *rect, int32_t a)
 {
     int32_t y = rect->y;
     rect->y = rect_min(rect->y + rect->height, rect->y + a);
     rect->height -= a;

     Rect result = { rect->x, y, rect->width, rect->y - y };

     return result;
 }

Rect cut_bottom (Rect *rect, int32_t a)
{
     rect->height = rect_min (rect->height, rect->height - a);

     Rect result = { rect->x, rect->y + rect->height, rect->width, a};

     return result;
}

 //
 // Obtiene el rectangulo (no modifica el rectangulo de entrada)
 //

 Rect get_left (Rect *rect, int32_t a)
 {
     Rect result =  {rect->x, rect->y, a, rect->height};

     return result;
 }

Rect get_right (Rect *rect, int32_t a)
{
    Rect result = {rect->x + rect->width - a, rect->y, a, rect->height};
    return result;
}

Rect get_top (Rect *rect, int32_t a)
 {
    Rect result = { rect->x, rect->y, rect->width, a };
     return result;
 }

Rect get_bottom (Rect *rect, int32_t a)
{
    Rect result = { rect->x, rect->y + rect->height - a, rect->width, a};
    return result;
}


Rect shrink(Rect *rect, int32_t margin)
 {
     // TODO: Recorta un margen del rect en todos sus lados.

     return *rect;
 }
