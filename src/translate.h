#if !defined(_TRANSLATE_H_)
#define _TRANSLATE_H_

enum TRANSLATION_TEXT
{
	STR_GAME_TITLE,
	STR_BTN_PLAY,
	STR_BTN_OPTIONS,
	STR_PAUSE,
	STR_LOST,
	STR_CAT_SKIN,
	STR_CAT_LEVEL,
	STR_CAT_VELOCITY,
	STR_CAT_INPUT,
	STR_SKIN_DEFAULT,
	STR_SKIN_RETRO,
	STR_SKIN_CLASSIC,
	STR_SKIN_MINIMAL,
	STR_SKIN_RARE,
	STR_SKIN_3D,
	STR_LEVEL_DEFAULT,
    STR_LEVEL_PHONE,
	STR_LEVEL_SMALL,
	STR_LEVEL_BIG,
	STR_VEL_NORMAL,
	STR_VEL_SLOW,
	STR_VEL_FAST,
    STR_VEL_PROGRESSIVE,
	STR_INPUT_KEYBOARD,
	STR_INPUT_SWIPES,
	STR_INPUT_DUALPAD,
    STR_INPUT_HPAD,
    STR_INPUT_TRIANGLEPAD,
	STR_BTN_CLOSE,

	STR_SPANISH,
	STR_ENGLISH,

	STRINGS_COUNT
};

enum LANGUAGES
{
	SPANISH,
	ENGLISH,
};

struct Game_state;

const char* get_text(LANGUAGES lang, TRANSLATION_TEXT text);

#if defined(_IMPLEMENT_TRANSLATE_)

const char *es_strings[STRINGS_COUNT] = { "Snake", "Jugar", "Opciones", "PAUSA", "PERDISTE", "TEMA", "NIVEL", "VELOCIDAD", "ENTRADA", "DEFAULT", "RETRO", "CLASSIC", "MINIMAL", "MONSTRUO", "3D", "DEFAULT", "Largo", "PEQUEÑO", "GRANDE", "NORMAL", "LENTO", "RAPIDO", "GRADUAL", "TECLADO", "SWIPES", "PAD 1", "H-PAD", "PAD 2", "Volver", "Español", "Ingles" };
const char *en_strings[STRINGS_COUNT] = { "Snake", "Play", "Options", "PAUSE", "LOST", "SKIN", "LEVEL", "VELOCITY", "INPUT", "DEFAULT", "RETRO", "CLASSIC", "MINIMAL", "MONSTER", "3D", "DEFAULT", "Long", "SMALL", "BIG", "NORMAL", "SLOW", "FAST", "GRADUAL", "KEYBOARD", "SWIPES", "PAD 1", "H-PAD", "PAD 2", "Back", "Spanish", "English" };

const char* get_text(LANGUAGES lang, TRANSLATION_TEXT text)
{
	switch(lang)
	{
		case SPANISH:
		{
			return es_strings[text];
		} break;

		case ENGLISH:
		{
			return en_strings[text];
		} break;

		default:
		{
			return en_strings[text];
		} break;
	}
}
#endif


#endif
