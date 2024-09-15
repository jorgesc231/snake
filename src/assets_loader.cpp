// Carga de shaders, texturas, sonidos y fuentes de texto.
#include "assets_loader.h"
#include "snake.h"

#include <stdio.h>

#if defined(__ANDROID__)
#define ASSET_FOLDER "%s%s"
#elif defined(NDEBUG)
// TODO: Deberia cambiar dependiendo de la plataforma...
#define ASSET_FOLDER "%s../assets/%s"
#else
// Release
#define ASSET_FOLDER "%sassets/%s"
#endif

// NOTE: el working dir en raspberry no es el que deberia
// Construye la ruta hacia los assets basado en la ubicacion del ejecutable:
char* get_data_path(AssetManager *manager, const char* asset_path)
{
    static char path_buffer[PATH_MAX];
    
    snprintf(path_buffer, PATH_MAX, ASSET_FOLDER, manager->base_path, asset_path);
    return path_buffer;
}

char* get_base_path(AssetManager *manager, const char* file_name)
{
    static char path_buffer[PATH_MAX];

    snprintf(path_buffer, PATH_MAX, "%s%s", manager->base_path, file_name);
    return path_buffer;
}

uint32_t init_asset_manager(AssetManager *manager)
{
    if (getcwd(manager->cwd, sizeof(manager->cwd)) != NULL)
    {
        SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "Current working dir: %s\n", manager->cwd);
    }
    else
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "getcwd() error");
        return 1;
    }

    // Contruir el path absoluto hacia los assets
    if ((manager->base_path = SDL_GetBasePath()))
    {
        SDL_LogInfo(CATEGORY_ENGINE_SNAKE, "Base Path: %s\n", manager->base_path);
    }
    else
    {
        // Si no se puede obtener el base path, usamos el CWD como tal
        #if defined(__ANDROID__)
        // Caso especial para android, no implementa la funcion SDL_GetBasePath()
        manager->base_path = "";
        manager->android_internal_storage_path = SDL_AndroidGetInternalStoragePath();
        #else
        manager->base_path = manager->cwd;
        #endif

        SDL_LogWarn(CATEGORY_ENGINE_SNAKE, "Can't get the Base Path, Using CWD...: %s\n", manager->base_path);
    }
    
    return 1;
}

static size_t get_file_length(SDL_RWops *file)
{
    size_t length;
    
    size_t current_pos = SDL_RWtell(file);
    
    SDL_RWseek(file, 0, SEEK_END);
    
    length = SDL_RWtell(file);
    
    // Return the file to its previous position
    SDL_RWseek(file, current_pos, SEEK_SET);
    
    return length;
}


static GLuint shader_load(const char *filename, GLenum shader_type)
{
    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Loading Shader: %s\n", filename);

    SDL_RWops *file = SDL_RWFromFile(filename, "r");
    
    if (!file)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Can't open file: %s\n", filename);
        
        return 0;
    }
    
    size_t length = get_file_length(file);
    
    // Alloc space for the file (plus '\0' termination)
    GLchar* shader_src = (GLchar*)calloc(length + 1, 1);
    
    if (!shader_src)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Out of memory when reading file: %s\n", filename);
        SDL_RWclose(file);
        file = NULL;
        
        return 0;
    }
    
    SDL_RWread(file, shader_src, 1, length);
    
    // Done with the file
    SDL_RWclose(file);
    file = NULL;
    
    
    // Create the shader
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const GLchar**)&shader_src, NULL);
    free(shader_src);
    shader_src = NULL;
    
    // Compile
    glCompileShader(shader);
    GLint compile_succeeded = GL_FALSE;
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_succeeded);
    
    if (!compile_succeeded)
    {
        
        // Compilation failed. Print error info
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Compilation of shader: %s failed\n", filename);
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
        GLchar* errLog = (GLchar*)malloc(logLength);
        
        if (errLog)
        {
            glGetShaderInfoLog(shader, logLength, &logLength, errLog);
            SDL_LogError(CATEGORY_ENGINE_SNAKE, "%s\n", errLog);
            free(errLog);
        }
        else
        {
            SDL_LogError(CATEGORY_ENGINE_SNAKE, "Couldn't get shader log; out of memory\n");
        }
        
        glDeleteShader(shader);
        shader = 0;
    }
    
    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Shader Loaded: %s \n", filename);

    return shader;
}

