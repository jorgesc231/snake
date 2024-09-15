#ifndef __SNAKE_H__
#define __SNAKE_H__

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <SDL.h>
#include <SDL_image.h>
//#include <SDL_ttf.h>
#include <SDL_mixer.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#else

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#endif

#if defined(__ANDROID__)
#include <SDL_opengles2.h>
#else
#include <SDL2/SDL_opengles2.h>
//#include <GLES2/gl2.h>          // Use GL ES 2
#endif

#include "renderer.h"
#include "assets_loader.h"
#include "translate.h"

#define CONFIG_FILE_NAME "config.txt"

#define MAX_SNAKE_LENGTH 1024
#define DEFAULT_COLUMNS_COUNT 17
#define DEFAULT_ROWS_COUNT 15
#define DEFAULT_CELL_SIZE 48
#define INITIAL_TAIL_COUNTER 3

#define DEFAULT_TIMESTEP 0.13f
#define DEFAULT_NORMAL_TIMESTEP 0.12f
#define DEFAULT_FAST_TIMESTEP 0.10f

#define DIRECTION_BUFFER_SIZE 2

enum {
    CATEGORY_GAME_SNAKE = SDL_LOG_CATEGORY_CUSTOM,
    CATEGORY_ENGINE_SNAKE,
};

struct v2 {
    int32_t x, y;
};

enum GAME_STATUS {
    INIT,
    PLAY,
    PAUSED,
    LOST,
};

enum DIFFICULTY {
    DIFFICULTY_SLOW,
    DIFFICULTY_NORMAL,
    DIFFICULTY_FAST,
    DIFFICULTY_PROGRESSIVE,

    DIFFICULTY_COUNT
};

enum GAME_LEVEL {
    LEVEL_DEFAULT,
    LEVEL_SMALL,
    LEVEL_BIG,
    LEVEL_PHONE,

    LEVEL_COUNT
};

enum GAME_SKIN {
    SKIN_DEFAULT,
    SKIN_RETRO,
    //SKIN_CLASSIC,
    SKIN_MINIMAL,
    SKIN_RARE,

    GAME_SKIN_COUNT
};

enum CONTROLLER_TYPE {
    CONTROLLER_KEYBOARD,
	CONTROLLER_TOUCH,
	CONTROLLER_TOUCH2,
	CONTROLLER_TOUCH3,

    CONTROLLER_TYPE_COUNT
};

enum
{
    CH_ANY = -1,
    CH_EAT,
    CH_GAME_OVER,
    CH_INTERACTION,
};

enum
{
    SND_SNAKE_EAT,
    SND_SNAKE_DIE,
    SND_INTERACTION,
    SND_MAX
};

enum
{
    TOUCH_INPUT_UP,
    TOUCH_INPUT_DOWN,
    TOUCH_INPUT_LEFT,
    TOUCH_INPUT_RIGHT,
};

typedef enum _DIRECTION_
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_LEFT,
} DIRECTION;

struct Snake {
    v2 position;
    v2 direction;
};

typedef struct
{
    int16_t rows, columns;
    int32_t hight_score_by_difficulty[DIFFICULTY_COUNT];

} SNAKE_LEVEL_INFO;

struct Game_state {

	int32_t game_over = false;
	int32_t quit = false;
	int32_t show_menu = false;

	int32_t audio_loaded = false;
	int32_t audio_enabled = true;
    
	int32_t show_status_screen = true;
	int32_t accept_input = true;

    LANGUAGES selected_language = ENGLISH;

    CONTROLLER_TYPE controller_type = CONTROLLER_KEYBOARD;

    DIRECTION direction_buffer[DIRECTION_BUFFER_SIZE] = {};

    Rect touch_input_rects[8] = {};
    Triangle touch_input_triangles[4] = {};
    DIRECTION input_dir = DIR_NONE;

    GAME_STATUS status = PAUSED;
    DIFFICULTY difficulty = DIFFICULTY_NORMAL;
    GAME_SKIN game_skin = SKIN_DEFAULT;
    GAME_LEVEL selected_level = LEVEL_DEFAULT;

    SNAKE_LEVEL_INFO levels[LEVEL_COUNT];

    float time = 0;
    float time_step = DEFAULT_NORMAL_TIMESTEP;
    float time_out = 0.6;
    int32_t time_out_completed = false;

    Snake snake[MAX_SNAKE_LENGTH] = {0};
    int32_t tail_counter = 0;

    int32_t cell_size = DEFAULT_CELL_SIZE;
    int32_t rows = DEFAULT_ROWS_COUNT, columns = DEFAULT_COLUMNS_COUNT;

    v2 food_pos = {10, 7};
    int32_t food_active = false;

    Rect screen_rect, score_rect;
    int32_t score_bar_size = PC_SCORE_BAR_SIZE;

    int32_t score = 0;
    
#if defined(NDEBUG)
    // debug gui state
    int32_t show_demo_window = false;
    int32_t show_another_window = false;
    int32_t show_debug_overlay = false;
#endif
};

int log_init(Game_state *state);
int32_t init_game(Renderer *renderer, Game_state *state);
void set_game_state(Game_state *state);
void main_loop();
void process_events(Renderer *renderer, Game_state *state);
void update_game(Game_state *state, float elapsed_time);
void update_snake(Game_state *state);
void render_game(Renderer *renderer, Game_state *state);

void shutdown_game(Renderer *renderer);

// Init the levels
void init_levels (Game_state *state);
void change_level(Game_state *state, GAME_LEVEL level);

bool is_over(Rect rect, int32_t x, int32_t y);

void add_input(Game_state* state, DIRECTION dir);
DIRECTION get_input(Game_state* state);

#endif
