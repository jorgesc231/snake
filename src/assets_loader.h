#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

#define ATLAS_IMAGE_PATH      "atlas.png"
#define ATLAS_WIDTH			  256
#define ATLAS_HEIGHT		  256
//#define ATLAS_SPRITE_COUNT    15

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
static AtlasSprite DescAtlas[ATLAS_SPRITE_COUNT] = {
	{ NO_TEXTURE, 120, 80, 39, 39, 0},      // 39 para evitar lineas entre los cuadrados.
	{ APPLE, 0, 0, 39, 39, 0},

	{ TAIL_RIGHT, 120, 0, 39, 39, 0},
    { TAIL_UP, 160, 0, 39, 39, 0},
	{ TAIL_DOWN, 40, 80, 39, 39, 0},
    { TAIL_LEFT, 80, 80, 39, 39, 0},

	{ BODY_BOTTOMLEFT, 40, 0, 39, 39, 0},
    { BODY_BOTTOMRIGHT, 80, 0, 39, 39, 0},
	{ BODY_HORIZONTAL, 200, 0, 39, 39, 0},
    { BODY_TOPLEFT, 0, 40, 39, 39, 0},
    { BODY_TOPRIGHT, 40, 40, 39, 39, 0},
    { BODY_VERTICAL, 80, 40, 39, 39, 0},

	{ HEAD_DOWN, 120, 40, 40, 40, 0},
    { HEAD_LEFT, 160, 40, 40, 40, 0},
    { HEAD_RIGHT, 200, 40, 40, 40, 0},
    { HEAD_UP, 0, 80, 40, 40, 0},
};


char* data_path(char *path_buffer, char* base_path, const char* asset_path);

GLuint shaderProgLoad(const char* vertFilename, const char* fragFilename);

GLuint texture_load(const char* filename);
void texture_destroy(GLuint texName);


#endif