static void shader_destroy(GLuint shaderID)
{
    glDeleteShader(shaderID);
}


GLuint shader_prog_load(AssetManager *asset_manager, const char* vert_filename, const char* frag_filename)
{
    GLuint vert_shader = shader_load(get_data_path(asset_manager, vert_filename), GL_VERTEX_SHADER);
    
    if (!vert_shader)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Couldn't load vertex shader: %s\n", vert_filename);
        return 0;
    }
    
    GLuint frag_shader = shader_load(get_data_path(asset_manager, frag_filename), GL_FRAGMENT_SHADER);
    
    if (!frag_shader)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Couldn't load fragment shader: %s\n", frag_filename);
        shader_destroy(vert_shader);
        vert_shader = 0;
        
        return 0;
    }
    
    GLuint shader_prog = glCreateProgram();
    
    if (shader_prog)
    {
        glAttachShader(shader_prog, vert_shader);
        glAttachShader(shader_prog, frag_shader);
        
        glLinkProgram(shader_prog);
        
        GLint linking_succeeded = GL_FALSE;
        glGetProgramiv(shader_prog, GL_LINK_STATUS, &linking_succeeded);
        
        if (!linking_succeeded)
        {
            SDL_LogError(CATEGORY_ENGINE_SNAKE, "Linking shader failed (vert. shader: %s, frag. shader: %s\n)", vert_filename, frag_filename);
            
            GLint log_length = 0;
            glGetProgramiv(shader_prog, GL_INFO_LOG_LENGTH, &log_length);
            
            GLchar* err_log = (GLchar*) malloc(log_length);
            
            if (err_log)
            {
                glGetProgramInfoLog(shader_prog, log_length, &log_length, err_log);
                SDL_LogError(CATEGORY_ENGINE_SNAKE, "%s\n", err_log);
                free(err_log);
            }
            else
            {
                SDL_LogError(CATEGORY_ENGINE_SNAKE, "Couldn't get shader link log; out of memory\n");
            }
            
            glDeleteProgram(shader_prog);
            shader_prog = 0;
        }
    }
    else
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Couldn't create shader program\n");
    }
    
    // Don't need these any more
    shader_destroy(vert_shader);
    shader_destroy(frag_shader);
    
    return shader_prog;
}

void shaderProgDestroy(GLuint shaderProg)
{
    glDeleteProgram(shaderProg);
}



// Texture loading
GLuint texture_load(AssetManager *assets, const char* filename)
{  
    char *absolute_path = get_data_path(assets, filename);

    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Loading Texture: %s\n", absolute_path);

    // TODO: Usar assets en svg para evitar problemas de escalado
    // Load the image
#if __USE_SVG__
    SDL_RWops *file = SDL_RWFromFile(absolute_path, "r");
    SDL_Surface* texSurf = IMG_LoadSizedSVG_RW(file, 1024, 1024);
#else
    SDL_Surface* texSurf = IMG_Load(absolute_path);
#endif

    if (!texSurf)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Loading image %s failed with error: %s", absolute_path, IMG_GetError());
        return 0;
    }
    
    // Determine the format
    // NOTE: Only supporting 24 and 32-bit images
    
    GLenum format;
    GLenum type = GL_UNSIGNED_BYTE;

    switch (texSurf->format->BytesPerPixel)
    {
        case 3:
            format = GL_RGB;
        break;
        case 4:
            format = GL_RGBA;
        break;
        default:
            SDL_LogError(CATEGORY_ENGINE_SNAKE, "Can't load image %s; it isn't a 24/32-bit image\n", absolute_path);
            SDL_FreeSurface(texSurf);
            texSurf = NULL;
            return 0;
    }
    
    // Create the texture
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set up the filtering
    // NOTE: Failure to do this may result in no texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);


    glTexImage2D(GL_TEXTURE_2D, 0, format, texSurf->w, texSurf->h, 0, format, type, texSurf->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        // Failed
        glDeleteBuffers(1, &texture);
        texture = 0;
        SDL_FreeSurface(texSurf);
        texSurf = NULL;
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Creating texture %s failed, code %u\n", filename, err);
        //      __debugbreak();
        return 0;
    }
    
    // Cleanup
    SDL_FreeSurface(texSurf);
    texSurf = NULL;
    
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Texture Loaded: %s\n", absolute_path);

    return texture;
}

