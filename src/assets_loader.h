#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengles2.h>

#include <glm/glm.hpp>

#define MAX_SND_CHANNELS 8

#define ATLAS_WIDTH     1024
#define ATLAS_HEIGHT    1024
#define SPRITE_SIZE     128
#define SPRITE_POS(pos) (pos * SPRITE_SIZE)

#define CONFIG_FORMAT_VERSION 1

struct Game_state;

struct AssetManager {
    int config_version = 0;
    const char *base_path = NULL;
    char cwd[PATH_MAX];
    const char *android_internal_storage_path;
};

typedef enum _SPRITE_ID {
    NO_TEXTURE,
    APPLE = 1,
    TAIL_RIGHT,
    TAIL_UP,
    TAIL_DOWN,
    TAIL_LEFT,

    BODY_BOTTOMLEFT,
    BODY_BOTTOMRIGHT,
    BODY_HORIZONTAL,
    BODY_TOPLEFT,
    BODY_TOPRIGHT,
    BODY_VERTICAL,

    HEAD_DOWN,
    HEAD_LEFT,
    HEAD_RIGHT,
    HEAD_UP,

    UI_NO_AUDIO,
    UI_AUDIO,
    UI_PAUSE,
    UI_LEFT_ARROW,
    UI_RIGHT_ARROW,
    UI_UP_ARROW,
    UI_DOWN_ARROW,

    UI_TROPHY,

    FRUIT,
    RARE_TAIL_RIGHT,
    RARE_TAIL_UP,
    RARE_TAIL_DOWN,
    RARE_TAIL_LEFT,

    RARE_BODY_BOTTOMLEFT,
    RARE_BODY_BOTTOMRIGHT,
    RARE_BODY_HORIZONTAL,
    RARE_BODY_TOPLEFT,
    RARE_BODY_TOPRIGHT,
    RARE_BODY_VERTICAL,

    RARE_HEAD_DOWN,
    RARE_HEAD_LEFT,
    RARE_HEAD_RIGHT,
    RARE_HEAD_UP,

    RARE_GRASS_CLEAR,
    RARE_GRASS_DARK,

    RETRO_BODY,
    RETRO_FOOD,
    RETRO_CELL,

    ATLAS_SPRITE_COUNT
} SPRITE_ID;

// Atlas sprite properties
typedef struct AtlasSprite {
    SPRITE_ID Id;
    int positionX, positionY;
    int sourceWidth, sourceHeight;
    int padding;
} AtlasSprite;

// Atlas sprites array
static AtlasSprite DescAtlas[] = {
    { NO_TEXTURE, SPRITE_POS(7), SPRITE_POS(7), SPRITE_SIZE, SPRITE_SIZE, 0},
    { APPLE, SPRITE_POS(0), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},

    { TAIL_RIGHT, SPRITE_POS(3), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { TAIL_UP, SPRITE_POS(4), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { TAIL_DOWN, SPRITE_POS(5), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},
    { TAIL_LEFT, SPRITE_POS(6), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},

    { BODY_BOTTOMLEFT, SPRITE_POS(1), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { BODY_BOTTOMRIGHT, SPRITE_POS(2), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { BODY_HORIZONTAL, SPRITE_POS(5), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { BODY_TOPLEFT, SPRITE_POS(6), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { BODY_TOPRIGHT, SPRITE_POS(7), SPRITE_POS(0), SPRITE_SIZE, SPRITE_SIZE, 0},
    { BODY_VERTICAL, SPRITE_POS(0), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},

    { HEAD_DOWN, SPRITE_POS(1), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},
    { HEAD_LEFT, SPRITE_POS(2), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},
    { HEAD_RIGHT, SPRITE_POS(3), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},
    { HEAD_UP, SPRITE_POS(4), SPRITE_POS(1), SPRITE_SIZE, SPRITE_SIZE, 0},

    { UI_NO_AUDIO, SPRITE_POS(2), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },
    { UI_AUDIO, SPRITE_POS(1), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },
    { UI_PAUSE, SPRITE_POS(0), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },
    { UI_LEFT_ARROW, SPRITE_POS(4), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },
    { UI_RIGHT_ARROW, SPRITE_POS(6), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },
    { UI_UP_ARROW, SPRITE_POS(5), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },
    { UI_DOWN_ARROW, SPRITE_POS(7), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },

    { UI_TROPHY, SPRITE_POS(3), SPRITE_POS(5), SPRITE_SIZE, SPRITE_SIZE, 0 },

    { FRUIT, SPRITE_POS(0), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},

    { RARE_TAIL_RIGHT, SPRITE_POS(4), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_TAIL_UP, SPRITE_POS(5), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_TAIL_DOWN, SPRITE_POS(6), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_TAIL_LEFT, SPRITE_POS(3), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},

    { RARE_BODY_BOTTOMLEFT, SPRITE_POS(3), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_BODY_BOTTOMRIGHT, SPRITE_POS(4), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_BODY_HORIZONTAL, SPRITE_POS(1), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_BODY_TOPLEFT, SPRITE_POS(6), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_BODY_TOPRIGHT, SPRITE_POS(5), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_BODY_VERTICAL, SPRITE_POS(2), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},

    { RARE_HEAD_DOWN, SPRITE_POS(2), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_HEAD_LEFT, SPRITE_POS(7), SPRITE_POS(2), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_HEAD_RIGHT, SPRITE_POS(0), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_HEAD_UP, SPRITE_POS(1), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},

    { RARE_GRASS_CLEAR, SPRITE_POS(7), SPRITE_POS(3), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RARE_GRASS_DARK, SPRITE_POS(0), SPRITE_POS(4), SPRITE_SIZE, SPRITE_SIZE, 0},

    { RETRO_BODY, SPRITE_POS(1), SPRITE_POS(4), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RETRO_FOOD, SPRITE_POS(3), SPRITE_POS(4), SPRITE_SIZE, SPRITE_SIZE, 0},
    { RETRO_CELL, SPRITE_POS(2), SPRITE_POS(4), SPRITE_SIZE, SPRITE_SIZE, 0},
};

int load_state_file(AssetManager *assets, const char *filename, Game_state *state);
bool write_state_file(AssetManager *assets, const char *filename, Game_state *state);

uint32_t init_asset_manager(AssetManager *manager);

char* get_data_path(AssetManager *manager, const char* asset_path);
char* get_base_path(AssetManager *manager, const char* file_name);

GLuint shader_prog_load(AssetManager *manager, const char* vertFilename, const char* fragFilename);

GLuint texture_load(AssetManager *assets, const char* filename);
void texture_destroy(GLuint texName);

void* load_font(AssetManager *assets, const char *filename, void *font_data, int32_t *font_data_size);

uint32_t init_audio(Mix_Chunk **sounds_buffer, int SND_MAX);

int32_t get_system_language(Game_state *state);

#endif