void texture_destroy(GLuint tex_id)
{
    glDeleteTextures(1, &tex_id);
}

void init_levels (Game_state *state)
{
    for (int level = LEVEL_DEFAULT; level < LEVEL_COUNT; level++)
    {
        switch (level)
        {
            case LEVEL_SMALL:
            {
                state->levels[LEVEL_SMALL].rows = 10;
                state->levels[LEVEL_SMALL].columns = 12;
            } break;
            case LEVEL_BIG:
            {
                state->levels[LEVEL_BIG].rows = 24;
                state->levels[LEVEL_BIG].columns = 28;
            } break;

            case LEVEL_PHONE:
            {
                state->levels[LEVEL_PHONE].rows = 28;
                state->levels[LEVEL_PHONE].columns = 14;
            } break;

            default:
            case LEVEL_DEFAULT:
            {
                state->levels[LEVEL_DEFAULT].rows = DEFAULT_ROWS_COUNT;
                state->levels[LEVEL_DEFAULT].columns = DEFAULT_COLUMNS_COUNT;
            } break;
        }

        for (int i = 0; i < DIFFICULTY_COUNT; i++)  state->levels[level].hight_score_by_difficulty[i] = 0;
    }
}

void change_level (Game_state *state, GAME_LEVEL level)
{
    state->selected_level = level;
    state->rows = state->levels[level].rows;
    state->columns = state->levels[level].columns;

    set_game_state(state);
}

int32_t get_system_language(Game_state *state)
{
    SDL_Locale *locales = SDL_GetPreferredLocales();
    if (locales != nullptr)
    {
        for (SDL_Locale *locale = locales; locale->language != nullptr; locale++)
        {
            SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Lang: %s - Country: %s", locale->language, locale->country);

            if (locale->language[0] == 'e')
            {
                // Supported Languages (Other languages default to English)
                if (locale->language[1] == 'n') { state->selected_language = ENGLISH; break; }
                if (locale->language[1] == 's') { state->selected_language = SPANISH; break; }
            }
        }

        SDL_free(locales);
        return 1;
    }
    else return 0;
}


// Load Audio


// Load Font
void* load_font(AssetManager *assets, const char *filename, void *font_data, int32_t *font_data_size)
{
    char *absolute_path = get_data_path(assets, filename);

    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Loading Font: %s\n", absolute_path);

    SDL_RWops *file = SDL_RWFromFile(absolute_path, "r");
    
    if (!file)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Can't open file: %s\n", absolute_path);
        
        return 0;
    }
    
    size_t length = get_file_length(file);
    
    // Alloc space for the file (plus '\0' termination)
    
    void* file_content = (void*)calloc(length + 1, 1);
    
    if (!file_content)
    {
        
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Out of memory when reading file: %s\n", absolute_path);
        SDL_RWclose(file);
        file = NULL;
        
        return 0;
    }
    
    SDL_RWread(file, file_content, 1, length);
    
    // Done with the file
    SDL_RWclose(file);
    file = NULL;
    
    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "Font Loaded: %s \n", absolute_path);

    font_data = file_content;
    *font_data_size = length;

    return file_content;
}

uint32_t init_audio(Mix_Chunk **sounds_buffer, int SND_MAX)
{
    // Inicializa SDL_Mixer
    int32_t flags = MIX_INIT_MP3;
    int32_t initialized_flags = Mix_Init(flags);
    if ((initialized_flags & flags) != flags)
    {
        SDL_LogError(CATEGORY_ENGINE_SNAKE, "Couldn't initialize SDL Mixer: %s\n", Mix_GetError());
        return 0;
    }
    else
    {
        if (SDL_GetNumAudioDevices(0) == 0 || Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)
        {
            SDL_LogError(1, "Couldn't initialize Audio Device\n");
            return 0;
        }

        Mix_AllocateChannels(MAX_SND_CHANNELS);

        memset(sounds_buffer, 0, sizeof(Mix_Chunk*) * SND_MAX);
    }
    
    return 1;
}

// Load Saved State File
int load_state_file(AssetManager *assets, const char *filename, Game_state *state)
{
    char *absolute_path = get_base_path(assets, filename);

    SDL_LogDebug(CATEGORY_GAME_SNAKE, "Loading File: %s\n", absolute_path);

    // TODO: Temporal
    static char path_buffer[PATH_MAX];
    snprintf(path_buffer, PATH_MAX, "%s/%s", assets->android_internal_storage_path, filename);

#if defined(__ANDROID__)
    SDL_RWops *file = SDL_RWFromFile(path_buffer, "r");
#else
    SDL_RWops *file = SDL_RWFromFile(absolute_path, "r");
#endif

    if (!file)
    {
        SDL_LogWarn(CATEGORY_GAME_SNAKE, "Can't open Config file: %s\n", absolute_path);

        return 0;
    }

    //size_t length = get_file_length(file);

    char buffer[100] = {};
    char caracter = SDL_ReadU8(file);

    for (int i = 0; caracter != '\0' && i < 100; i++)
    {
    	buffer[i] = caracter;
    	caracter = SDL_ReadU8(file);
    }

    sscanf((const char *)&buffer,
    		"ver %d\naudio %d\nskin %d\ndif %d\ncon %d\nlevel %d\n%d %d %d %d\n%d %d %d %d\n%d %d %d %d\n%d %d %d %d\n",
            &assets->config_version,
    		(int32_t*)&state->audio_enabled, (int32_t*)&state->game_skin, (int32_t*)&state->difficulty,
			(int32_t*)&state->controller_type, (int32_t*)&state->selected_level,

            &state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_SLOW],
            &state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_NORMAL],
            &state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_FAST],
           &state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE],

            &state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_SLOW],
            &state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_NORMAL],
            &state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_FAST],
           &state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE],

            &state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_SLOW],
            &state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_NORMAL],
            &state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_FAST],
           &state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE],

            &state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_SLOW],
            &state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_NORMAL],
            &state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_FAST],
            &state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE]
           );

    // Done with the file
    SDL_RWclose(file);
    file = NULL;

    SDL_LogDebug(CATEGORY_ENGINE_SNAKE, "File Loaded: %s \n", absolute_path);

    return 1;
}

// Load Saved State File
bool write_state_file(AssetManager *assets, const char *filename, Game_state *state)
{
    char *absolute_path = get_base_path(assets, filename);

    SDL_LogDebug(CATEGORY_GAME_SNAKE, "Loading Config File: %s\n", absolute_path);

    // TODO: Temporal
    static char path_buffer[PATH_MAX];
    snprintf(path_buffer, PATH_MAX, "%s/%s", assets->android_internal_storage_path, filename);

#if defined(__ANDROID__)
    SDL_RWops *file = SDL_RWFromFile(path_buffer, "w");
#else
    SDL_RWops *file = SDL_RWFromFile(absolute_path, "w");
#endif

    if (!file)
    {
        SDL_LogError(CATEGORY_GAME_SNAKE, "Can't open Config File: %s\n", absolute_path);
        return 0;
    }

    // Write configuration
    char buffer[100] = {};
    snprintf(buffer, 100, "ver %d\naudio %d\nskin %d\ndif %d\ncon %d\nlevel %d\n%d %d %d %d\n%d %d %d %d\n%d %d %d %d\n%d %d %d %d\n",
             CONFIG_FORMAT_VERSION,
             state->audio_enabled, state->game_skin, state->difficulty, state->controller_type, state->selected_level,
             state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_SLOW],
             state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_NORMAL],
             state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_FAST],
             state->levels[LEVEL_DEFAULT].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE],

             state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_SLOW],
             state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_NORMAL],
             state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_FAST],
             state->levels[LEVEL_SMALL].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE],

             state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_SLOW],
             state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_NORMAL],
             state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_FAST],
             state->levels[LEVEL_BIG].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE],

             state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_SLOW],
             state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_NORMAL],
             state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_FAST],
             state->levels[LEVEL_PHONE].hight_score_by_difficulty[DIFFICULTY_PROGRESSIVE]
            );

    for (int i = 0; buffer[i] != '\0' && i < 100; i++)
    {
    	SDL_WriteU8(file, buffer[i]);
    }

    // Done with the file
    SDL_RWclose(file);
    file = NULL;

    SDL_LogDebug(CATEGORY_GAME_SNAKE, "Config File Saved: %s \n", absolute_path);

    return 1;
}
