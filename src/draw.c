#include "global.h"

#define TEXTURE_CARD_WIDTH  69
#define TEXTURE_CARD_HEIGHT 93

#define BLIND_CHIP_WIDTH  32
#define BLIND_CHIP_HEIGHT 32

int tex_enhancers, tex_deck, tex_deck2, tex_gamepad_ui, tex_editions, tex_shop, tex_ui_assets, tex_tags;
int tex_title_logo = -1;
int tex_round_corners = -1;
int tex_blind_chips[2][3];
int tex_jokers[2][4];
int tex_tarots[2][2];
int tex_boosters[2];

int font_big, font_small;

bool g_panel_shadow = false;

int game_draw_get_blind_reward(int blind);

// Dessatura uma cor ABGR em direcao ao cinza (0 = original, 1 = cinza)
static uint32_t desaturate_color(uint32_t c, float amt)
{
    float r = (float)(c & 0xFF);
    float g = (float)((c >> 8) & 0xFF);
    float b = (float)((c >> 16) & 0xFF);
    float gray = (r + g + b) / 3.0f;
    r += (gray - r) * amt;
    g += (gray - g) * amt;
    b += (gray - b) * amt;
    return 0xFF000000u | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
}

void game_draw_rounded_box(float x, float y, float w, float h, float r, uint32_t fill, uint32_t border, float t);
uint32_t game_draw_stage_border_color(int draw_stage);

#define GAMEPAD_UI_CROSS            0
#define GAMEPAD_UI_CIRCLE           1
#define GAMEPAD_UI_SQUARE           2
#define GAMEPAD_UI_TRIANGLE         3
#define GAMEPAD_UI_LEFT_TRIGGER     4
#define GAMEPAD_UI_RIGHT_TRIGGER    5

#define GAMEPAD_UI_COUNT            6

#define UI_ASSETS_CHIP              0
#define UI_ASSETS_ACE               1
#define UI_ASSETS_FACE              2
#define UI_ASSETS_NUMBER            3
#define UI_ASSETS_HEARTS            4
#define UI_ASSETS_DIAMONDS          5
#define UI_ASSETS_CLUBS             6
#define UI_ASSETS_SPADES            7

#define UI_ASSETS_COUNT             8

struct TexCoords
{
    int x, y;
}
g_enhancement_tex_coords[CARD_ENHANCEMENT_COUNT] = {
    { 71, 1 },  // CARD_ENHANCEMENT_NONE
    { 71, 95 },   // CARD_ENHANCEMENT_BONUS
    { 142, 95 },   // CARD_ENHANCEMENT_MULT
    { 213, 95 },   // CARD_ENHANCEMENT_WILD
    { 355, 95 },   // CARD_ENHANCEMENT_GLASS
    { 426, 95 },   // CARD_ENHANCEMENT_STEEL
    { 355, 1 },   // CARD_ENHANCEMENT_STONE
    { 426, 1 },   // CARD_ENHANCEMENT_GOLD
    { 284, 95 }    // CARD_ENHANCEMENT_LUCKY
},
g_gamepad_ui_text_coords[GAMEPAD_UI_COUNT] = {
    { 0, 32 }, // GAMEPAD_UI_CROSS
    { 32, 32 }, // GAMEPAD_UI_CIRCLE
    { 64, 32 }, // GAMEPAD_UI_SQUARE
    { 96, 32 }, // GAMEPAD_UI_TRIANGLE
    { 128, 64 }, // GAMEPAD_UI_LEFT_TRIGGER
    { 160, 64 }  // GAMEPAD_UI_RIGHT_TRIGGER
},
g_editions_tex_coords[CARD_EDITION_COUNT] = {
    { 1, 1 },   // CARD_EDITION_BASE
    { 143, 1 }, // CARD_EDITION_FOIL
    { 72, 1 }, // CARD_EDITION_HOLOGRAPHIC
    { 1, 1 }, // CARD_EDITION_POLYCHROME
    { 1, 1 },   // CARD_EDITION_NEGATIVE
},
g_seal_tex_coords[] = {
    { 0, 0 },     // CARD_SEAL_NONE
    { 143, 1 },   // CARD_SEAL_GOLD
    { 356, 381 }, // CARD_SEAL_RED
    { 427, 381 }, // CARD_SEAL_BLUE
    { 285, 381 }  // CARD_SEAL_PURPLE
},
g_ui_assets_tex_coords[UI_ASSETS_COUNT] = {
    { 0, 0 }, // UI_ASSETS_CHIP
    { 18, 0 }, // UI_ASSETS_ACE
    { 36, 0 }, // UI_ASSETS_FACE
    { 54, 0 }, // UI_ASSETS_NUMBER
    { 0, 18 }, // UI_ASSETS_HEARTS
    { 18, 18 }, // UI_ASSETS_DIAMONDS
    { 36, 18 }, // UI_ASSETS_CLUBS
    { 54, 18 }  // UI_ASSETS_SPADES
};

int g_boss_blind_tex_rows[BOSS_BLIND_COUNT] = {
    7,  // BOSS_BLIND_HOOK -> 7
    2,  // BOSS_BLIND_OX -> 2
    3,  // BOSS_BLIND_HOUSE -> 3
    9,  // BOSS_BLIND_WALL -> 9
    10, // BOSS_BLIND_WHEEL -> 10
    11, // BOSS_BLIND_ARM -> 11
    4,  // BOSS_BLIND_CLUB -> 4
    5,  // BOSS_BLIND_FISH -> 5
    12, // BOSS_BLIND_PSYCHIC -> 12
    13, // BOSS_BLIND_GOAD -> 13
    14, // BOSS_BLIND_WATER -> 14
    6,  // BOSS_BLIND_WINDOW -> 6
    8,  // BOSS_BLIND_MANACLE -> 8
    17, // BOSS_BLIND_EYE -> 17
    18, // BOSS_BLIND_MOUTH -> 18
    19, // BOSS_BLIND_PLANT -> 19
    15, // BOSS_BLIND_SERPENT -> 15
    16, // BOSS_BLIND_PILLAR -> 16
    20, // BOSS_BLIND_NEEDLE -> 20
    21, // BOSS_BLIND_HEAD -> 21
    22, // BOSS_BLIND_TOOTH -> 22
    24, // BOSS_BLIND_FLINT -> 24
    23, // BOSS_BLIND_MARK -> 23
    27, // BOSS_BLIND_AMBER_ACORN -> 27
    28, // BOSS_BLIND_VERDANT_LEAF -> 28
    29, // BOSS_BLIND_VIOLET_VESSEL -> 29
    25, // BOSS_BLIND_CRIMSON_HEART -> 25
    26  // BOSS_BLIND_CERULEAN_BELL -> 26
};

int32_t g_time = 0;
uint64_t g_title_anim_start = 0;

void game_init_draw_load_fonts()
{
    // font_big = graphics_load_font("font_big.png", 8, 8, 16, 16);
    // font_small = graphics_load_font("font_small.png", 6, 8, 16, 16);
    font_big = graphics_load_big_font();
    font_small = graphics_load_small_font();
}

bool game_init_draw()
{
    char str[64];

    if ((tex_enhancers = graphics_load_texture_from_archive_16bit("resources/textures/1x/Enhancers.png", 0, 0)) < 0) return false;

    struct Image deck_image = graphics_load_image_from_archive("resources/textures/1x/8BitDeck.png");
    if (deck_image.data == NULL) return false;
    game_draw_loading_text("Creating deck texture 0", COLOR_WHITE, COLOR_BLACK);
    tex_deck = graphics_load_texture_from_image_16bit(&deck_image, 0, 0);
    game_draw_loading_text("Creating deck texture 1", COLOR_WHITE, COLOR_BLACK);
    tex_deck2 = graphics_load_texture_from_image_16bit(&deck_image, (TEXTURE_CARD_WIDTH + 2) * 7, 0);
    graphics_destroy_image(&deck_image);

    struct Image joker_image = graphics_load_image_from_archive("resources/textures/1x/Jokers.png");
    if (joker_image.data == NULL) return false;
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            DEBUG_PRINTF("Loading Jokers (%d; %d)\n", i, j);
            sprintf(str, "Creating joker texture %d", i*4+j);
            game_draw_loading_text(str, COLOR_WHITE, COLOR_BLACK);
            tex_jokers[i][j] = graphics_load_texture_from_image_16bit(&joker_image, i * (TEXTURE_CARD_WIDTH + 2) * 7, j * (TEXTURE_CARD_HEIGHT + 2) * 5);
        }
    }
    graphics_destroy_image(&joker_image);

    tex_gamepad_ui = graphics_load_texture_from_archive_16bit("resources/textures/1x/gamepad_ui.png", 0, 0);
    if (tex_gamepad_ui < 0) return false;

    game_draw_loading_text("Loading Tarots.png", COLOR_WHITE, COLOR_BLACK);
    struct Image tarots_image = graphics_load_image_from_archive("resources/textures/1x/Tarots.png");
    if (tarots_image.data == NULL) return false;
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            DEBUG_PRINTF("Loading Tarots (%d; %d)\n", i, j);
            sprintf(str, "Creating tarots texture %d", i*2+j);
            game_draw_loading_text(str, COLOR_WHITE, COLOR_BLACK);
            tex_tarots[i][j] = graphics_load_texture_from_image_16bit(&tarots_image, i * (TEXTURE_CARD_WIDTH + 2) * 7, j * (TEXTURE_CARD_HEIGHT + 2) * 5);
        }
    }
    graphics_destroy_image(&tarots_image);

    tex_editions = graphics_load_texture_16bit("assets/editions.png", 0, 0);
    if (tex_editions < 0) return false;

    struct Image boosters_image = graphics_load_image_from_archive("resources/textures/1x/boosters.png");
    if (boosters_image.data == NULL) return false;
    for(int j = 0; j < 2; j++)
    {
        DEBUG_PRINTF("Loading boosters (%d)\n", j);
        sprintf(str, "Creating boosters texture %d", j);
        game_draw_loading_text(str, COLOR_WHITE, COLOR_BLACK);
        tex_boosters[j] = graphics_load_texture_from_image_16bit(&boosters_image, 0, j * (TEXTURE_CARD_HEIGHT + 2) * 5);
    }
    graphics_destroy_image(&boosters_image);

    tex_shop = graphics_load_texture_from_archive_16bit("resources/textures/1x/ShopSignAnimation.png", 0, 0);
    if (tex_shop < 0) return false;

    tex_ui_assets = graphics_load_texture_from_archive_16bit("resources/textures/1x/ui_assets.png", 0, 0);
    if (tex_ui_assets < 0) return false;

    tex_tags = graphics_load_texture_from_archive_16bit("resources/textures/1x/tags.png", 0, 0);
    if (tex_tags < 0) return false;

    game_draw_loading_text("Loading title logo", COLOR_WHITE, COLOR_BLACK);
    tex_title_logo = graphics_load_texture_from_archive_16bit("resources/textures/1x/balatro.png", 0, 0);
    if (tex_title_logo < 0)
    {
        game_draw_loading_text("Title logo not found (using text)", COLOR_TEXT_RED, COLOR_BLACK);
    }

    // Tile 16x16 de cantos arredondados gerado em codigo (4 quadrantes 8x8 com alpha).
    game_draw_loading_text("Loading round corners", COLOR_WHITE, COLOR_BLACK);
    {
        struct Image corners;
        corners.w = 16;
        corners.h = 16;
        corners.channels = 4;
        corners.data = (uint8_t *)malloc(16 * 16 * 4);
        if (corners.data != NULL)
        {
            uint32_t *px = (uint32_t *)corners.data;
            for (int qy = 0; qy < 2; qy++)
            {
                for (int qx = 0; qx < 2; qx++)
                {
                    for (int j = 0; j < 8; j++)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            float lx = (qx == 0) ? (7.5f - (float)i) : ((float)i - 0.5f);
                            float ly = (qy == 0) ? (7.5f - (float)j) : ((float)j - 0.5f);
                            float d = sqrtf(lx * lx + ly * ly);
                            px[(qy * 8 + j) * 16 + qx * 8 + i] = (d <= 8.0f) ? 0xFFFFFFFFu : 0x00000000u;
                        }
                    }
                }
            }
            tex_round_corners = graphics_load_texture_from_image_16bit(&corners, 0, 0);
            free(corners.data);
        }
    }
    if (tex_round_corners < 0) return false;

    struct Image chips_image = graphics_load_image_from_archive("resources/textures/1x/BlindChips.png");
    if (chips_image.data == NULL) return false;
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 3; j++)
        {
	    DEBUG_PRINTF("Loading blind chips (%d)\n", i, j);
            sprintf(str, "Creating blind chips texture %d", i*3+j);
            game_draw_loading_text(str, COLOR_WHITE, COLOR_BLACK);
            tex_blind_chips[i][j] = graphics_load_texture_from_image_16bit(&chips_image, i * (BLIND_CHIP_WIDTH + 2) * 15, j * (BLIND_CHIP_HEIGHT + 2) * 15);
        }
    }
    graphics_destroy_image(&chips_image);

    sceKernelDcacheWritebackInvalidateAll();

    bg_shader_init();
    bg_splash_init();

    return true;
}

float g_loading_text_y = 0.0f;

void game_draw_clear_loading(uint32_t color)
{
    graphics_clear_loading_text(color);

    graphics_show_loading_text(font_small, "Port by S\x82rgio Vieira & luxotick", 4.0f, SCREEN_HEIGHT - 16.0f, 1.0f, COLOR_LIGHT_GREY, false, COLOR_BLACK);
    graphics_show_loading_text(font_small, "github.com/internalregister & github.com/luxotick", 4.0f, SCREEN_HEIGHT - 8.0f, 1.0f, COLOR_LIGHT_GREY, false, COLOR_BLACK);
}

void game_draw_loading_text(const char *text, uint32_t color, uint32_t back_color)
{
    // Otimização: cada chamada custa 2x sceGuSwapBuffers + WaitVblank (~33ms).
    // Durante o load (~30 arquivos) isso somava ~1s só em vsync. Descarta
    // mensagens intermediárias; erros (vermelho) sempre desenham.
    static int s_loading_text_counter = 0;
    s_loading_text_counter++;
    if (color != COLOR_TEXT_RED && (s_loading_text_counter % 2) == 0) return;

    if (g_loading_text_y == 0.0f) game_draw_clear_loading(back_color);
    graphics_show_loading_text(font_small, text, 0, g_loading_text_y, 1.0f, color, false, COLOR_BLACK);
    g_loading_text_y += 8.0f;
    if (g_loading_text_y >= SCREEN_HEIGHT) g_loading_text_y = 0.0f;
}

void game_draw_card_hint(struct Card *card)
{
    int line_count = 1;
    if (card->extra_chips > 0) line_count++;
    if (card->enhancement != CARD_ENHANCEMENT_NONE && card->enhancement != CARD_ENHANCEMENT_STONE) line_count++;
    if (card->edition != CARD_EDITION_BASE) line_count++;
    if (card->seal != CARD_SEAL_NONE) line_count++;

    // Clamp estilo cardtest (x em [2, 478-w]).
    float left = card->draw.x - CARD_HINT_WIDTH / 2.0f + CARD_WIDTH / 2.0f;
    if (left < 2.0f)
    {
        left = 2.0f;
    }
    if (left > SCREEN_WIDTH - CARD_HINT_WIDTH)
    {
        left = SCREEN_WIDTH - CARD_HINT_WIDTH;
    }
    float height = CARD_HINT_MIN_HEIGHT + 8.0f * line_count;
    float top = card->draw.y - height - 4.0f;
    if (top < 0.0f)
    {
        top = card->draw.y + CARD_HEIGHT + 4.0f;
    }
    // Caixa arredondada estilo cardtest (draw_round_box).
    graphics_set_texture(-1, -1);
    game_draw_rounded_box(left, top, CARD_HINT_WIDTH, height, 5, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);
    game_draw_rounded_box(left + 2, top + CARD_HINT_MIN_HEIGHT - 12.0f, CARD_HINT_WIDTH - 4, 8.0f * line_count + 8.0f, 3, COLOR_WHITE, COLOR_WHITE, 1);

    char str[64];
    game_set_card_name(card, str);
    graphics_draw_text_center(font_small, str, left + CARD_HINT_WIDTH / 2.0f, top + 10.0f, 1.0f, COLOR_WHITE);

    sprintf(str, "#3+%d#- chips", game_util_get_card_original_chips(card));
    float text_y = top + 14.0f + 12.0f;
    graphics_draw_text_formatted_center(font_small, str, NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
    text_y += 8.0f;

    if (card->extra_chips > 0)
    {
        sprintf(str, "#3+%d#- extra chips", card->extra_chips);
        graphics_draw_text_formatted_center(font_small, str, NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
        text_y += 8.0f;
    }

    if (card->enhancement != CARD_ENHANCEMENT_NONE && card->enhancement != CARD_ENHANCEMENT_STONE)
    {
        graphics_draw_text_formatted_center(font_small, g_enhancement_hint[card->enhancement], NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
        text_y += 8.0f;
    }

    if (card->edition != CARD_EDITION_BASE)
    {
        graphics_draw_text_formatted_center(font_small, g_edition_hint[card->edition], NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
        text_y += 8.0f;
    }

    if (card->seal != CARD_SEAL_NONE)
    {
        const char *seal_hints[] = { "", "#5Gold Seal#-: earn #5$3#-", "#5Red Seal#-: retrigger card", "#5Blue Seal#-: creates Planet", "#5Purple Seal#-: discard for Tarot" };
        graphics_draw_text_formatted_center(font_small, seal_hints[card->seal], NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
    }
}

void game_draw_joker_hint(struct Joker *joker)
{
    int line_count = 0;
    for (int i = HINT_JOKER_TYPE_LENGTH - 1; i >= 0; i--)
    {
        if (g_joker_types[joker->type].hint_lines[i][0] != '\0')
        {
            line_count = i + 1;
            break;
        }
    }

    float top = joker->draw.y + CARD_HEIGHT + 4.0f;
    float height = CARD_HINT_MIN_HEIGHT + 8.0f * line_count + 10.0f + (joker->edition != CARD_EDITION_BASE ? 10.0f : 0.0f);
    float left = joker->draw.x - CARD_HINT_WIDTH / 2.0f + CARD_WIDTH / 2.0f;
    if (left > SCREEN_WIDTH - CARD_HINT_WIDTH)
    {
        left = SCREEN_WIDTH - CARD_HINT_WIDTH;
    }

    if (top + height > SCREEN_HEIGHT)
    {
        top = joker->draw.y - height - 4.0f;
    }

    graphics_set_texture(-1, -1);
    graphics_draw_quad(left, top, CARD_HINT_WIDTH, height, 0, 0, 0, 0, COLOR_DARK_GREY);
    graphics_draw_quad(left + 2, top + CARD_HINT_MIN_HEIGHT - 12.0f, CARD_HINT_WIDTH - 4, height - CARD_HINT_MIN_HEIGHT + 8.0f, 0, 0, 0, 0, COLOR_WHITE);

    graphics_draw_text_center(font_small, g_joker_types[joker->type].name, left + CARD_HINT_WIDTH / 2.0f, top + 10.0f, 1.0f, COLOR_WHITE);

    for (int i = 0; i < line_count; i++)
    {
        graphics_draw_text_formatted_center(font_small, g_joker_types[joker->type].hint_lines[i], (void*)joker, left + CARD_HINT_WIDTH / 2.0f, top + 14.0f + 12.0f + (8.0f * i), 1.0f, COLOR_DARK_GREY_2);
    }

    float current_y = top + 14.0f + 12.0f + (8.0f * line_count);

    if (joker->edition != CARD_EDITION_BASE)
    {
        graphics_draw_text_formatted_center(font_small, g_edition_hint[joker->edition], NULL, left + CARD_HINT_WIDTH / 2.0f, current_y + 2.0f, 1.0f, COLOR_DARK_GREY_2);
        current_y += 10.0f;
    }

    graphics_draw_text_formatted_center(font_small, g_rarity_hint[g_joker_types[joker->type].rarity], NULL, left + CARD_HINT_WIDTH / 2.0f, current_y + 2.0f, 1.0f, COLOR_DARK_GREY_2);
}

void game_draw_tarot_hint(struct Tarot *tarot)
{
    int line_count = 0;
    for (int i = HINT_TAROT_TYPE_LENGTH - 1; i >= 0; i--)
    {
        if (g_tarot_types[tarot->type].hint_lines[i][0] != '\0')
        {
            line_count = i + 1;
            break;
        }
    }

    float top = tarot->draw.y + CARD_HEIGHT + 4.0f;
    float height = CARD_HINT_MIN_HEIGHT + 8.0f * line_count;
    float left = tarot->draw.x - CARD_HINT_WIDTH / 2.0f + CARD_WIDTH / 2.0f;
    if (left > SCREEN_WIDTH - CARD_HINT_WIDTH)
    {
        left = SCREEN_WIDTH - CARD_HINT_WIDTH;
    }

    if (top + height > SCREEN_HEIGHT)
    {
        top = tarot->draw.y - height - 4.0f;
    }

    graphics_set_texture(-1, -1);
    graphics_draw_quad(left, top, CARD_HINT_WIDTH, height, 0, 0, 0, 0, COLOR_DARK_GREY);
    graphics_draw_quad(left + 2, top + CARD_HINT_MIN_HEIGHT - 12.0f, CARD_HINT_WIDTH - 4, 8.0f * line_count + 8.0f, 0, 0, 0, 0, COLOR_WHITE);

    graphics_draw_text_center(font_small, g_tarot_types[tarot->type].name, left + CARD_HINT_WIDTH / 2.0f, top + 10.0f, 1.0f, COLOR_WHITE);


    for (int i = 0; i < line_count; i++)
    {
        graphics_draw_text_formatted_center(font_small, g_tarot_types[tarot->type].hint_lines[i], (void*)tarot, left + CARD_HINT_WIDTH / 2.0f, top + 14.0f + 12.0f + (8.0f * i), 1.0f, COLOR_DARK_GREY_2);
    }
}

void game_draw_spectral_hint(struct Spectral *spectral)
{
    int line_count = 0;
    for (int i = HINT_SPECTRAL_TYPE_LENGTH - 1; i >= 0; i--)
    {
        if (g_spectral_types[spectral->type].hint_lines[i][0] != '\0')
        {
            line_count = i + 1;
            break;
        }
    }

    float top = spectral->draw.y + CARD_HEIGHT + 4.0f;
    float height = CARD_HINT_MIN_HEIGHT + 8.0f * line_count;
    float left = spectral->draw.x - CARD_HINT_WIDTH / 2.0f + CARD_WIDTH / 2.0f;
    if (left > SCREEN_WIDTH - CARD_HINT_WIDTH)
    {
        left = SCREEN_WIDTH - CARD_HINT_WIDTH;
    }

    if (top + height > SCREEN_HEIGHT)
    {
        top = spectral->draw.y - height - 4.0f;
    }

    graphics_set_texture(-1, -1);
    graphics_draw_quad(left, top, CARD_HINT_WIDTH, height, 0, 0, 0, 0, 0xFF424E54);
    graphics_draw_quad(left + 2, top + CARD_HINT_MIN_HEIGHT - 12.0f, CARD_HINT_WIDTH - 4, 8.0f * line_count + 8.0f, 0, 0, 0, 0, COLOR_WHITE);

    graphics_draw_text_center(font_small, g_spectral_types[spectral->type].name, left + CARD_HINT_WIDTH / 2.0f, top + 10.0f, 1.0f, COLOR_WHITE);

    for (int i = 0; i < line_count; i++)
    {
        graphics_draw_text_formatted_center(font_small, g_spectral_types[spectral->type].hint_lines[i], (void*)spectral, left + CARD_HINT_WIDTH / 2.0f, top + 14.0f + 12.0f + (8.0f * i), 1.0f, COLOR_DARK_GREY_2);
    }
}

void game_draw_planet_hint(struct Planet *planet)
{
    char str[32];
    int line_count = 4;

    float top = planet->draw.y + CARD_HEIGHT + 4.0f;
    float height = CARD_HINT_MIN_HEIGHT + 8.0f * line_count;
    float left = planet->draw.x - CARD_HINT_WIDTH / 2.0f + CARD_WIDTH / 2.0f;
    if (left > SCREEN_WIDTH - CARD_HINT_WIDTH)
    {
        left = SCREEN_WIDTH - CARD_HINT_WIDTH;
    }

    if (top + height > SCREEN_HEIGHT)
    {
        top = planet->draw.y - height - 4.0f;
    }

    graphics_set_texture(-1, -1);
    graphics_draw_quad(left, top, CARD_HINT_WIDTH, height, 0, 0, 0, 0, COLOR_DARK_GREY);
    graphics_draw_quad(left + 2, top + CARD_HINT_MIN_HEIGHT - 12.0f, CARD_HINT_WIDTH - 4, 8.0f * line_count + 8.0f, 0, 0, 0, 0, COLOR_WHITE);

    graphics_draw_text_center(font_small, g_planet_types[planet->type].name, left + CARD_HINT_WIDTH / 2.0f, top + 10.0f, 1.0f, COLOR_WHITE);

    struct PlanetType *planet_type = &(g_planet_types[planet->type]);

    float text_y = top + 14.0f + 12.0f;
    sprintf(str, "[lvl.%d] Level Up", g_game_state.poker_hand_level[planet_type->poker_hand]);
    graphics_draw_text_formatted_center(font_small, str, (void*)planet, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
    text_y += 8.0f;
    sprintf(str, "#5%s", g_poker_hand_names[planet_type->poker_hand]);
    graphics_draw_text_formatted_center(font_small, str, (void*)planet, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
    text_y += 8.0f;
    sprintf(str, "#2+%d#- Mult and", planet_type->mult);
    graphics_draw_text_formatted_center(font_small, str, NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
    text_y += 8.0f;
    sprintf(str, "#3+%d#- chips", planet_type->chips);
    graphics_draw_text_formatted_center(font_small, str, NULL, left + CARD_HINT_WIDTH / 2.0f, text_y, 1.0f, COLOR_DARK_GREY_2);
}

void game_draw_booster_hint(struct BoosterPack *booster)
{
    struct BoosterPackType *booster_type = &(g_booster_types[booster->type][booster->size][booster->image]);

    int line_count = 0;
    for (int i = HINT_BOOSTER_TYPE_LENGTH - 1; i >= 0; i--)
    {
        if (booster_type->hint_lines[i][0] != '\0')
        {
            line_count = i + 1;
            break;
        }
    }

    float top = booster->draw.y + CARD_HEIGHT + 4.0f;
    float height = CARD_HINT_MIN_HEIGHT + 8.0f * line_count;
    float left = booster->draw.x - CARD_HINT_WIDTH / 2.0f + CARD_WIDTH / 2.0f;

    if (top + height > SCREEN_HEIGHT)
    {
        top = booster->draw.y - height - 14.0f;
    }

    graphics_set_texture(-1, -1);
    graphics_draw_quad(left, top, CARD_HINT_WIDTH, height, 0, 0, 0, 0, COLOR_DARK_GREY);
    graphics_draw_quad(left + 2, top + CARD_HINT_MIN_HEIGHT - 12.0f, CARD_HINT_WIDTH - 4, 8.0f * line_count + 8.0f, 0, 0, 0, 0, COLOR_WHITE);

    graphics_draw_text_center(font_small, booster_type->name, left + CARD_HINT_WIDTH / 2.0f, top + 10.0f, 1.0f, COLOR_WHITE);

    for (int i = 0; i < line_count; i++)
    {
        graphics_draw_text_formatted_center(font_small, booster_type->hint_lines[i], NULL, left + CARD_HINT_WIDTH / 2.0f, top + 14.0f + 12.0f + (8.0f * i), 1.0f, COLOR_DARK_GREY_2);
    }
}

bool g_freeze_cards = false;

float sin_table[256] = {
 0.0000000f,  0.0245412f,  0.0490677f,  0.0735646f,  0.0980171f,  0.1224107f,  0.1467305f,  0.1709619f,
 0.1950903f,  0.2191012f,  0.2429802f,  0.2667128f,  0.2902847f,  0.3136818f,  0.3368898f,  0.3598951f,
 0.3826835f,  0.4052413f,  0.4275551f,  0.4496113f,  0.4713967f,  0.4928982f,  0.5141027f,  0.5349976f,
 0.5555702f,  0.5758082f,  0.5956993f,  0.6152316f,  0.6343933f,  0.6531729f,  0.6715590f,  0.6895406f,
 0.7071068f,  0.7242470f,  0.7409511f,  0.7572088f,  0.7730104f,  0.7883464f,  0.8032075f,  0.8175848f,
 0.8314696f,  0.8448536f,  0.8577286f,  0.8700870f,  0.8819212f,  0.8932243f,  0.9039893f,  0.9142098f,
 0.9238795f,  0.9329928f,  0.9415441f,  0.9495282f,  0.9569404f,  0.9637761f,  0.9700313f,  0.9757021f,
 0.9807853f,  0.9852777f,  0.9891765f,  0.9924796f,  0.9951847f,  0.9972904f,  0.9987954f,  0.9996988f,
 1.0000000f,  0.9996988f,  0.9987954f,  0.9972904f,  0.9951847f,  0.9924795f,  0.9891765f,  0.9852777f,
 0.9807853f,  0.9757022f,  0.9700313f,  0.9637761f,  0.9569403f,  0.9495282f,  0.9415441f,  0.9329928f,
 0.9238796f,  0.9142097f,  0.9039893f,  0.8932243f,  0.8819213f,  0.8700870f,  0.8577286f,  0.8448535f,
 0.8314697f,  0.8175848f,  0.8032075f,  0.7883465f,  0.7730105f,  0.7572088f,  0.7409511f,  0.7242471f,
 0.7071068f,  0.6895405f,  0.6715590f,  0.6531729f,  0.6343933f,  0.6152315f,  0.5956994f,  0.5758082f,
 0.5555702f,  0.5349977f,  0.5141028f,  0.4928981f,  0.4713968f,  0.4496114f,  0.4275551f,  0.4052412f,
 0.3826835f,  0.3598951f,  0.3368898f,  0.3136818f,  0.2902847f,  0.2667127f,  0.2429801f,  0.2191013f,
 0.1950903f,  0.1709618f,  0.1467306f,  0.1224107f,  0.0980171f,  0.0735644f,  0.0490677f,  0.0245412f,
-0.0000001f, -0.0245411f, -0.0490677f, -0.0735646f, -0.0980170f, -0.1224106f, -0.1467305f, -0.1709620f,
-0.1950902f, -0.2191012f, -0.2429802f, -0.2667127f, -0.2902847f, -0.3136818f, -0.3368900f, -0.3598950f,
-0.3826834f, -0.4052414f, -0.4275550f, -0.4496113f, -0.4713968f, -0.4928983f, -0.5141027f, -0.5349976f,
-0.5555703f, -0.5758081f, -0.5956993f, -0.6152316f, -0.6343932f, -0.6531728f, -0.6715590f, -0.6895406f,
-0.7071067f, -0.7242471f, -0.7409512f, -0.7572088f, -0.7730104f, -0.7883465f, -0.8032076f, -0.8175849f,
-0.8314695f, -0.8448535f, -0.8577285f, -0.8700870f, -0.8819213f, -0.8932244f, -0.9039894f, -0.9142097f,
-0.9238795f, -0.9329928f, -0.9415441f, -0.9495282f, -0.9569404f, -0.9637761f, -0.9700312f, -0.9757021f,
-0.9807853f, -0.9852777f, -0.9891765f, -0.9924796f, -0.9951847f, -0.9972904f, -0.9987954f, -0.9996988f,
-1.0000000f, -0.9996988f, -0.9987954f, -0.9972904f, -0.9951847f, -0.9924796f, -0.9891765f, -0.9852777f,
-0.9807853f, -0.9757021f, -0.9700312f, -0.9637761f, -0.9569404f, -0.9495282f, -0.9415441f, -0.9329928f,
-0.9238794f, -0.9142098f, -0.9039894f, -0.8932244f, -0.8819213f, -0.8700870f, -0.8577285f, -0.8448535f,
-0.8314697f, -0.8175849f, -0.8032076f, -0.7883464f, -0.7730104f, -0.7572088f, -0.7409510f, -0.7242472f,
-0.7071069f, -0.6895406f, -0.6715590f, -0.6531728f, -0.6343932f, -0.6152315f, -0.5956995f, -0.5758083f,
-0.5555703f, -0.5349976f, -0.5141027f, -0.4928981f, -0.4713965f, -0.4496115f, -0.4275552f, -0.4052414f,
-0.3826834f, -0.3598950f, -0.3368897f, -0.3136815f, -0.2902848f, -0.2667129f, -0.2429802f, -0.2191012f,
-0.1950902f, -0.1709617f, -0.1467302f, -0.1224108f, -0.0980172f, -0.0735646f, -0.0490676f, -0.0245411f
};

float cos_table[256] = {
 1.0000000f,  0.9996988f,  0.9987954f,  0.9972904f,  0.9951847f,  0.9924796f,  0.9891765f,  0.9852777f,
 0.9807853f,  0.9757021f,  0.9700313f,  0.9637761f,  0.9569404f,  0.9495282f,  0.9415441f,  0.9329928f,
 0.9238795f,  0.9142098f,  0.9039893f,  0.8932243f,  0.8819213f,  0.8700870f,  0.8577286f,  0.8448536f,
 0.8314696f,  0.8175848f,  0.8032075f,  0.7883465f,  0.7730105f,  0.7572088f,  0.7409511f,  0.7242471f,
 0.7071068f,  0.6895406f,  0.6715590f,  0.6531729f,  0.6343933f,  0.6152316f,  0.5956993f,  0.5758082f,
 0.5555702f,  0.5349976f,  0.5141028f,  0.4928982f,  0.4713968f,  0.4496113f,  0.4275551f,  0.4052413f,
 0.3826834f,  0.3598951f,  0.3368898f,  0.3136818f,  0.2902846f,  0.2667128f,  0.2429802f,  0.2191012f,
 0.1950904f,  0.1709619f,  0.1467305f,  0.1224106f,  0.0980171f,  0.0735646f,  0.0490676f,  0.0245413f,
-0.0000000f, -0.0245412f, -0.0490676f, -0.0735646f, -0.0980171f, -0.1224107f, -0.1467305f, -0.1709619f,
-0.1950903f, -0.2191012f, -0.2429802f, -0.2667127f, -0.2902847f, -0.3136817f, -0.3368898f, -0.3598951f,
-0.3826834f, -0.4052413f, -0.4275551f, -0.4496114f, -0.4713966f, -0.4928982f, -0.5141028f, -0.5349977f,
-0.5555702f, -0.5758082f, -0.5956994f, -0.6152315f, -0.6343933f, -0.6531729f, -0.6715590f, -0.6895405f,
-0.7071068f, -0.7242472f, -0.7409511f, -0.7572088f, -0.7730105f, -0.7883465f, -0.8032075f, -0.8175848f,
-0.8314697f, -0.8448535f, -0.8577286f, -0.8700870f, -0.8819212f, -0.8932243f, -0.9039893f, -0.9142098f,
-0.9238795f, -0.9329928f, -0.9415441f, -0.9495282f, -0.9569404f, -0.9637761f, -0.9700313f, -0.9757021f,
-0.9807853f, -0.9852777f, -0.9891765f, -0.9924795f, -0.9951847f, -0.9972905f, -0.9987954f, -0.9996988f,
-1.0000000f, -0.9996988f, -0.9987954f, -0.9972904f, -0.9951847f, -0.9924796f, -0.9891765f, -0.9852777f,
-0.9807853f, -0.9757021f, -0.9700313f, -0.9637761f, -0.9569404f, -0.9495282f, -0.9415441f, -0.9329928f,
-0.9238795f, -0.9142097f, -0.9039893f, -0.8932243f, -0.8819212f, -0.8700869f, -0.8577287f, -0.8448536f,
-0.8314695f, -0.8175849f, -0.8032075f, -0.7883464f, -0.7730106f, -0.7572089f, -0.7409511f, -0.7242470f,
-0.7071068f, -0.6895406f, -0.6715589f, -0.6531729f, -0.6343933f, -0.6152316f, -0.5956992f, -0.5758080f,
-0.5555704f, -0.5349978f, -0.5141028f, -0.4928982f, -0.4713967f, -0.4496112f, -0.4275549f, -0.4052415f,
-0.3826836f, -0.3598951f, -0.3368899f, -0.3136817f, -0.2902845f, -0.2667126f, -0.2429804f, -0.2191014f,
-0.1950904f, -0.1709619f, -0.1467304f, -0.1224105f, -0.0980169f, -0.0735648f, -0.0490678f, -0.0245413f,
 0.0000000f,  0.0245413f,  0.0490678f,  0.0735648f,  0.0980170f,  0.1224106f,  0.1467304f,  0.1709619f,
 0.1950904f,  0.2191014f,  0.2429804f,  0.2667126f,  0.2902846f,  0.3136817f,  0.3368899f,  0.3598951f,
 0.3826836f,  0.4052411f,  0.4275549f,  0.4496112f,  0.4713967f,  0.4928982f,  0.5141028f,  0.5349978f,
 0.5555701f,  0.5758080f,  0.5956993f,  0.6152316f,  0.6343933f,  0.6531729f,  0.6715591f,  0.6895404f,
 0.7071066f,  0.7242470f,  0.7409511f,  0.7572089f,  0.7730106f,  0.7883465f,  0.8032074f,  0.8175848f,
 0.8314696f,  0.8448536f,  0.8577287f,  0.8700871f,  0.8819214f,  0.8932242f,  0.9039893f,  0.9142097f,
 0.9238796f,  0.9329928f,  0.9415441f,  0.9495283f,  0.9569403f,  0.9637761f,  0.9700313f,  0.9757021f,
 0.9807853f,  0.9852777f,  0.9891766f,  0.9924795f,  0.9951847f,  0.9972904f,  0.9987954f,  0.9996988f
};

void game_draw_get_oscillating_position(struct DrawObject *draw, float *x, float *y)
{
    *x = draw->x;
    *y = draw->y;
}

// Bounce rapido de escala (aumenta e diminui): ±5% com periodo curto (~0.4s).
// Substitui o antigo "girinho leve" (orbita de ±1px em x/y).
float game_draw_get_bounce_scale(struct DrawObject *draw)
{
    if (!g_settings.move_cards || g_freeze_cards)
    {
        return 1.0f;
    }
    float t = (float)(g_time + draw->r) / 4.0f;
    float t2 = t / (2 * M_PI);
    int t3 = (int)((t2 - (float)((int)t2)) * 256.0f);
    t3 = CLAMP(t3, 0, 255);
    return 1.0f + sin_table[t3] * 0.05f;
}

uint32_t game_draw_lerp_colour(uint32_t colour_a, uint32_t colour_b, float t)
{
    t = CLAMP(t, 0.0f, 1.0f);
    int a_a = (colour_a >> 24) & 0xFF;
    int r_a = colour_a & 0xFF;
    int g_a = (colour_a >> 8) & 0xFF;
    int b_a = (colour_a >> 16) & 0xFF;
    int a_b = (colour_b >> 24) & 0xFF;
    int r_b = colour_b & 0xFF;
    int g_b = (colour_b >> 8) & 0xFF;
    int b_b = (colour_b >> 16) & 0xFF;
    int a = a_a + (int)((float)(a_b - a_a) * t);
    int r = r_a + (int)((float)(r_b - r_a) * t);
    int g = g_a + (int)((float)(g_b - g_a) * t);
    int b = b_a + (int)((float)(b_b - b_a) * t);
	    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
	}

		void game_draw_card_effect_quad(float card_x, float card_y, float card_w, float card_h, float local_x, float local_y, float w, float h, uint32_t colour, float angle)
	{
	    float x = card_x + local_x;
	    float y = card_y + local_y;
	    float right = x + w;
	    float bottom = y + h;
	    float card_right = card_x + card_w;
	    float card_bottom = card_y + card_h;

	    if (x < card_x)
	    {
		w -= card_x - x;
		x = card_x;
	    }
	    if (y < card_y)
	    {
		h -= card_y - y;
		y = card_y;
	    }
	    if (right > card_right) w -= right - card_right;
	    if (bottom > card_bottom) h -= bottom - card_bottom;
	    if (w <= 0.0f || h <= 0.0f) return;

	    graphics_draw_rotated_quad(x, y, w, h, 0, 0, 0, 0, colour, angle);
	}

	void game_draw_edition_shader(int edition, float x, float y, float w, float h, float angle, float seed)
	{
	    if (edition == CARD_EDITION_BASE) return;

	    float time = (float)g_time * 0.035f + seed * 0.17f;
	    graphics_set_no_texture();

	    if (edition == CARD_EDITION_NEGATIVE)
	    {
		graphics_draw_rotated_quad(x + 1.0f, y + 1.0f, w - 2.0f, h - 2.0f, 0, 0, 0, 0, 0x70453B34, angle);

		for (int i = 0; i < 5; i++)
		{
		    float phase = time + (float)i * 1.37f;
		    float px = 3.0f + fmodf((phase * 9.0f + (float)i * 13.0f), w - 10.0f);
		    uint32_t colour = (i & 1) ? 0x385E5EBC : 0x3838599A;
		    game_draw_card_effect_quad(x, y, w, h, px, 5.0f, 2.0f, h - 10.0f, colour, angle);
		}
		return;
	    }

	    if (edition == CARD_EDITION_FOIL)
	    {
		float pulse = 0.5f + 0.5f * sinf(time * 1.8f);
		uint32_t cool_tint = game_draw_lerp_colour(0x1AB6CAD8, 0x26FFFFFF, pulse);
		graphics_draw_rotated_quad(x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, 0, 0, 0, 0, cool_tint, angle);
		graphics_draw_rotated_quad(x + 6.0f, y + 6.0f, w - 12.0f, h - 12.0f, 0, 0, 0, 0, 0x12FFFFFF, angle);
		return;
	    }

	    if (edition == CARD_EDITION_HOLOGRAPHIC)
	    {
		float pulse_a = 0.5f + 0.5f * sinf(time * 1.35f);
		float pulse_b = 0.5f + 0.5f * cosf(time * 1.07f + 1.4f);
		graphics_draw_rotated_quad(x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, 0, 0, 0, 0, game_draw_lerp_colour(0x26FFC7EE, 0x2CE6F1FF, pulse_a), angle);
		graphics_draw_rotated_quad(x + 4.0f, y + 4.0f, w - 8.0f, h - 8.0f, 0, 0, 0, 0, game_draw_lerp_colour(0x18B6E9FF, 0x20FFDBF5, pulse_b), angle);

		float pattern_left = 5.0f;
		float pattern_top = 6.0f;
		float pattern_w = w - 10.0f;
		float pattern_h = h - 12.0f;
		int cols = 5;
		int rows = 8;
		float cell_w = pattern_w / (float)cols;
		float cell_h = pattern_h / (float)rows;
		for (int row = 0; row < rows; row++)
		{
		    for (int col = 0; col < cols; col++)
		    {
			float local_x = pattern_left + (float)col * cell_w;
			float local_y = pattern_top + (float)row * cell_h;
			float shimmer = 0.5f + 0.5f * sinf(time * 1.4f + (float)(row * 3 + col) * 0.73f);
			uint32_t colour = game_draw_lerp_colour(0x18FFFFFF, 0x26FFB9F1, shimmer);
			game_draw_card_effect_quad(x, y, w, h, local_x + cell_w * 0.45f, local_y, 1.0f, cell_h * 0.8f, colour, angle);
			game_draw_card_effect_quad(x, y, w, h, local_x, local_y + cell_h * 0.45f, cell_w * 0.8f, 1.0f, colour, angle);
			if (((row + col) & 1) == 0)
			{
			    game_draw_card_effect_quad(x, y, w, h, local_x + cell_w * 0.20f, local_y + cell_h * 0.20f, 2.0f, 2.0f, 0x22FFFFFF, angle);
			    game_draw_card_effect_quad(x, y, w, h, local_x + cell_w * 0.65f, local_y + cell_h * 0.65f, 2.0f, 2.0f, 0x1C8DEBFF, angle);
			}
		    }
		}
		graphics_draw_rotated_quad(x + 8.0f, y + 8.0f, w - 16.0f, h - 16.0f, 0, 0, 0, 0, 0x10FFFFFF, angle);
		return;
	    }

	    if (edition == CARD_EDITION_POLYCHROME)
	    {
		float pulse_a = 0.5f + 0.5f * sinf(time * 1.12f);
		float pulse_b = 0.5f + 0.5f * sinf(time * 0.87f + 2.1f);
		uint32_t rainbow_a = game_draw_lerp_colour(0x22FF4D72, 0x22FFE04D, pulse_a);
		uint32_t rainbow_b = game_draw_lerp_colour(0x224DBDFF, 0x22B84DFF, pulse_b);
		graphics_draw_rotated_quad(x + 2.0f, y + 2.0f, w - 4.0f, h - 4.0f, 0, 0, 0, 0, rainbow_a, angle);
		graphics_draw_rotated_quad(x + 4.0f, y + 4.0f, w - 8.0f, h - 8.0f, 0, 0, 0, 0, rainbow_b, angle);
		graphics_draw_rotated_quad(x + 8.0f, y + 8.0f, w - 16.0f, h - 16.0f, 0, 0, 0, 0, 0x14FFFFFF, angle);
	    }
	}

// Centro do leque para o tilt base (cardtest: ROT_MAX 4° no centro da mão).
// O chamador ajusta por grupo (mão: centro da mão; resto: centro da tela).
static float g_tilt_center_x = SCREEN_WIDTH / 2.0f;

// Layout compartilhado da carta (cardtest afinado): posicao oscilante,
// bob idle +-0.5px, tilt mola 500/20 + base lateral + wave +-0.77g.
// step_physics=false apenas le tilt_w (pass de sombra); true integra a
// mola (chamar uma vez por carta por frame).
// Quando true, as cartas desenham retas no mat: sem leque, sem wave.
// So aparece o giro de arrasto (drag_angle, pivo na base), que relaxa
// sozinho para 0. Ligado so nos loops das cartas jogadas.
static bool g_mat_straight = false;

static void game_draw_card_layout(struct Card *card, struct DrawObject *draw, float *out_x, float *out_y, float *out_w, float *out_h, float *out_scale, bool step_physics)
{
    float x, y;
    game_draw_get_oscillating_position(draw, &x, &y);
    // Escala visível: base + hover/juice do Moveable (fora da mão = 1.0).
    float eff_scale = draw->scale * game_get_card_motion_scale(card);

    // Ondulacao idle (cardtest afinado: tilt +-0.77g, flutuacao +-0.5px).
    float wt = (float)(g_time + draw->r) / 40.0f;
    float wt2 = wt / (2 * M_PI);
    int wt3 = (int)((wt2 - (float)((int)wt2)) * 256.0f);
    wt3 = CLAMP(wt3, 0, 255);
    y += sin_table[(wt3 + 64) & 255] * 0.5f;

    x -= ((CARD_WIDTH * eff_scale) - CARD_WIDTH) / 2.0f;
    y -= ((CARD_HEIGHT * eff_scale) - CARD_HEIGHT) / 2.0f;
    float w = CARD_WIDTH * eff_scale;
    float h = CARD_HEIGHT * eff_scale;
    if (g_mat_straight)
    {
        // Mat: reta. O arrasto gira em torno do topo-centro (pivo): desloca
        // o desenho para a rotacao (que e em torno do centro) manter o topo
        // parado — a base balanca para o lado oposto ao movimento.
        if (step_physics)
        {
            draw->drag_angle += (0.0f - draw->drag_angle) * 0.2f;
            if (draw->drag_angle > -0.0005f && draw->drag_angle < 0.0005f) draw->drag_angle = 0.0f;
        }
        card->draw.angle = draw->drag_angle;
        x -= sinf(card->draw.angle) * (h / 2.0f);
        y -= (1.0f - cosf(card->draw.angle)) * (h / 2.0f);
    }
    else
    // Tilt (cardtest: leque ±4° em torno do centro, mola 500/20, kick 4.3)
    {
        float base = -((g_tilt_center_x - x) / (SCREEN_WIDTH / 2.0f)) * 0.13f;
        if (step_physics)
        {
            float force = -500.0f * draw->tilt_w - 20.0f * draw->tilt_wv;
            draw->tilt_wv += force * (1.0f / 60.0f);
            draw->tilt_w += draw->tilt_wv * (1.0f / 60.0f);
        }
        card->draw.angle = base + draw->tilt_w + sin_table[wt3] * 0.0134f +
                           game_get_card_motion_sway(card);
    }

    *out_x = x;
    *out_y = y;
    *out_w = w;
    *out_h = h;
    *out_scale = eff_scale;
}

static void game_draw_card_shadow_at(float x, float y, float w, float h, float angle)
{
    // Sombra suave, gira junto com a carta (lado oposto ao centro).
    float shx = -((x + w / 2.0f - SCREEN_WIDTH / 2.0f) / (SCREEN_WIDTH / 2.0f)) * 6.0f;
    graphics_set_no_texture();
    graphics_draw_rotated_quad(x + shx - 2, y + 2, w + 4, h + 4, 0, 0, 0, 0, 0x18000000, angle);
    graphics_draw_rotated_quad(x + shx, y + 4, w, h, 0, 0, 0, 0, 0x40000000, angle);
}

// Pass de sombra (cardtest): todas as sombras primeiro, nenhum corpo recebe
// sombra de outra carta por cima. Nao integra a fisica (so le).
void game_draw_card_shadow(struct Card *card, struct DrawObject *draw_override)
{
    struct DrawObject *draw = &card->draw;
    if (draw_override != NULL)
    {
        draw = draw_override;
    }
    float x, y, w, h, eff_scale;
    game_draw_card_layout(card, draw, &x, &y, &w, &h, &eff_scale, false);
    game_draw_card_shadow_at(x, y, w, h, card->draw.angle);
}

	void game_draw_card_body(struct Card *card, struct DrawObject *draw_override)
	{
	    struct DrawObject *draw = &card->draw;
	    if (draw_override != NULL)
	    {
		draw = draw_override;
	    }

    float x, y, w, h, eff_scale;
    game_draw_card_layout(card, draw, &x, &y, &w, &h, &eff_scale, true);

	    if (card->face_down)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_rotated_quad(
            x, y, w, h,
            1 + g_deck_types[g_game_state.deck_type].u * (TEXTURE_CARD_WIDTH + 2),
            1 + g_deck_types[g_game_state.deck_type].v * (TEXTURE_CARD_HEIGHT + 2),
            TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, card->draw.angle);
        if (card->draw.white_factor > 0.0f)
        {
            uint32_t color = 0xFFFFFF | ((uint32_t)(255.0f * (card->draw.white_factor > 1.0f ? 1.0f : card->draw.white_factor))<<24);
            graphics_draw_rotated_quad(x, y, w, h, 0, 0, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, color, card->draw.angle);
        }
        return;
    }

    if (card->edition == CARD_EDITION_NEGATIVE)
    {
        graphics_flush_quads();
        sceGuEnable(GU_COLOR_LOGIC_OP);
        sceGuLogicalOp(GU_COPY_INVERTED);
        sceGuEnable(GU_ALPHA_TEST);
        sceGuAlphaFunc(GU_GREATER, 0, 0xFF);
    }

    graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_rotated_quad(x, y, w, h, g_enhancement_tex_coords[card->enhancement].x, g_enhancement_tex_coords[card->enhancement].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, card->draw.angle);

    if (card->enhancement != CARD_ENHANCEMENT_STONE)
    {
        if (card->rank < 7)
        {
            graphics_set_texture(tex_deck, GRAPHICS_TEXTURE_FILTER_LINEAR);
            graphics_draw_rotated_quad(x, y, w, h, 1 + (TEXTURE_CARD_WIDTH + 2) * card->rank, 1 + (TEXTURE_CARD_HEIGHT + 2) * card->suit, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, card->draw.angle);
        }
        else
        {
            graphics_set_texture(tex_deck2, GRAPHICS_TEXTURE_FILTER_LINEAR);
            graphics_draw_rotated_quad(x, y, w, h, 1 + (TEXTURE_CARD_WIDTH + 2) * (card->rank - 7), 1 + (TEXTURE_CARD_HEIGHT + 2) * card->suit, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, card->draw.angle);
        }
    }

    if (card->edition == CARD_EDITION_NEGATIVE)
    {
        graphics_flush_quads();
        sceGuDisable(GU_ALPHA_TEST);
        sceGuDisable(GU_COLOR_LOGIC_OP);
    }

    if (card->edition != CARD_EDITION_BASE && card->edition != CARD_EDITION_NEGATIVE)
    {
        graphics_set_texture(tex_editions, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_rotated_quad(x, y, w, h, g_editions_tex_coords[card->edition].x, g_editions_tex_coords[card->edition].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0x7FFFFFFF, card->draw.angle);
    }
    game_draw_edition_shader(card->edition, x, y, w, h, card->draw.angle, card->draw.r);

    if (card->seal != CARD_SEAL_NONE)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_rotated_quad(x, y, w, h, g_seal_tex_coords[card->seal].x, g_seal_tex_coords[card->seal].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, card->draw.angle);
    }

    if (game_util_is_card_debuffed(card))
    {
        graphics_set_no_texture();
        game_draw_card_effect_quad(x, y, w, h, 4.0f * eff_scale, 27.0f * eff_scale, 40.0f * eff_scale, 9.0f * eff_scale, 0xCC0000FF, card->draw.angle + 0.78f);
        game_draw_card_effect_quad(x, y, w, h, 4.0f * eff_scale, 29.0f * eff_scale, 40.0f * eff_scale, 9.0f * eff_scale, 0xCC0000FF, card->draw.angle - 0.78f);
    }

    if (card->draw.white_factor > 0.0f)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        uint32_t color = 0xFFFFFF | ((uint32_t)(255.0f * (card->draw.white_factor > 1.0f ? 1.0f : card->draw.white_factor))<<24);
        graphics_draw_quad(x, y, w, h, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, color);
    }
}

// Sombra+corpo juntos (shop, deck info...). Na mao o chamador usa
// game_draw_card_shadow/game_draw_card_body em 2 passes (cardtest).
void game_draw_card(struct Card *card, struct DrawObject *draw_override)
{
    game_draw_card_shadow(card, draw_override);
    game_draw_card_body(card, draw_override);
}

void game_draw_joker(struct Joker *joker)
{
    int tex_joker_x, tex_joker_y = 0;
    struct JokerType *joker_type = &g_joker_types[joker->type];
    tex_joker_x = joker_type->u / 7;
    tex_joker_y = joker_type->v / 5;

    float x, y;
    game_draw_get_oscillating_position(&(joker->draw), &x, &y);
    float bounce = game_draw_get_bounce_scale(&(joker->draw));
    float eff_scale = joker->draw.scale * bounce;

    x -= ((CARD_WIDTH * eff_scale) - CARD_WIDTH) / 2.0f;
    y -= ((CARD_HEIGHT * eff_scale) - CARD_HEIGHT) / 2.0f;
    float w = CARD_WIDTH * eff_scale;
    float h = CARD_HEIGHT * eff_scale;

    if (joker->face_down)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_rotated_quad(
            x, y, w, h,
            1 + g_deck_types[g_game_state.deck_type].u * (TEXTURE_CARD_WIDTH + 2),
            1 + g_deck_types[g_game_state.deck_type].v * (TEXTURE_CARD_HEIGHT + 2),
            TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, joker->draw.angle);
        return;
    }

    if (joker->edition == CARD_EDITION_NEGATIVE)
    {
        graphics_flush_quads();
        sceGuEnable(GU_COLOR_LOGIC_OP);
        sceGuLogicalOp(GU_COPY_INVERTED);
        sceGuEnable(GU_ALPHA_TEST);
        sceGuAlphaFunc(GU_GREATER, 0, 0xFF);
    }

    graphics_set_texture(tex_jokers[tex_joker_x][tex_joker_y], GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_rotated_quad(
        x, y, w, h,
        1 + (joker_type->u - tex_joker_x * 7) * (TEXTURE_CARD_WIDTH + 2),
        1 + (joker_type->v - tex_joker_y * 5) * (TEXTURE_CARD_HEIGHT + 2),
        TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0xFFFFFFFF, joker->draw.angle);

    if (joker->type == JOKER_TYPE_HOLOGRAM)
    {
        tex_joker_x = 2 / 7;
        tex_joker_y = 9 / 5;
        graphics_set_texture(tex_jokers[tex_joker_x][tex_joker_y], GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_rotated_quad(
            x, y, w, h,
            1 + (2 - tex_joker_x * 7) * (TEXTURE_CARD_WIDTH + 2),
            1 + (9 - tex_joker_y * 5) * (TEXTURE_CARD_HEIGHT + 2),
            TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0xFFFFFFFF, joker->draw.angle);
    }

    if (joker->edition == CARD_EDITION_NEGATIVE)
    {
        graphics_flush_quads();
        sceGuDisable(GU_ALPHA_TEST);
        sceGuDisable(GU_COLOR_LOGIC_OP);
    }

    if (joker->edition != CARD_EDITION_BASE && joker->edition != CARD_EDITION_NEGATIVE)
    {
        graphics_set_texture(tex_editions, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_rotated_quad(x, y, w, h, g_editions_tex_coords[joker->edition].x, g_editions_tex_coords[joker->edition].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0x7FFFFFFF, joker->draw.angle);
    }
    game_draw_edition_shader(joker->edition, x, y, w, h, joker->draw.angle, joker->draw.r);

    if (joker->disabled)
    {
        graphics_set_no_texture();
        game_draw_card_effect_quad(x, y, w, h, 4.0f * eff_scale, 27.0f * eff_scale, 40.0f * eff_scale, 9.0f * eff_scale, 0xCC0000FF, joker->draw.angle + 0.78f);
        game_draw_card_effect_quad(x, y, w, h, 4.0f * eff_scale, 29.0f * eff_scale, 40.0f * eff_scale, 9.0f * eff_scale, 0xCC0000FF, joker->draw.angle - 0.78f);
    }
}

void game_draw_card_frame(struct DrawObject *draw)
{
    float card_x = draw->x, card_y = draw->y;
    graphics_set_no_texture();
    graphics_draw_quad(card_x - 3.0f, card_y - 3.0f, CARD_WIDTH + 6.0f, 2.0f, 0, 0, 0, 0, COLOR_WHITE);
    graphics_draw_quad(card_x - 3.0f, card_y - 3.0f, 2.0f, CARD_HEIGHT + 6.0f, 0, 0, 0, 0, COLOR_WHITE);
    graphics_draw_quad(card_x - 3.0f, card_y + CARD_HEIGHT + 1.0f, CARD_WIDTH + 6.0f, 2.0f, 0, 0, 0, 0, COLOR_WHITE);
    graphics_draw_quad(card_x + CARD_WIDTH + 1.0f, card_y - 3.0f, 2.0f, CARD_HEIGHT + 6.0f, 0, 0, 0, 0, COLOR_WHITE);
}

void game_draw_booster_frame(struct DrawObject *draw)
{
    float card_x = draw->x, card_y = draw->y;
    graphics_set_no_texture();
    graphics_draw_quad(card_x - 3.0f, card_y - 3.0f, BOOSTER_WIDTH + 6.0f, 2.0f, 0, 0, 0, 0, COLOR_WHITE);
    graphics_draw_quad(card_x - 3.0f, card_y - 3.0f, 2.0f, BOOSTER_HEIGHT + 6.0f, 0, 0, 0, 0, COLOR_WHITE);
    graphics_draw_quad(card_x - 3.0f, card_y + BOOSTER_HEIGHT + 1.0f, BOOSTER_WIDTH + 6.0f, 2.0f, 0, 0, 0, 0, COLOR_WHITE);
    graphics_draw_quad(card_x + BOOSTER_WIDTH + 1.0f, card_y - 3.0f, 2.0f, BOOSTER_HEIGHT + 6.0f, 0, 0, 0, 0, COLOR_WHITE);
}

void game_draw_item_button(struct DrawObject *draw, int gamepad_ui, char *text, char *text2, uint32_t button_color, bool left)
{
    float item_x = draw->x, item_y = draw->y;
    float x = left ? item_x - 40.0f : item_x + CARD_WIDTH;
    float height = text2 != NULL ? 35.0f : 30.0f;
    graphics_set_no_texture();
    graphics_draw_quad(x, item_y + (CARD_HEIGHT / 2.0f) - 20.0f, 40.0f, height, 0, 0, 0, 0, button_color);
    graphics_set_texture(tex_gamepad_ui, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_quad(x + 7.0f, item_y + (CARD_HEIGHT / 2.0f) - 22.0f, 18.0f, 18.0f,
        g_gamepad_ui_text_coords[gamepad_ui].x,
        g_gamepad_ui_text_coords[gamepad_ui].y,
        32, 32, COLOR_WHITE);
    graphics_draw_text_center(font_small, text, x + 20.0f, item_y + (CARD_HEIGHT / 2.0f), 1.0f, COLOR_WHITE);
    if (text2 != NULL) graphics_draw_text_center(font_small, text2, x + 20.0f, item_y + (CARD_HEIGHT / 2.0f) + 10.0f, 1.0f, COLOR_WHITE);
}

void game_draw_price_tag(struct DrawObject *draw, int price)
{
    float card_x = draw->x, card_y = draw->y;
    graphics_set_no_texture();
    graphics_draw_quad(card_x + (CARD_WIDTH / 2.0f) - 15.0f, card_y - 15.0f, 30.0f, 16.0f, 0, 0, 0, 0, COLOR_DARK_GREY_2);
    char str[16];
    sprintf(str, "$%d", price);
    graphics_draw_text_center(font_small, str, card_x + (CARD_WIDTH / 2.0f), card_y - 7.0f, 1.0f, COLOR_SCORE_NUMBER_TEXT_MONEY);
}

void game_draw_tarot(struct Tarot *tarot)
{
    int tex_tarot_x, tex_tarot_y = 0;
    struct TarotType *tarot_type = &g_tarot_types[tarot->type];
    tex_tarot_x = tarot_type->u / 7;
    tex_tarot_y = tarot_type->v / 5;

    float x, y;
    game_draw_get_oscillating_position(&(tarot->draw), &x, &y);
    float bounce = game_draw_get_bounce_scale(&(tarot->draw));
    float eff_scale = tarot->draw.scale * bounce;
    x -= ((CARD_WIDTH * eff_scale) - CARD_WIDTH) / 2.0f;
    y -= ((CARD_HEIGHT * eff_scale) - CARD_HEIGHT) / 2.0f;
    float w = CARD_WIDTH * eff_scale;
    float h = CARD_HEIGHT * eff_scale;

    graphics_set_texture(tex_tarots[tex_tarot_x][tex_tarot_y], GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_rotated_quad(
        x, y, w, h,
        1 + (tarot_type->u - tex_tarot_x * 7) * (TEXTURE_CARD_WIDTH + 2),
        1 + (tarot_type->v - tex_tarot_y * 5) * (TEXTURE_CARD_HEIGHT + 2),
        TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, tarot->draw.angle);

    if (tarot->draw.white_factor > 0.0f)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        uint32_t color = 0xFFFFFF | ((uint32_t)(255.0f * (tarot->draw.white_factor > 1.0f ? 1.0f : tarot->draw.white_factor))<<24);
        graphics_draw_quad(x, y, w, h, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, color);
    }
}

void game_draw_spectral(struct Spectral *spectral)
{
    int tex_spectral_x, tex_spectral_y = 0;
    struct SpectralType *spectral_type = &g_spectral_types[spectral->type];
    tex_spectral_x = spectral_type->u / 7;
    tex_spectral_y = spectral_type->v / 5;

    float x, y;
    game_draw_get_oscillating_position(&(spectral->draw), &x, &y);
    float bounce = game_draw_get_bounce_scale(&(spectral->draw));
    float eff_scale = spectral->draw.scale * bounce;
    x -= ((CARD_WIDTH * eff_scale) - CARD_WIDTH) / 2.0f;
    y -= ((CARD_HEIGHT * eff_scale) - CARD_HEIGHT) / 2.0f;
    float w = CARD_WIDTH * eff_scale;
    float h = CARD_HEIGHT * eff_scale;

    graphics_set_texture(tex_tarots[tex_spectral_x][tex_spectral_y], GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_rotated_quad(
        x, y, w, h,
        1 + (spectral_type->u - tex_spectral_x * 7) * (TEXTURE_CARD_WIDTH + 2),
        1 + (spectral_type->v - tex_spectral_y * 5) * (TEXTURE_CARD_HEIGHT + 2),
        TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, spectral->draw.angle);

    if (spectral->draw.white_factor > 0.0f)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        uint32_t color = 0xFFFFFF | ((uint32_t)(255.0f * (spectral->draw.white_factor > 1.0f ? 1.0f : spectral->draw.white_factor))<<24);
        graphics_draw_quad(x, y, w, h, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, color);
    }
}

void game_draw_planet(struct Planet *planet)
{
    int tex_planet_x, tex_planet_y = 0;
    struct PlanetType *planet_type = &g_planet_types[planet->type];
    tex_planet_x = planet_type->u / 7;
    tex_planet_y = planet_type->v / 5;

    float x, y;
    game_draw_get_oscillating_position(&(planet->draw), &x, &y);
    float bounce = game_draw_get_bounce_scale(&(planet->draw));
    float eff_scale = planet->draw.scale * bounce;
    x -= ((CARD_WIDTH * eff_scale) - CARD_WIDTH) / 2.0f;
    y -= ((CARD_HEIGHT * eff_scale) - CARD_HEIGHT) / 2.0f;
    float w = CARD_WIDTH * eff_scale;
    float h = CARD_HEIGHT * eff_scale;

    graphics_set_texture(tex_tarots[tex_planet_x][tex_planet_y], GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_rotated_quad(
        x, y, w, h,
        1 + (planet_type->u - tex_planet_x * 7) * (TEXTURE_CARD_WIDTH + 2),
        1 + (planet_type->v - tex_planet_y * 5) * (TEXTURE_CARD_HEIGHT + 2),
        TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, planet->draw.angle);

    if (planet->draw.white_factor > 0.0f)
    {
        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        uint32_t color = 0xFFFFFF | ((uint32_t)(255.0f * (planet->draw.white_factor > 1.0f ? 1.0f : planet->draw.white_factor))<<24);
        graphics_draw_quad(x, y, w, h, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, color);
    }
}

void game_draw_booster(struct BoosterPack *booster)
{
    int tex_booster_y = 0;
    struct BoosterPackType *booster_type = &(g_booster_types[booster->type][booster->size][booster->image]);
    tex_booster_y = booster_type->v / 5;

    float x, y;
    game_draw_get_oscillating_position(&(booster->draw), &x, &y);
    float bounce = game_draw_get_bounce_scale(&(booster->draw));
    float eff_scale = booster->draw.scale * bounce;
    float bw = BOOSTER_WIDTH * eff_scale;
    float bh = BOOSTER_HEIGHT * eff_scale;
    x -= (bw - BOOSTER_WIDTH) / 2.0f;
    y -= (bh - BOOSTER_HEIGHT) / 2.0f;

    graphics_set_texture(tex_boosters[tex_booster_y], GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_rotated_quad(
        x, y, bw, bh,
        1 + (booster_type->u) * (TEXTURE_CARD_WIDTH + 2),
        1 + (booster_type->v - tex_booster_y * 5) * (TEXTURE_CARD_HEIGHT + 2),
        TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE, booster->draw.angle);
    game_draw_price_tag(&(booster->draw), game_util_get_booster_price(booster));
}

void game_draw_shop_singles()
{
    for (int i = 0; i < g_game_state.shop.total_items; i++)
    {
        if (!g_game_state.shop.items[i].available) continue;

        if (g_game_state.shop.items[i].type == ITEM_TYPE_JOKER)
        {
            struct Joker *joker = &(g_game_state.shop.items[i].info.joker);
            game_draw_joker(joker);
            game_draw_price_tag(&joker->draw, game_util_get_joker_buy_price(joker));
        }
        else if (g_game_state.shop.items[i].type == ITEM_TYPE_PLANET)
        {
            struct Planet *planet = &(g_game_state.shop.items[i].info.planet);
            game_draw_planet(planet);
            game_draw_price_tag(&planet->draw, game_util_get_planet_buy_price(planet));
        }
        else if (g_game_state.shop.items[i].type == ITEM_TYPE_TAROT)
        {
            struct Tarot *tarot = &(g_game_state.shop.items[i].info.tarot);
            game_draw_tarot(tarot);
            game_draw_price_tag(&tarot->draw, game_util_get_tarot_buy_price(tarot));
        }
        else if (g_game_state.shop.items[i].type == ITEM_TYPE_SPECTRAL)
        {
            struct Spectral *spectral = &(g_game_state.shop.items[i].info.spectral);
            game_draw_spectral(spectral);
            game_draw_price_tag(&spectral->draw, game_util_get_spectral_buy_price(spectral));
        }
    }
}
void game_draw_shop_boosters()
{
    for (int i = 0; i < g_game_state.shop.total_boosters; i++)
    {
        if (!g_game_state.shop.boosters[i].available) continue;

        struct BoosterPack *booster = &(g_game_state.shop.boosters[i].booster);
        game_draw_booster(booster);
    }
}

void game_draw_shop_selected_single()
{
    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SHOP_SINGLES)
    {
        struct DrawObject *draw = NULL;
        switch (g_game_state.shop.items[g_game_state.highlighted_item].type)
        {
            case ITEM_TYPE_JOKER:
                game_draw_joker_hint(&(g_game_state.shop.items[g_game_state.highlighted_item].info.joker));
                draw = &g_game_state.shop.items[g_game_state.highlighted_item].info.joker.draw;
                break;
            case ITEM_TYPE_CARD:
                game_draw_card_hint(&(g_game_state.shop.items[g_game_state.highlighted_item].info.card));
                draw = &g_game_state.shop.items[g_game_state.highlighted_item].info.card.draw;
                break;
            case ITEM_TYPE_PLANET:
                game_draw_planet_hint(&(g_game_state.shop.items[g_game_state.highlighted_item].info.planet));
                draw = &g_game_state.shop.items[g_game_state.highlighted_item].info.planet.draw;
                break;
            case ITEM_TYPE_TAROT:
                game_draw_tarot_hint(&(g_game_state.shop.items[g_game_state.highlighted_item].info.tarot));
                draw = &g_game_state.shop.items[g_game_state.highlighted_item].info.tarot.draw;
                break;
            case ITEM_TYPE_SPECTRAL:
                game_draw_spectral_hint(&(g_game_state.shop.items[g_game_state.highlighted_item].info.spectral));
                draw = &g_game_state.shop.items[g_game_state.highlighted_item].info.spectral.draw;
                break;
            default:
                break;
        }

        game_draw_card_frame(draw);
        game_draw_item_button(draw, GAMEPAD_UI_RIGHT_TRIGGER, "Buy", NULL, COLOR_RED_BUTTON, false);
        if (g_game_state.shop.items[g_game_state.highlighted_item].type == ITEM_TYPE_PLANET ||
            g_game_state.shop.items[g_game_state.highlighted_item].type == ITEM_TYPE_TAROT ||
            g_game_state.shop.items[g_game_state.highlighted_item].type == ITEM_TYPE_SPECTRAL)
        {
            game_draw_item_button(draw, GAMEPAD_UI_LEFT_TRIGGER, "Buy", "&Use", COLOR_GREEN_BUTTON, true);
        }
    }
}

void game_draw_shop_selected_booster()
{
    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SHOP_BOOSTERS)
    {
        struct DrawObject *draw = &(g_game_state.shop.boosters[g_game_state.highlighted_item].booster.draw);

        game_draw_booster_frame(draw);
        game_draw_item_button(draw, GAMEPAD_UI_RIGHT_TRIGGER, "Open", NULL, COLOR_GREEN_BUTTON, false);
        game_draw_booster_hint(&(g_game_state.shop.boosters[g_game_state.highlighted_item].booster));
    }
}

void game_draw_consumables()
{
    graphics_draw_solid_quad(DRAW_CONSUMABLES_X, DRAW_CONSUMABLES_y, DRAW_CONSUMABLES_WIDTH, CARD_HEIGHT, COLOR_SHADOW);

    for (int i = 0; i < g_game_state.consumables.item_count; i++)
    {
        if (i == g_game_state.highlighted_item && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_CONSUMABLES) continue;
        if (g_game_state.consumables.items[i].type == ITEM_TYPE_PLANET) game_draw_planet(&(g_game_state.consumables.items[i].planet));
        if (g_game_state.consumables.items[i].type == ITEM_TYPE_TAROT) game_draw_tarot(&(g_game_state.consumables.items[i].tarot));
        if (g_game_state.consumables.items[i].type == ITEM_TYPE_SPECTRAL) game_draw_spectral(&(g_game_state.consumables.items[i].spectral));
    }

    char str[24];
    sprintf(str, "%d/%d", g_game_state.consumables.item_count, g_game_state.consumable_slots);
    graphics_draw_text(font_small, str, DRAW_CONSUMABLES_X + 2.0f, DRAW_CONSUMABLES_y + CARD_HEIGHT + 1.0f, 1.0f, COLOR_WHITE);

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_CONSUMABLES)
    {
        if (g_game_state.consumables.items[g_game_state.highlighted_item].type == ITEM_TYPE_PLANET)
        {
            struct Planet *selected_planet = &(g_game_state.consumables.items[g_game_state.highlighted_item].planet);
            game_draw_card_frame(&selected_planet->draw);
            game_draw_planet(selected_planet);
            sprintf(str, "$%d", game_util_get_planet_sell_price(selected_planet));
            game_draw_item_button(&selected_planet->draw, GAMEPAD_UI_LEFT_TRIGGER, "Sell", str, COLOR_GREEN_BUTTON, true);
            game_draw_item_button(&selected_planet->draw, GAMEPAD_UI_RIGHT_TRIGGER, "Use", NULL, COLOR_RED_BUTTON, false);
            game_draw_planet_hint(selected_planet);
        }
        else if (g_game_state.consumables.items[g_game_state.highlighted_item].type == ITEM_TYPE_TAROT)
        {
            struct Tarot *selected_tarot = &(g_game_state.consumables.items[g_game_state.highlighted_item].tarot);
            game_draw_card_frame(&selected_tarot->draw);
            game_draw_tarot(selected_tarot);
            sprintf(str, "$%d", game_util_get_tarot_sell_price(selected_tarot));
            game_draw_item_button(&selected_tarot->draw, GAMEPAD_UI_LEFT_TRIGGER, "Sell", str, COLOR_GREEN_BUTTON, true);
            game_draw_item_button(&selected_tarot->draw, GAMEPAD_UI_RIGHT_TRIGGER, "Use", NULL, COLOR_RED_BUTTON, false);
            game_draw_tarot_hint(selected_tarot);
        }
        else if (g_game_state.consumables.items[g_game_state.highlighted_item].type == ITEM_TYPE_SPECTRAL)
        {
            struct Spectral *selected_spectral = &(g_game_state.consumables.items[g_game_state.highlighted_item].spectral);
            game_draw_card_frame(&selected_spectral->draw);
            game_draw_spectral(selected_spectral);
            sprintf(str, "$%d", game_util_get_spectral_sell_price(selected_spectral));
            game_draw_item_button(&selected_spectral->draw, GAMEPAD_UI_LEFT_TRIGGER, "Sell", str, COLOR_GREEN_BUTTON, true);
            game_draw_item_button(&selected_spectral->draw, GAMEPAD_UI_RIGHT_TRIGGER, "Use", NULL, COLOR_RED_BUTTON, false);
            game_draw_spectral_hint(selected_spectral);
        }
    }
}

void game_draw_top_jokers()
{
    graphics_draw_solid_quad(DRAW_JOKERS_X, DRAW_JOKERS_Y, DRAW_JOKERS_WIDTH, CARD_HEIGHT, COLOR_SHADOW);

    for (int i = 0; i < g_game_state.jokers.joker_count; i++)
    {
        if (i == g_game_state.highlighted_item && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_JOKERS) continue;
        game_draw_joker(&g_game_state.jokers.jokers[i]);
    }

    char str[24];
    sprintf(str, "%d/%d", g_game_state.jokers.joker_count, g_game_state.joker_slots + g_game_state.jokers.negative_count);
    graphics_draw_text(font_small, str, DRAW_JOKERS_X + 2.0f, DRAW_JOKERS_Y + CARD_HEIGHT + 1.0f, 1.0f, COLOR_WHITE);

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_JOKERS)
    {
        struct Joker *selected_joker = &g_game_state.jokers.jokers[g_game_state.highlighted_item];
        game_draw_card_frame(&selected_joker->draw);
        game_draw_joker(selected_joker);
        sprintf(str, "$%d", game_util_get_joker_sell_price(selected_joker));
        game_draw_item_button(&selected_joker->draw, GAMEPAD_UI_LEFT_TRIGGER, "Sell", str, COLOR_GREEN_BUTTON, true);
        game_draw_joker_hint(&(g_game_state.jokers.jokers[g_game_state.highlighted_item]));
    }
}

void game_draw_ingame_hand()
{
    graphics_set_no_texture();
    graphics_draw_quad(DRAW_HAND_X, game_get_hand_y(), DRAW_HAND_WIDTH, CARD_HEIGHT, 0, 0, 0, 0, COLOR_SHADOW);
    // Leque centrado na mão (cardtest: ROT_MAX em torno do centro do leque).
    g_tilt_center_x = DRAW_HAND_X + DRAW_HAND_WIDTH / 2.0f;

    // Foco: impulso de tilt (cardtest) + juice de hover do Balatro.
    // A escala de hover (1.05) e o juice são integrados na mola em
    // game_smooth_hand_cards(); aqui só disparamos o feedback.
    static int prev_focus_idx = -1;
    int focus_idx = -1;
    if (g_game_state.show_highlighted_card && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_HAND &&
        g_game_state.highlighted_item > -1 && g_game_state.highlighted_item < g_game_state.hand.card_count)
    {
        focus_idx = g_game_state.highlighted_item;
    }
    if (focus_idx != prev_focus_idx)
    {
        if (focus_idx >= 0 && focus_idx < g_game_state.hand.card_count)
        {
            struct Card *focused = g_game_state.hand.cards[focus_idx];
            focused->draw.tilt_wv += 4.3f;
            game_card_juice_up(focused, 0.05f, 0.03f);
        }
        prev_focus_idx = focus_idx;
    }

    // Pass 1 — sombras (cardtest): todas no fundo, nenhum corpo recebe
    // sombra de outra carta por cima. Nao-selecionadas, depois selecionadas.
    for (int i = 0; i < g_game_state.hand.card_count; i++)
    {
        if (g_game_state.hand.cards[i]->selected) continue;
        game_draw_card_shadow(g_game_state.hand.cards[i], NULL);
    }
    for (int i = 0; i < g_game_state.hand.card_count; i++)
    {
        if (!g_game_state.hand.cards[i]->selected) continue;
        game_draw_card_shadow(g_game_state.hand.cards[i], NULL);
    }
    // Pass 2 — corpos (cardtest: marcadas desenham por cima das outras).
    for (int i = 0; i < g_game_state.hand.card_count; i++)
    {
        if (g_game_state.hand.cards[i]->selected) continue;
        game_draw_card_body(g_game_state.hand.cards[i], NULL);
    }
    for (int i = 0; i < g_game_state.hand.card_count; i++)
    {
        if (!g_game_state.hand.cards[i]->selected) continue;
        game_draw_card_body(g_game_state.hand.cards[i], NULL);
    }

    if (g_game_state.show_highlighted_card && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_HAND)
    {
        if (g_game_state.highlighted_item > -1 && g_game_state.highlighted_item < g_game_state.hand.card_count)
        {
            game_draw_card_hint(g_game_state.hand.cards[g_game_state.highlighted_item]);
        }
    }

    // Sombra em pass separado tambem nas jogadas (cardtest).
    // No mat: retas (sem leque), com pivo de arrasto.
    g_mat_straight = true;
    for (int i = 0; i < g_game_state.played_hand.card_count; i++)
    {
        game_draw_card_shadow(g_game_state.played_hand.cards[i], NULL);
    }
    for (int i = 0; i < g_game_state.played_hand.card_count; i++)
    {
        game_draw_card_body(g_game_state.played_hand.cards[i], NULL);
    }
    g_mat_straight = false;

    g_tilt_center_x = SCREEN_WIDTH / 2.0f;

    // Draw card count
    char str[24];
    sprintf(str, "%d/%d", g_game_state.hand.card_count, game_util_get_hand_size());
    graphics_draw_text(font_small, str, DRAW_HAND_X + 2.0f, game_get_hand_y() + CARD_HEIGHT + 1.0f, 1.0f, COLOR_WHITE);

    // Draw other buttons
    if (game_get_hand_y() == DRAW_HAND_Y)
    {
        graphics_set_no_texture();
        float x = DRAW_HAND_X + (DRAW_HAND_WIDTH / 2.0f) - 40.0f;
        float y = DRAW_HAND_Y + CARD_HEIGHT;
        graphics_draw_quad(x, y, 80.0f, 30.0f, 0, 0, 0, 0, COLOR_DARK_GREY);
        graphics_draw_text_center(font_small, "Sort Hand", x + 40.0f, y + 7.0f, 1.0f, COLOR_WHITE);
        graphics_set_no_texture();
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SORT_HAND_RANK)
        {
            game_draw_rounded_box(x + 1.0f, y + 13.0f, 37.0f, 16.0f, 4, COLOR_WHITE, COLOR_WHITE, 1);
        }
        game_draw_rounded_box(x + 3.0f, y + 14.0f, 34.0f, 14.0f, 3, g_game_state.sort_criteria == SORT_CRITERIA_RANK ? COLOR_TEXT_ORANGE : 0xFF0097FC, g_game_state.sort_criteria == SORT_CRITERIA_RANK ? COLOR_TEXT_ORANGE : 0xFF0097FC, 1);
        graphics_draw_text_center(font_small, "Rank", x + 20.0f, y + 20.0f, 1.0f, COLOR_WHITE);
        graphics_set_no_texture();
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SORT_HAND_SUIT)
        {
            game_draw_rounded_box(x + 41.0f, y + 13.0f, 37.0f, 16.0f, 4, COLOR_WHITE, COLOR_WHITE, 1);
        }
        game_draw_rounded_box(x + 43.0f, y + 14.0f, 34.0f, 14.0f, 3, g_game_state.sort_criteria == SORT_CRITERIA_SUIT ? COLOR_TEXT_ORANGE : 0xFF0097FC, g_game_state.sort_criteria == SORT_CRITERIA_SUIT ? COLOR_TEXT_ORANGE : 0xFF0097FC, 1);
        graphics_draw_text_center(font_small, "Suit", x + 62.0f, y + 20.0f, 1.0f, COLOR_WHITE);

        // Play Hand
        x = DRAW_HAND_X + (DRAW_HAND_WIDTH / 2.0f) - 120.0f;
        y = DRAW_HAND_Y + CARD_HEIGHT + 2.0f;
        bool can_play = game_util_can_play_selected_hand();
        graphics_set_no_texture();
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_HAND_PLAY)
        {
            game_draw_rounded_box(x - 2.0f, y - 2.0f, 82.0f, 30.0f, 5, COLOR_WHITE, COLOR_WHITE, 1);
        }
        game_draw_rounded_box(x, y, 78.0f, 26.0f, 4, can_play ? COLOR_LIGHT_BLUE : COLOR_DARK_GREY, can_play ? COLOR_LIGHT_BLUE : COLOR_DARK_GREY, 1);
        graphics_draw_text_center(font_small, "Play Hand", x + 48.0f, y + 12.0f, 1.0f, can_play ? COLOR_WHITE : COLOR_LIGHT_GREY_2);
        graphics_set_texture(tex_gamepad_ui, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_quad(x + 2.0f, y + 4.0f, 16.0f, 16.0f,
            g_gamepad_ui_text_coords[GAMEPAD_UI_SQUARE].x,
            g_gamepad_ui_text_coords[GAMEPAD_UI_SQUARE].y,
            32, 32, can_play ? COLOR_WHITE : COLOR_LIGHT_GREY_2);

        // Discard
        x = DRAW_HAND_X + (DRAW_HAND_WIDTH / 2.0f) + 42.0f;
        bool can_discard = g_game_state.selected_cards_count > 0 && g_game_state.current_discards > 0;
        graphics_set_no_texture();
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_HAND_DISCARD)
        {
            game_draw_rounded_box(x - 2.0f, y - 2.0f, 82.0f, 30.0f, 5, COLOR_WHITE, COLOR_WHITE, 1);
        }
        game_draw_rounded_box(x, y, 78.0f, 26.0f, 4, can_discard ? COLOR_LIGHT_RED : COLOR_DARK_GREY, can_discard ? COLOR_LIGHT_RED : COLOR_DARK_GREY, 1);
        graphics_draw_text_center(font_small, "Discard", x + 30.0f, y + 12.0f, 1.0f, can_discard ? COLOR_WHITE : COLOR_LIGHT_GREY_2);
        graphics_set_texture(tex_gamepad_ui, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_quad(x + 60.0f, y + 4.0f, 16.0f, 16.0f,
            g_gamepad_ui_text_coords[GAMEPAD_UI_TRIANGLE].x,
            g_gamepad_ui_text_coords[GAMEPAD_UI_TRIANGLE].y,
            32, 32, can_discard ? COLOR_WHITE : COLOR_LIGHT_GREY_2);
    }
}

void game_draw_booster_hand()
{
    graphics_set_no_texture();
    graphics_draw_quad(DRAW_HAND_X, DRAW_HAND_Y_IN_BOOSTER, DRAW_HAND_WIDTH, CARD_HEIGHT, 0, 0, 0, 0, COLOR_SHADOW);
    g_tilt_center_x = DRAW_HAND_X + DRAW_HAND_WIDTH / 2.0f;

    // Sombra em pass separado (cardtest).
    for (int i = 0; i < g_game_state.hand.card_count; i++)
    {
        game_draw_card_shadow(g_game_state.hand.cards[i], NULL);
    }
    for (int i = 0; i < g_game_state.hand.card_count; i++)
    {
        game_draw_card_body(g_game_state.hand.cards[i], NULL);
    }
    g_tilt_center_x = SCREEN_WIDTH / 2.0f;
}

void game_draw_booster_hand_selected_card()
{
    if (g_game_state.show_highlighted_card && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_HAND)
    {
        if (g_game_state.highlighted_item > -1 && g_game_state.highlighted_item < g_game_state.hand.card_count)
        {
            game_draw_card_hint(g_game_state.hand.cards[g_game_state.highlighted_item]);
        }
    }
}

void game_draw_deck()
{
    char str[24];

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_DECK)
    {
        struct DrawObject draw;
        draw.x = DRAW_DECK_X;
        draw.y = DRAW_DECK_Y;
        game_draw_card_frame(&(draw));
    }

    // Pilha do deck estilo cardtest: 2 lombadas atras + carta do topo.
    graphics_set_no_texture();
    graphics_draw_quad(DRAW_DECK_X + 4, DRAW_DECK_Y + 4, CARD_WIDTH, CARD_HEIGHT, 0, 0, 0, 0, COLOR_DARK_GREY_2);
    graphics_draw_quad(DRAW_DECK_X + 2, DRAW_DECK_Y + 2, CARD_WIDTH, CARD_HEIGHT, 0, 0, 0, 0, 0xFF5A5A5A);
    graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_quad(DRAW_DECK_X, DRAW_DECK_Y, CARD_WIDTH, CARD_HEIGHT, 0, 0, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, COLOR_WHITE);
    // Deck size
    sprintf(str, "%d/%d", g_game_state.current_deck.card_count, g_game_state.full_deck.card_count);
    graphics_draw_text_center(font_small, str, DRAW_DECK_X + (CARD_WIDTH / 2.0f), DRAW_DECK_Y + CARD_HEIGHT + 8.0f, 1.0f, COLOR_WHITE);
}

#define DRAW_LEFT_INFO_WIDTH 117

static void game_draw_format_score_value(char *str, size_t size, double value)
{
    if (value < 1000000.0)
    {
        snprintf(str, size, "%.0f", value);
    }
    else
    {
        snprintf(str, size, "%.4g", value);
    }
}

static float game_draw_get_font_char_width(int font)
{
    return font == font_big ? 8.0f : 6.0f;
}

static void game_draw_text_fit(int font, const char *text, float x, float y, float max_width, float size, uint32_t color)
{
    float width = strlen(text) * game_draw_get_font_char_width(font) * size;
    float scale = size;
    if (width > max_width && width > 0.0f)
    {
        scale = MAX(0.60f, max_width / (strlen(text) * game_draw_get_font_char_width(font)));
    }
    graphics_draw_text(font, text, x, y, scale, color);
}

static void game_draw_text_fit_min(int font, const char *text, float x, float y, float max_width, float size, float min_scale, uint32_t color)
{
    float width = strlen(text) * game_draw_get_font_char_width(font) * size;
    float scale = size;
    if (width > max_width && width > 0.0f)
    {
        scale = MAX(min_scale, max_width / (strlen(text) * game_draw_get_font_char_width(font)));
    }
    graphics_draw_text(font, text, x, y, scale, color);
}

static void game_draw_text_fit_center(int font, const char *text, float x, float y, float max_width, float size, uint32_t color)
{
    float width = strlen(text) * game_draw_get_font_char_width(font) * size;
    float scale = size;
    if (width > max_width && width > 0.0f)
    {
        scale = MAX(0.60f, max_width / (strlen(text) * game_draw_get_font_char_width(font)));
    }
    graphics_draw_text_center(font, text, x, y, scale, color);
}

static void game_draw_text_fit_center_min(int font, const char *text, float x, float y, float max_width, float size, float min_scale, uint32_t color)
{
    float width = strlen(text) * game_draw_get_font_char_width(font) * size;
    float scale = size;
    if (width > max_width && width > 0.0f)
    {
        scale = MAX(min_scale, max_width / (strlen(text) * game_draw_get_font_char_width(font)));
    }
    graphics_draw_text_center(font, text, x, y, scale, color);
}

bool game_draw_should_burn_score()
{
    if (g_debug_info.force_score_flames && g_game_state.stage == GAME_STAGE_INGAME)
    {
        return true;
    }

    if (g_game_state.stage != GAME_STAGE_INGAME ||
        g_game_state.sub_stage != GAME_SUBSTAGE_INGAME_PICK_HAND ||
        g_game_state.current_poker_hand == GAME_POKER_HAND_NONE)
    {
        return false;
    }

    double hand_score = g_game_state.current_base_chips * g_game_state.current_base_mult;
    return hand_score > 0.0 && hand_score >= game_get_current_blind_score();
}

float game_draw_get_score_flame_intensity()
{
    if (!game_draw_should_burn_score()) return 0.0f;

    if (g_debug_info.force_score_flames)
    {
        return 1.5f + (float)((g_time / 16) % 4);
    }

    double hand_score = g_game_state.current_base_chips * g_game_state.current_base_mult;
    float intensity = (float)(log(hand_score) / log(5.0) - 2.0);
    return CLAMP(intensity, 0.35f, 5.5f);
}

void game_draw_score_flames(float x, float y, float w, float h, uint32_t colour_1, uint32_t colour_2, float id)
{
    float intensity = game_draw_get_score_flame_intensity();
    if (intensity <= 0.0f) return;

    // Corpo translucido sobre a caixa (composicao original)
    graphics_set_no_texture();
    float body_alpha = CLAMP(0.10f + intensity * 0.025f, 0.14f, 0.28f);
    uint32_t body_colour = (colour_1 & 0x00FFFFFF) | ((uint32_t)(body_alpha * 255.0f) << 24);
    graphics_draw_solid_quad(x, y - 2.0f, w, h + 2.0f, body_colour);

    // Chamas procedurais (shader do Balatro) subindo acima da caixa
    bg_draw_flames(x, y, w, h, colour_1, colour_2, intensity, id);
}

void game_draw_round_bars(float x, float y, float w, float h, float r, uint32_t color)
{
    graphics_set_no_texture();
    graphics_draw_quad(x + r, y, w - 2.0f * r, r, 0, 0, 0, 0, color);
    graphics_draw_quad(x + r, y + h - r, w - 2.0f * r, r, 0, 0, 0, 0, color);
    graphics_draw_quad(x, y + r, r, h - 2.0f * r, 0, 0, 0, 0, color);
    graphics_draw_quad(x + w - r, y + r, r, h - 2.0f * r, 0, 0, 0, 0, color);
    graphics_draw_quad(x + r, y + r, w - 2.0f * r, h - 2.0f * r, 0, 0, 0, 0, color);
}

void game_draw_round_corners(float x, float y, float w, float h, float r, uint32_t color)
{
    if (tex_round_corners < 0) return;
    graphics_set_texture(tex_round_corners, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_quad(x, y, r, r, 0, 0, 8, 8, color);
    graphics_draw_quad(x + w - r, y, r, r, 8, 0, 8, 8, color);
    graphics_draw_quad(x, y + h - r, r, r, 0, 8, 8, 8, color);
    graphics_draw_quad(x + w - r, y + h - r, r, r, 8, 8, 8, 8, color);
    graphics_set_no_texture();
}

// Caixa com cantos arredondados: camada da borda em tamanho cheio + preenchimento recuado.
// Para solido arredondado sem borda visivel, passe border = fill.
void game_draw_rounded_box(float x, float y, float w, float h, float r, uint32_t fill, uint32_t border, float t)
{
    if (g_panel_shadow)
    {
        g_panel_shadow = false;
        // Sombra colada e translúcida (estilo cardtest): 1px, ~40% preto.
        game_draw_rounded_box(x + 1, y + 1, w, h, r, 0x66000000, 0x66000000, t);
        g_panel_shadow = true;
    }
    float rf = r - t;
    if (rf < 1.0f) rf = 1.0f;
    game_draw_round_bars(x, y, w, h, r, border);
    game_draw_round_corners(x, y, w, h, r, border);
    game_draw_round_bars(x + t, y + t, w - 2.0f * t, h - 2.0f * t, rf, fill);
    game_draw_round_corners(x + t, y + t, w - 2.0f * t, h - 2.0f * t, rf, fill);
}

uint32_t game_draw_stage_border_color(int draw_stage)
{
    switch (draw_stage)
    {
        case GAME_STAGE_BLINDS: return 0xFF9C557B; // roxo
        case GAME_STAGE_INGAME: return COLOR_TEXT_ORANGE; // laranja/dourado
        case GAME_STAGE_SHOP: return COLOR_TEXT_GREEN; // verde
        case GAME_STAGE_PAUSE_MENU: return COLOR_LIGHT_RED; // vermelho
        default: return COLOR_LIGHT_GREY; // titulo e demais: cinza
    }
}

void game_draw_left_info()
{
    static int shop_anim = 0;
    char str[24];
    g_panel_shadow = true;
    float score_flame_intensity = game_draw_get_score_flame_intensity();
    audio_set_score_flame_intensity(score_flame_intensity);

    // Fundo chapado ponta a ponta, sem cantos arredondados
    graphics_set_no_texture();
    graphics_draw_solid_quad(0, 0, DRAW_LEFT_INFO_WIDTH, SCREEN_HEIGHT, 0xFF373729);

    int y = 8.0f;

    switch (g_game_state.stage)
    {
        case GAME_STAGE_BLINDS:
        {
            graphics_draw_text(font_big, "Choose your", 6, y, 1.0f, COLOR_WHITE);
            y += 16;
            graphics_draw_text(font_big, "next Blind", 6, y, 1.0f, COLOR_WHITE);
            y += 28;
            break;
        }
        case GAME_STAGE_INGAME:
        {
            int blind_row = 0;
            if (g_game_state.blind == GAME_BLIND_SMALL) blind_row = 0;
            else if (g_game_state.blind == GAME_BLIND_LARGE) blind_row = 1;
            else if (g_game_state.blind == GAME_BLIND_BOSS) {
                if (g_game_state.boss_blind_type >= 0 && g_game_state.boss_blind_type < BOSS_BLIND_COUNT) {
                    blind_row = g_boss_blind_tex_rows[g_game_state.boss_blind_type];
                } else {
                    blind_row = g_boss_blind_tex_rows[0];
                }
            }

            uint32_t blind_banner = COLOR_LIGHT_BLUE;
            if (g_game_state.blind == GAME_BLIND_LARGE) blind_banner = 0xFF005396;
            else if (g_game_state.blind == GAME_BLIND_BOSS) blind_banner = COLOR_LIGHT_RED;
            graphics_set_no_texture();
            game_draw_rounded_box(6, 6, 105, 24, 5, blind_banner, blind_banner, 1);
            switch (g_game_state.blind)
            {
                case GAME_BLIND_SMALL:
                case GAME_BLIND_LARGE:
                case GAME_BLIND_BOSS:
                    game_draw_text_fit_center_min(font_big, game_util_get_blind_name(g_game_state.blind), 58.5f, 18, 97.0f, 1.1f, 0.85f, COLOR_WHITE);
                    break;
            }
            y += 28;

            // Caixa principal na cor do blind levemente dessaturada, sem borda
            uint32_t blind_box_fill = desaturate_color(blind_banner, 0.25f);
            game_draw_rounded_box(6, 32, 105, 66, 5, blind_box_fill, blind_box_fill, 1);
            int frame = (g_game_counter / 4) % 21;
            graphics_set_texture(tex_blind_chips[frame/15][blind_row/15], GRAPHICS_TEXTURE_FILTER_NEAREST);
            graphics_draw_quad(10.0f, 50.0f, 32, 32, (frame % 15) * 34, (blind_row % 15) * 34, 34, 34, 0x77000000);
            graphics_draw_quad(9.0f, 49.0f, 32, 32, (frame % 15) * 34, (blind_row % 15) * 34, 34, 34, COLOR_WHITE);
            graphics_set_no_texture();

            game_draw_rounded_box(44, 40, 65, 54, 4, 0xFF141414, 0xFF141414, 1);
            {
                game_draw_text_fit_center(font_small, "SCORE MIN", 76.5f, 48.5f, 61.0f, 0.9f, COLOR_WHITE);
                game_draw_format_score_value(str, sizeof(str), game_get_current_blind_score());
                float w_str = (float)strlen(str) * game_draw_get_font_char_width(font_small) * 1.3f;
                float x0 = 76.5f - (10.0f + 3.0f + w_str) / 2.0f;
                graphics_set_texture(tex_ui_assets, GRAPHICS_TEXTURE_FILTER_NEAREST);
                graphics_draw_quad(x0 + 1, 58.0f, 10, 10, g_ui_assets_tex_coords[UI_ASSETS_CHIP].x, g_ui_assets_tex_coords[UI_ASSETS_CHIP].y, 18, 18, 0x77000000);
                graphics_draw_quad(x0, 57.0f, 10, 10, g_ui_assets_tex_coords[UI_ASSETS_CHIP].x, g_ui_assets_tex_coords[UI_ASSETS_CHIP].y, 18, 18, COLOR_WHITE);
                graphics_set_no_texture();
                game_draw_text_fit(font_small, str, x0 + 13, 56.0f, 40.0f, 1.3f, COLOR_LIGHT_RED);
                game_draw_text_fit_center(font_small, "REWARD", 76.5f, 72.5f, 57.0f, 0.88f, COLOR_LIGHT_GREY_2);
                int rew = game_draw_get_blind_reward(g_game_state.blind);
                if (rew < 0) rew = 0;
                if (rew > 8) rew = 8;
                for (int i = 0; i < rew; i++) str[i] = '$';
                str[rew] = '\0';
                game_draw_text_fit_center(font_small, str, 76.5f, 81.5f, 57.0f, 1.3f, COLOR_YELLOW);
            }
            y += 66;

            break;
        }
        case GAME_STAGE_SHOP:
        {
            if (g_game_counter % 10 == 0) shop_anim++;
            if (shop_anim > 3) shop_anim = 0;

            graphics_set_texture(tex_shop, GRAPHICS_TEXTURE_FILTER_LINEAR);
            graphics_draw_quad(4.0f, y - 8.0f, DRAW_LEFT_INFO_WIDTH - 4.0f, 50.0f, 113 * shop_anim, 0, 113, 57, COLOR_WHITE);
            y += 44;
            break;
        }
        default:
        {
            y += 44;
            break;
        }
    }

    game_draw_rounded_box(6, y, 105, 26, 5, 0xFF272518, 0xFF272518, 1);

    game_draw_text_fit_center(font_small, "ROUND", 31, y + 9, 34.0f, 0.88f, COLOR_WHITE);
    game_draw_text_fit_center(font_small, "SCORE", 31, y + 17, 34.0f, 0.88f, COLOR_WHITE);

    game_draw_rounded_box(56, y + 2, 51, 22, 3, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);
    sprintf(str, "%g", g_game_state.score);
    {
        float w_str = (float)strlen(str) * game_draw_get_font_char_width(font_big) * 1.35f;
        float x0 = 81.5f - (10.0f + 3.0f + w_str) / 2.0f;
        graphics_set_texture(tex_ui_assets, GRAPHICS_TEXTURE_FILTER_NEAREST);
        graphics_draw_quad(x0 + 1, y + 9, 9, 9, g_ui_assets_tex_coords[UI_ASSETS_CHIP].x, g_ui_assets_tex_coords[UI_ASSETS_CHIP].y, 18, 18, 0x77000000);
        graphics_draw_quad(x0, y + 8, 9, 9, g_ui_assets_tex_coords[UI_ASSETS_CHIP].x, g_ui_assets_tex_coords[UI_ASSETS_CHIP].y, 18, 18, COLOR_WHITE);
        graphics_set_no_texture();
        game_draw_text_fit(font_big, str, x0 + 12, y + 7, 34.0f, 1.35f, COLOR_WHITE);
    }

    y += 30;

    game_draw_rounded_box(6, y, 105, 48, 5, 0xFF272518, 0xFF272518, 1);
    // Draw poker hand
    game_draw_text_fit_center(font_small, g_poker_hand_names[g_game_state.current_poker_hand], 52, y + 10, 62.0f, 1.23f, COLOR_WHITE);
    // Draw poker hand level
    if (g_game_state.current_poker_hand != GAME_POKER_HAND_NONE)
    {
        sprintf(str, "lvl.%d", g_game_state.poker_hand_level[g_game_state.current_poker_hand]);
        game_draw_text_fit(font_small, str, 86, y + 7, 26.0f, 0.75f, COLOR_WHITE);
    }

    graphics_set_no_texture();
    if (score_flame_intensity > 0.0f)
    {
        int chip_dx = ((g_time * 3) % 5) - 2;
        int chip_dy = ((g_time * 5) % 5) - 2;
        int mult_dx = ((g_time * 7) % 5) - 2;
        int mult_dy = ((g_time * 11) % 5) - 2;
        game_draw_score_flames(11.0f, (float)y + 21.0f, 46.0f, 23.0f, 0xDDEF8700, 0xFF49B3EF, 1.0f);
        game_draw_score_flames(74.0f, (float)y + 21.0f, 32.0f, 23.0f, 0xDD4040FF, 0xFF008FFF, 2.0f);
        game_draw_rounded_box(11 + chip_dx, y + 21 + chip_dy, 46, 23, 4, COLOR_LIGHT_BLUE, COLOR_LIGHT_BLUE, 1);
        game_draw_rounded_box(74 + mult_dx, y + 21 + mult_dy, 32, 23, 4, COLOR_LIGHT_RED, COLOR_LIGHT_RED, 1);
    }
    else
    {
        game_draw_rounded_box(11, y + 21, 46, 23, 4, COLOR_LIGHT_BLUE, COLOR_LIGHT_BLUE, 1);
        game_draw_rounded_box(74, y + 21, 32, 23, 4, COLOR_LIGHT_RED, COLOR_LIGHT_RED, 1);
    }

    game_draw_format_score_value(str, sizeof(str), g_game_state.current_base_chips);
    {
        float chips_w = (float)strlen(str) * game_draw_get_font_char_width(font_small) * 1.4f;
        graphics_draw_text_center(font_small, str, 11.0f + 46.0f - 4.0f - chips_w / 2.0f, y + 32.0f, 1.4f, COLOR_WHITE);
    }
    game_draw_text_fit_center(font_small, "X", 63.5f, y + 32.0f, 12.0f, 1.4f, COLOR_LIGHT_RED);
    game_draw_format_score_value(str, sizeof(str), g_game_state.current_base_mult);
    {
        float mult_w = (float)strlen(str) * game_draw_get_font_char_width(font_small) * 1.4f;
        graphics_draw_text_center(font_small, str, 74.0f + 4.0f + mult_w / 2.0f, y + 32.0f, 1.4f, COLOR_WHITE);
    }

    y += 52;
    // Linha Hands / Discards em largura total (Run Info agora e no SELECT)
    game_draw_rounded_box(6, y, 51, 34, 4, 0xFF272518, 0xFF272518, 1);

    game_draw_text_fit_center(font_small, "HANDS", 31.5f, y + 5, 45.0f, 0.88f, COLOR_WHITE);
    game_draw_rounded_box(10, y + 11, 43, 18, 2, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);

    sprintf(str, "%d", g_game_state.current_hands);
    uint32_t hands_color = COLOR_TEXT_BLUE;
    if (g_game_state.current_hands <= 0) hands_color = COLOR_LIGHT_GREY_2;
    else if (g_game_state.current_hands == 1) hands_color = COLOR_TEXT_ORANGE;
    game_draw_text_fit_center(font_small, str, 31.5f, y + 19, 39.0f, 1.4f, hands_color);
    sprintf(str, "%d", g_game_state.current_discards);
    uint32_t discards_color = COLOR_LIGHT_RED;
    if (g_game_state.current_discards <= 0) discards_color = COLOR_LIGHT_GREY_2;
    else if (g_game_state.current_discards == 1) discards_color = COLOR_TEXT_ORANGE;
    game_draw_rounded_box(60, y, 51, 34, 4, 0xFF272518, 0xFF272518, 1);

    game_draw_text_fit_center(font_small, "DISCARDS", 85.5f, y + 5, 45.0f, 0.88f, COLOR_WHITE);
    game_draw_rounded_box(64, y + 11, 43, 18, 2, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);
    game_draw_text_fit_center(font_small, str, 85.5f, y + 19, 39.0f, 1.4f, discards_color);

    y += 36;
    game_draw_rounded_box(6, y, 105, 16, 4, 0xFF272518, 0xFF272518, 1);
    game_draw_rounded_box(10, y + 1, 97, 14, 2, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);

    sprintf(str, "$%d", g_game_state.wealth);
    game_draw_text_fit_center(font_big, str, 58.5f, y + 8, 90.0f, 1.35f, COLOR_TEXT_YELLOW);

    // Linha Ante / Round em largura total (Options agora e no START)
    game_draw_rounded_box(6, y + 18, 51, 32, 3, 0xFF272518, 0xFF272518, 1);

    game_draw_text_fit_center(font_small, "ANTE", 31.5f, y + 24, 45.0f, 0.88f, COLOR_WHITE);
    game_draw_rounded_box(9, y + 30, 45, 18, 2, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);

    sprintf(str, "#5%d#-/8", g_game_state.ante);
    graphics_draw_text_formatted_center(font_small, str, NULL, 31.5f, y + 38, 1.35f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.round);
    game_draw_rounded_box(60, y + 18, 51, 32, 3, 0xFF272518, 0xFF272518, 1);

    game_draw_text_fit_center(font_small, "ROUND", 85.5f, y + 24, 45.0f, 0.88f, COLOR_WHITE);
    game_draw_rounded_box(63, y + 30, 45, 18, 2, COLOR_DARK_GREY, COLOR_DARK_GREY, 1);
    game_draw_text_fit_center(font_small, str, 85.5f, y + 38, 41.0f, 1.35f, COLOR_TEXT_ORANGE);
    g_panel_shadow = false;
}

void game_draw_cash_out_panel()
{
    char str[32];

    int x = 140;
    float y = g_game_state.cash_out_panel_y;

    graphics_set_no_texture();
    graphics_draw_quad(x, y, 250, SCREEN_HEIGHT - 100, 0, 0, 0, 0, COLOR_DARK_GREY);
    y += 16;
    if (g_game_state.cash_out_done)
    {
        sprintf(str, "Cash Out: $%d", g_game_state.cash_out_value);
        float str_size = (float)strlen(str) * 8.0f;
        graphics_draw_solid_quad((140.0f + 125.0f) - (str_size / 2.0f) - 6.0f, y - 10.0f, str_size + 12.0f, 20.0f, COLOR_WHITE);
        graphics_draw_solid_quad((140.0f + 125.0f) - (str_size / 2.0f) - 4.0f, y - 8.0f, str_size + 8.0f, 16.0f, COLOR_TEXT_ORANGE);
        graphics_draw_text_center(font_big, str, (140.0f + 125.0f), y, 1.0f, COLOR_WHITE);
    }
    y += 20;
    if (g_game_state.cash_out_blind > -1)
    {
        sprintf(str, "$%d", g_game_state.cash_out_blind);
        graphics_draw_text(font_small, game_util_get_blind_name(g_game_state.blind), x + 4, y, 1.0f, COLOR_WHITE);
        graphics_draw_text(font_small, str, x + 200, y, 1.0f, COLOR_SCORE_NUMBER_TEXT_MONEY);
	y += BLIND_CHIP_HEIGHT;
	graphics_draw_text(font_small, "........................................", x + 4, y, 1.0f, COLOR_WHITE);
	y += 16;
    }
    if (g_game_state.cash_out_hands > 0)
    {
        sprintf(str, "%d", g_game_state.cash_out_hands);
		graphics_draw_text(font_big, str, x+4, y, 1.0f, COLOR_TEXT_BLUE);
        graphics_draw_text(font_small, "Remaining hands ($1 each)", x + 4 + 16, y, 1.0f, COLOR_WHITE);
        sprintf(str, "$%d", g_game_state.cash_out_hands);
        graphics_draw_text(font_small, str, x + 200, y, 1.0f, COLOR_SCORE_NUMBER_TEXT_MONEY);
        y += 16;
    }
    if (g_game_state.cash_out_interest > 0)
    {
        sprintf(str, "%d", g_game_state.cash_out_interest);
	graphics_draw_text(font_big, str, x + 4, y, 1.0f, COLOR_SCORE_NUMBER_TEXT_MONEY);
        graphics_draw_text(font_small, g_game_state.deck_type == DECK_TYPE_GREEN ? "Discards" : "Interest", x + 4 + 16, y, 1.0f, COLOR_WHITE);
        sprintf(str, "$%d", g_game_state.cash_out_interest);
        graphics_draw_text(font_small, str, x + 200, y, 1.0f, COLOR_SCORE_NUMBER_TEXT_MONEY);
        y += 16;
    }
    if (g_game_state.cash_out_jokers > 0)
    {
        sprintf(str, "%d", g_game_state.cash_out_jokers);
        graphics_draw_text(font_small, "Jokers", x + 4, y, 1.0f, COLOR_WHITE);
        sprintf(str, "$%d", g_game_state.cash_out_jokers);
        graphics_draw_text(font_small, str, x + 200, y, 1.0f, COLOR_SCORE_NUMBER_TEXT_MONEY);
    }
}

void game_draw_booster_items()
{
    graphics_set_no_texture();
    graphics_draw_quad(DRAW_BOOSTER_ITEMS_X, DRAW_BOOSTER_ITEMS_Y, DRAW_BOOSTER_ITEMS_WIDTH, CARD_HEIGHT, 0, 0, 0, 0, COLOR_SHADOW);

    for (int i = 0; i < g_game_state.shop.booster_total_items; i++)
    {
        if (g_game_state.shop.booster_items[i].available)
        {
            switch (g_game_state.shop.booster_items[i].type)
            {
                case ITEM_TYPE_CARD:
                    game_draw_card(&(g_game_state.shop.booster_items[i].info.card), NULL);
                    break;
                case ITEM_TYPE_JOKER:
                    game_draw_joker(&(g_game_state.shop.booster_items[i].info.joker));
                    break;
                case ITEM_TYPE_PLANET:
                    game_draw_planet(&(g_game_state.shop.booster_items[i].info.planet));
                    break;
                case ITEM_TYPE_TAROT:
                    game_draw_tarot(&(g_game_state.shop.booster_items[i].info.tarot));
                    break;
                case ITEM_TYPE_SPECTRAL:
                    game_draw_spectral(&(g_game_state.shop.booster_items[i].info.spectral));
                    break;
            }
        }
    }

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_BOOSTER_ITEMS)
    {
        if (g_game_state.highlighted_item > -1 && g_game_state.highlighted_item < g_game_state.shop.booster_total_items)
        {
            struct DrawObject *draw = &(g_game_state.shop.booster_items[g_game_state.highlighted_item].info.card.draw);
            game_draw_card_frame(draw);
            game_draw_item_button(draw, GAMEPAD_UI_RIGHT_TRIGGER, "Select", NULL, COLOR_RED_BUTTON, false);
            switch(g_game_state.shop.booster_items[g_game_state.highlighted_item].type)
            {
                case ITEM_TYPE_CARD:
                    game_draw_card_hint(&(g_game_state.shop.booster_items[g_game_state.highlighted_item].info.card));
                    break;
                case ITEM_TYPE_JOKER:
                    game_draw_joker_hint(&(g_game_state.shop.booster_items[g_game_state.highlighted_item].info.joker));
                    break;
                case ITEM_TYPE_PLANET:
                    game_draw_planet_hint(&(g_game_state.shop.booster_items[g_game_state.highlighted_item].info.planet));
                    break;
                case ITEM_TYPE_TAROT:
                    game_draw_tarot_hint(&(g_game_state.shop.booster_items[g_game_state.highlighted_item].info.tarot));
                    break;
                case ITEM_TYPE_SPECTRAL:
                    game_draw_spectral_hint(&(g_game_state.shop.booster_items[g_game_state.highlighted_item].info.spectral));
                    break;
            }
        }
    }
}

void game_draw_shop()
{
    game_draw_left_info();

    game_draw_deck();

    switch (g_game_state.sub_stage)
    {
        case GAME_SUBSTAGE_SHOP_MAIN:
        {
            float x = 127.0f;
            float y = 90.0f;

            graphics_draw_solid_quad(x, y, 290.0f, SCREEN_HEIGHT - y - 2.0f, COLOR_DARK_GREY_2);

            // Buttons
            if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND)
            {
                graphics_draw_solid_quad(x + 2.0f, y + 2.0f, 84.0f, 34.0f, COLOR_WHITE);
            }
            graphics_draw_solid_quad(x + 4.0f, y + 4.0f, 80.0f, 30.0f, COLOR_RED_BUTTON);
            graphics_draw_text_center(font_small, "Next Round", x + 4.0f + 40.0f, y + 4.0f + 15.0f, 1.0f, COLOR_WHITE);

            if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SHOP_REROLL)
            {
                graphics_draw_solid_quad(x + 2.0f, y + 36.0f, 84.0f, 34.0f, COLOR_WHITE);
            }
            graphics_draw_solid_quad(x + 4.0f, y + 38.0f, 80.0f, 30.0f, COLOR_GREEN_BUTTON);
            graphics_draw_text_center(font_small, "Reroll", x + 4.0f + 40.0f, y + 8.0f + 30.0f + 10.0f, 1.0f, COLOR_WHITE);
            char str[16];
            sprintf(str, "$%d", game_util_get_reroll_cost());
            graphics_draw_text_center(font_small, str, x + 4.0f + 40.0f, y + 8.0f + 30.0f + 15.0f + 8.0f, 1.0f, COLOR_WHITE);

            // Singles
            graphics_draw_solid_quad(x + 92.0f, y + 4.0f, 200.0f, 80.0f, COLOR_DARK_GREY);
            game_draw_shop_singles();

            // Boosters
            graphics_draw_solid_quad(x + 122.0f, y + 92.0f, 170.0f, 80.0f, COLOR_DARK_GREY);
            game_draw_shop_boosters();

            game_draw_shop_selected_single();

            game_draw_shop_selected_booster();

            break;
        }
        case GAME_SUBSTAGE_SHOP_BOOSTER_HAND:
        {
            game_draw_booster_hand();
        }
        case GAME_SUBSTAGE_SHOP_BOOSTER_NO_HAND:
        {
            graphics_draw_solid_quad(DRAW_BOOSTER_ITEMS_X + (DRAW_BOOSTER_ITEMS_WIDTH / 2.0f) - 50.0f, DRAW_BOOSTER_ITEMS_Y + CARD_HEIGHT, 100.0f, 30.0f, COLOR_DARK_GREY_2);
            graphics_draw_text_center(font_big, g_game_state.shop.opened_booster.size == BOOSTER_PACK_SIZE_MEGA ? "Choose 2" : "Choose 1",
                DRAW_BOOSTER_ITEMS_X + (DRAW_BOOSTER_ITEMS_WIDTH / 2.0f), DRAW_BOOSTER_ITEMS_Y + CARD_HEIGHT + 15.0f, 1.0f, COLOR_WHITE);

            game_draw_booster_items();

            break;
        }
    }

    game_draw_top_jokers();

    game_draw_consumables();

    switch (g_game_state.sub_stage)
    {
        case GAME_SUBSTAGE_SHOP_BOOSTER_HAND:
        {
            game_draw_booster_hand_selected_card();
            break;
        }
    }
}

void game_draw_score_number()
{
    if (g_game_state.score_number.show_score_number)
    {
        float x = 0.0f, y = 0.0f;
        if (g_game_state.score_number.card != NULL)
        {
            x = g_game_state.score_number.card->draw.x + CARD_WIDTH / 2.0f;
            y = g_game_state.score_number.card->draw.y - 16.0f;
        }
        else if (g_game_state.score_number.joker != NULL)
        {
            x = g_game_state.score_number.joker->draw.x + CARD_WIDTH / 2.0f;
            y = g_game_state.score_number.joker->draw.y + CARD_HEIGHT + 10.0f;
        }

        graphics_set_no_texture();
        float quad_size = 16.0f * g_game_state.score_number.back_quad_scale;

        switch (g_game_state.score_number.score_number_type)
        {
            case SCORE_NUMBER_ADD_CHIPS:
                graphics_draw_rotated_quad(x - quad_size / 2.0f, y - quad_size / 2.0f, quad_size, quad_size, 0, 0, 0, 0, COLOR_SCORE_NUMBER_TEXT_CHIPS, g_game_state.score_number.back_quad_angle);
                graphics_draw_text_center(font_big, g_game_state.score_number.text, x, y, g_game_state.score_number.text_scale, COLOR_WHITE);
                break;
            case SCORE_NUMBER_ADD_MULT:
                graphics_draw_rotated_quad(x - quad_size / 2.0f, y - quad_size / 2.0f, quad_size, quad_size, 0, 0, 0, 0, COLOR_SCORE_NUMBER_TEXT_ADD_MULT, g_game_state.score_number.back_quad_angle);
                graphics_draw_text_center(font_big, g_game_state.score_number.text, x, y, g_game_state.score_number.text_scale * 0.8, COLOR_WHITE);
                break;
            case SCORE_NUMBER_MULT_MULT:
            case SCORE_NUMBER_AGAIN:
                graphics_draw_rotated_quad(x - quad_size / 2.0f, y - quad_size / 2.0f, quad_size, quad_size, 0, 0, 0, 0, COLOR_SCORE_NUMBER_TEXT_MULT_MULT, g_game_state.score_number.back_quad_angle);
                graphics_draw_text_center(font_big, g_game_state.score_number.text, x, y, g_game_state.score_number.text_scale * 0.8, COLOR_WHITE);
                break;
            case SCORE_NUMBER_ADD_MONEY:
                graphics_draw_rotated_quad(x - quad_size / 2.0f, y - quad_size / 2.0f, quad_size, quad_size, 0, 0, 0, 0, COLOR_SCORE_NUMBER_TEXT_MONEY, g_game_state.score_number.back_quad_angle);
                graphics_draw_text_center(font_big, g_game_state.score_number.text, x, y, g_game_state.score_number.text_scale, COLOR_WHITE);
            default:
                break;
        }
    }
}

#define END_GAME_WIDTH 240
#define END_GAME_HEIGHT 220

void game_draw_end_game_info(const char *title, int color)
{
    char str[32];

    float x = (SCREEN_WIDTH - END_GAME_WIDTH) / 2.0f;
    float y = (SCREEN_HEIGHT - END_GAME_HEIGHT) / 2.0f;
    graphics_draw_solid_quad(x, y, END_GAME_WIDTH, END_GAME_HEIGHT, COLOR_DARK_GREY);

    y += 12.0f;
    graphics_draw_text_center(font_big, title, SCREEN_WIDTH / 2.0f, y, 1.0f, color);

    y += 20.0f;
    graphics_draw_solid_quad(x + 6.0f,  y - 6.0f, (END_GAME_WIDTH / 2.0f) - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + 70.0f, y - 2.0f, (END_GAME_WIDTH / 4.0f) - 24.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Ante", x + 28.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.ante);
    graphics_draw_text_center(font_small, str, x + (END_GAME_WIDTH / 4.0f) + 28.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    graphics_draw_solid_quad((END_GAME_WIDTH / 2.0f) + x + 6.0f, y - 6.0f, (END_GAME_WIDTH / 2.0f) - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad((END_GAME_WIDTH / 2.0f) + x + 70.0f, y - 2.0f, (END_GAME_WIDTH / 4.0f) - 24.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Round", (END_GAME_WIDTH / 2.0f) + x + 28.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.round);
    graphics_draw_text_center(font_small, str, (END_GAME_WIDTH / 2.0f) + x + (END_GAME_WIDTH / 4.0f) + 28.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 22.0f;
    graphics_draw_solid_quad(x + 6.0f, y - 6.0f, END_GAME_WIDTH - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH / 2.0f), y - 2.0f, (END_GAME_WIDTH / 2.0f) - 14.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Best Hand", x + 58.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%g", g_game_state.stats.max_score);
    graphics_draw_text_center(font_small, str, x + (END_GAME_WIDTH / 2.0f) + 54.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 22.0f;
    graphics_draw_solid_quad(x + 6.0f, y - 6.0f, END_GAME_WIDTH - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH / 2.0f), y - 2.0f, (END_GAME_WIDTH / 2.0f) - 14.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Most Played Hand", x + 58.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    graphics_draw_text_center(font_small, g_poker_hand_names[g_game_state.stats.most_played_poker_hand], x + (END_GAME_WIDTH / 2.0f) + 54.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 22.0f;
    graphics_draw_solid_quad(x + 6.0f, y - 6.0f, END_GAME_WIDTH - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH / 2.0f), y - 2.0f, (END_GAME_WIDTH / 2.0f) - 14.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Cards Played", x + 58.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.stats.cards_played);
    graphics_draw_text_center(font_small, str, x + (END_GAME_WIDTH / 2.0f) + 54.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 22.0f;
    graphics_draw_solid_quad(x + 6.0f, y - 6.0f, END_GAME_WIDTH - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH / 2.0f), y - 2.0f, (END_GAME_WIDTH / 2.0f) - 14.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Cards Discarded", x + 58.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.stats.cards_discarded);
    graphics_draw_text_center(font_small, str, x + (END_GAME_WIDTH / 2.0f) + 54.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 22.0f;
    graphics_draw_solid_quad(x + 6.0f, y - 6.0f, END_GAME_WIDTH - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH / 2.0f), y - 2.0f, (END_GAME_WIDTH / 2.0f) - 14.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Cards Purchased", x + 58.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.stats.cards_purchased);
    graphics_draw_text_center(font_small, str, x + (END_GAME_WIDTH / 2.0f) + 54.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 22.0f;
    graphics_draw_solid_quad(x + 6.0f, y - 6.0f, END_GAME_WIDTH - 12.0f, 20.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH / 2.0f), y - 2.0f, (END_GAME_WIDTH / 2.0f) - 14.0f, 12.0f, COLOR_DARK_GREY);
    graphics_draw_text_center(font_small, "Times Rerolled", x + 58.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    sprintf(str, "%d", g_game_state.stats.times_rerolled);
    graphics_draw_text_center(font_small, str, x + (END_GAME_WIDTH / 2.0f) + 54.0f, y + 4.0f, 1.0f, COLOR_WHITE);
    y += 30.0f;
    graphics_draw_solid_quad(x + (END_GAME_WIDTH - 100.0f) / 2.0f - 2.0f, y - 6.0f - 2.0f, 100.0f + 4.0f, 24.0f, COLOR_WHITE);
    graphics_draw_solid_quad(x + (END_GAME_WIDTH - 100.0f) / 2.0f, y - 6.0f, 100.0f, 20.0f, COLOR_RED_BUTTON);
    graphics_draw_text_center(font_small, "Start New Game", x + (END_GAME_WIDTH / 2.0f), y + 4.0f, 1.0f, COLOR_WHITE);
}

void game_draw_you_win_panel()
{
    game_draw_end_game_info("YOU WIN", COLOR_LIGHT_BLUE);
}

void game_draw_game_over_panel()
{
    game_draw_end_game_info("GAME OVER", COLOR_LIGHT_RED);
}

void game_draw_ingame()
{
    game_draw_left_info();

    game_draw_deck();

    game_draw_top_jokers();

    game_draw_consumables();

    switch (g_game_state.sub_stage)
    {
        case GAME_SUBSTAGE_INGAME_PICK_HAND:
        {
            game_draw_ingame_hand();
            break;
        }
        case GAME_SUBSTAGE_INGAME_WON:
        {
            game_draw_cash_out_panel();
            break;
        }
        case GAME_SUBSTAGE_INGAME_WON_END:
        {
            game_draw_you_win_panel();
            break;
        }
        case GAME_SUBSTAGE_INGAME_LOST_END:
        {
            game_draw_game_over_panel();
            break;
        }
    }

    game_draw_score_number();
}

int game_draw_get_blind_reward(int blind)
{
    switch (blind)
    {
        case GAME_BLIND_SMALL:
            return 3;
        case GAME_BLIND_LARGE:
            return 4;
        case GAME_BLIND_BOSS:
            return (g_game_state.boss_blind_type >= 0 && g_game_state.boss_blind_type < BOSS_BLIND_COUNT) ?
                g_boss_blind_types[g_game_state.boss_blind_type].reward : 5;
        default:
            return 0;
    }
}

static void game_draw_blind_tag_icon(int tag_type, float x, float y, float size, uint32_t color)
{
    if (tag_type < 0 || tag_type >= BLIND_TAG_COUNT) return;

    graphics_set_texture(tex_tags, GRAPHICS_TEXTURE_FILTER_NEAREST);
    graphics_draw_quad(x, y, size, size, g_blind_tag_types[tag_type].tex_x * 34, g_blind_tag_types[tag_type].tex_y * 34, 34, 34, color);
}

static void game_draw_blind_action_button(float x, float y, float w, const char *text, bool selected, bool enabled)
{
    uint32_t color = enabled ? COLOR_RED_BUTTON : COLOR_DARK_GREY;
    graphics_set_no_texture();
    if (selected)
    {
        graphics_draw_solid_quad(x - 2.0f, y - 2.0f, w + 4.0f, 22.0f, COLOR_WHITE);
    }
    graphics_draw_solid_quad(x, y, w, 18.0f, color);
    game_draw_text_fit_center(font_small, text, x + w / 2.0f, y + 9.0f, w - 4.0f, 1.0f, enabled ? COLOR_WHITE : COLOR_LIGHT_GREY_2);
}

static void game_draw_blind_tag_popup()
{
    if (!g_game_state.blind_tag_description_open || g_game_state.blind == GAME_BLIND_BOSS) return;

    int tag_type = g_game_state.blind_tags[g_game_state.blind];
    if (tag_type < 0 || tag_type >= BLIND_TAG_COUNT) return;

    float x = 282.0f;
    float y = 88.0f;
    graphics_set_no_texture();
    graphics_draw_solid_quad(x - 2.0f, y - 2.0f, 178.0f, 82.0f, COLOR_WHITE);
    graphics_draw_solid_quad(x, y, 174.0f, 78.0f, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(x + 2.0f, y + 2.0f, 170.0f, 74.0f, COLOR_DARK_GREY);

    game_draw_blind_tag_icon(tag_type, x + 7.0f, y + 7.0f, 24.0f, COLOR_WHITE);
    game_draw_text_fit(font_big, g_blind_tag_types[tag_type].name, x + 36.0f, y + 11.0f, 128.0f, 1.0f, COLOR_WHITE);

    for (int i = 0; i < 3; i++)
    {
        if (g_blind_tag_types[tag_type].description[i][0] == '\0') continue;
        game_draw_text_fit(font_small, g_blind_tag_types[tag_type].description[i], x + 8.0f, y + 40.0f + i * 10.0f, 158.0f, 1.0f, COLOR_WHITE);
    }
}

void game_draw_blind_select()
{
    char str[32];

    game_draw_left_info();

    game_draw_deck();

    int x = 127;
    for(int i = 0; i < 3; i++)
    {
        graphics_set_no_texture();

        int y = 84;

        if (i == g_game_state.blind && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_BLIND)
        {
            graphics_draw_quad(x - 2, y - 2, 94, SCREEN_HEIGHT - 82, 0, 0, 0, 0, COLOR_WHITE);
        }

        graphics_draw_quad(x, y, 90, SCREEN_HEIGHT - 84, 0, 0, 0, 0, COLOR_DARK_GREY);

        y += 5;
        int blind_row = 0;
        if (i == GAME_BLIND_SMALL) blind_row = 0;
        else if (i == GAME_BLIND_LARGE) blind_row = 1;
        else if (i == GAME_BLIND_BOSS) {
            if (g_game_state.boss_blind_type >= 0 && g_game_state.boss_blind_type < BOSS_BLIND_COUNT) {
                blind_row = g_boss_blind_tex_rows[g_game_state.boss_blind_type];
            } else {
                blind_row = g_boss_blind_tex_rows[0];
            }
        }

        int frame = (g_game_counter / 4) % 21;
        graphics_set_texture(tex_blind_chips[frame/15][blind_row/15], GRAPHICS_TEXTURE_FILTER_NEAREST);
        graphics_draw_quad(x + 30, y, 30, 30, (frame % 15) * 34, (blind_row % 15) * 34, 34, 34, COLOR_WHITE);
        graphics_set_no_texture();

        y += 34;
        game_draw_text_fit(font_small, game_util_get_blind_name(i), x + 2, y, 84.0f, i == GAME_BLIND_BOSS ? 1.1f : 1.0f, COLOR_WHITE);

        y += 11;
        graphics_draw_text(font_small, "Score at least", x + 2, y, 1.0f, COLOR_WHITE);

        y += 9;
        graphics_set_texture(tex_ui_assets, GRAPHICS_TEXTURE_FILTER_NEAREST);
        graphics_draw_quad(x + 2, y - 1, 10, 10, g_ui_assets_tex_coords[UI_ASSETS_CHIP].x, g_ui_assets_tex_coords[UI_ASSETS_CHIP].y, 18, 18, COLOR_WHITE);
        game_draw_format_score_value(str, sizeof(str), game_get_blind_score(i));
        game_draw_text_fit(font_small, str, x + 14, y, 72.0f, 1.0f, COLOR_LIGHT_RED);

        y += 14;
        sprintf(str, "Reward $%d", game_draw_get_blind_reward(i));
        game_draw_text_fit(font_small, str, x + 2, y, 84.0f, 1.0f, COLOR_YELLOW);

        y += 16;
        const char *effect_line_1 = "";
        const char *effect_line_2 = "";
        if (i == GAME_BLIND_BOSS &&
            g_game_state.boss_blind_type >= 0 &&
            g_game_state.boss_blind_type < BOSS_BLIND_COUNT)
        {
            effect_line_1 = g_boss_blind_types[g_game_state.boss_blind_type].effect_line_1;
            effect_line_2 = g_boss_blind_types[g_game_state.boss_blind_type].effect_line_2;
        }
        if (i == GAME_BLIND_BOSS)
        {
            game_draw_text_fit(font_small, effect_line_1, x + 2, y, 84.0f, 1.0f, COLOR_WHITE);
            y += 10;
            game_draw_text_fit(font_small, effect_line_2, x + 2, y, 84.0f, 1.0f, COLOR_WHITE);
        }
        else
        {
            y += 10;
        }

        if (i != GAME_BLIND_BOSS)
        {
            int tag_type = g_game_state.blind_tags[i];
            if (tag_type >= 0 && tag_type < BLIND_TAG_COUNT)
            {
                bool current = i == g_game_state.blind && g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_BLIND;
                bool enabled = i == g_game_state.blind;
                float action_y = 233.0f;

                graphics_draw_text_center(font_small, "or", x + 45.0f, action_y - 8.0f, 1.0f, enabled ? COLOR_WHITE : COLOR_LIGHT_GREY_2);

                if (current && g_game_state.blind_focused_action == BLIND_ACTION_TAG)
                {
                    graphics_set_no_texture();
                    graphics_draw_solid_quad(x + 7.0f, action_y - 2.0f, 24.0f, 24.0f, COLOR_WHITE);
                }
                graphics_set_no_texture();
                graphics_draw_solid_quad(x + 9.0f, action_y, 20.0f, 20.0f, enabled ? COLOR_LIGHT_GREY_2 : COLOR_DARK_GREY_2);
                game_draw_blind_tag_icon(tag_type, x + 10.0f, action_y + 1.0f, 18.0f, enabled ? COLOR_WHITE : 0x88666666);

                game_draw_blind_action_button(
                    x + 34.0f,
                    action_y + 3.0f,
                    48.0f,
                    "Skip",
                    current && g_game_state.blind_focused_action == BLIND_ACTION_SKIP,
                    enabled);
            }
        }

        if (i != g_game_state.blind)
        {
            graphics_set_no_texture();
            graphics_draw_quad(x, 84, 90, SCREEN_HEIGHT - 84, 0, 0, 0, 0, 0xAA666666);
        }

        x += 100;
    }

    game_draw_top_jokers();

    game_draw_consumables();

    game_draw_blind_tag_popup();

    // DEBUG temporario (remover depois): estado do input/evento.
    {
        char dbg[64];
        sprintf(dbg, "st%d sub%d foc%d", g_game_state.stage, g_game_state.sub_stage, (int)g_game_state.input_focused_zone);
        graphics_draw_text(font_small, dbg, 2.0f, 4.0f, 1.0f, COLOR_TEXT_GREEN);
        sprintf(dbg, "ev%d id%d sg%d", automated_event_debug_depth(), automated_event_debug_top(), automated_event_debug_stage());
        graphics_draw_text(font_small, dbg, 2.0f, 14.0f, 1.0f, COLOR_TEXT_GREEN);
        sprintf(dbg, "tag%d act%d", g_game_state.blind_tag_description_open ? 1 : 0, g_game_state.blind_focused_action);
        graphics_draw_text(font_small, dbg, 2.0f, 24.0f, 1.0f, COLOR_TEXT_GREEN);
    }

    game_draw_score_number();
}

int inner_game_draw_offscreen_deck_info_rank_func(const void *a, const void *b)
{
    struct Card *card_a = *((struct Card **)a);
    struct Card *card_b = *((struct Card **)b);

    if (card_a == NULL) return 1;
    if (card_b == NULL) return -1;

    int rank_a = card_a->enhancement == CARD_ENHANCEMENT_STONE ? -1 : card_a->rank;
    int rank_b = card_b->enhancement == CARD_ENHANCEMENT_STONE ? -1 : card_b->rank;

    return rank_b - rank_a;
}

void game_draw_offscreen_deck_info()
{
    graphics_set_offscreen_render_target();

    graphics_draw_solid_quad(0, 0, DRAW_DECK_INFO_WIDTH, DRAW_DECK_INFO_HEIGHT, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(2, 2, DRAW_DECK_INFO_WIDTH - 4.0f, DRAW_DECK_INFO_HEIGHT - 4.0f, COLOR_DARK_GREY);

    for (int i = 0; i < 4; i++)
    {
        g_game_state.deck_info.card_count[i] = 0;
    }

    for (int j = 0; j < CARD_RANK_COUNT; j++) { g_game_state.deck_info.rank_count[j] = 0; g_game_state.deck_info.effective_rank_count[j] = 0; }
    for (int j = 0; j < 4; j++) { g_game_state.deck_info.suit_count[j] = 0; g_game_state.deck_info.effective_suit_count[j] = 0; }

    g_game_state.deck_info.face_count = 0;
    g_game_state.deck_info.effective_face_count = 0;
    g_game_state.deck_info.number_count = 0;
    g_game_state.deck_info.effective_number_count = 0;

    for (int i = 0; i < (g_game_state.deck_info.partial ? g_game_state.current_deck.card_count : g_game_state.full_deck.card_count); i++)
    {
        struct Card *card = g_game_state.full_deck.cards[i];
        if (g_game_state.deck_info.partial)
        {
            card = g_game_state.current_deck.cards[i];
        }

        for (int j = 0; j < CARD_RANK_COUNT; j++)
        {
            if (card->rank == j)
            {
                g_game_state.deck_info.rank_count[j]++;
            }

            if (game_util_is_card_rank(card, j))
            {
                g_game_state.deck_info.effective_rank_count[j]++;
            }
        }

        for (int j = 0; j < 4; j++)
        {
            if (card->suit == j)
            {
                g_game_state.deck_info.suit_count[j]++;
            }

            if (game_util_is_card_suit(card, j))
            {
                g_game_state.deck_info.effective_suit_count[j]++;
            }
        }

        if (card->rank == CARD_RANK_JACK || card->rank == CARD_RANK_QUEEN || card->rank == CARD_RANK_KING)
        {
            g_game_state.deck_info.face_count++;
        }
        else
        {
            g_game_state.deck_info.number_count++;
        }

        if (game_util_is_card_face(card))
        {
            g_game_state.deck_info.effective_face_count++;
        }
        else
        {
            g_game_state.deck_info.effective_number_count++;
        }
    }

    for (int i = 0; i < g_game_state.full_deck.card_count; i++)
    {
        struct Card *card = g_game_state.full_deck.cards[i];

        g_game_state.deck_info.cards[card->suit][g_game_state.deck_info.card_count[card->suit]] = card;
        g_game_state.deck_info.card_count[card->suit]++;
    }

    for (int i = 0; i < 4; i++)
    {
        qsort((void*)(g_game_state.deck_info.cards[i]), g_game_state.deck_info.card_count[i], sizeof(struct Card *), inner_game_draw_offscreen_deck_info_rank_func);
    }

    bool in_current_deck = false;

    g_freeze_cards = true;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < g_game_state.deck_info.card_count[i]; j++)
        {
            struct Card *card = g_game_state.deck_info.cards[i][j];
            struct DrawObject *draw = &(g_game_state.deck_info.draw[i][j]);
            game_init_draw_object(draw);
            draw->x = game_util_get_item_position(j, g_game_state.deck_info.card_count[i], 90.0f, DRAW_DECK_INFO_WIDTH - 96.0f, CARD_WIDTH);
            draw->y = 6.0f + 50.0f * i;

            game_draw_card(card, draw);

            in_current_deck = false;
            for(int w = 0; w < g_game_state.current_deck.card_count; w++)
            {
                if (g_game_state.current_deck.cards[w] == card)
                {
                    in_current_deck = true;
                    break;
                }
            }

            g_game_state.deck_info.in_current_deck[i][j] = in_current_deck;

            if (g_game_state.deck_info.partial && !in_current_deck)
            {
                graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
                graphics_draw_quad(draw->x, draw->y, CARD_WIDTH, CARD_HEIGHT, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0xC0FFFFFF & COLOR_DARK_GREY);
            }

            draw->x += (SCREEN_WIDTH - DRAW_DECK_INFO_WIDTH) / 2.0f;
            draw->y += (SCREEN_HEIGHT - DRAW_DECK_INFO_HEIGHT) / 2.0f;
        }
    }

    graphics_unset_offscreen_render_target();
}

// void game_draw_deck_draw_cards()
// {
//     float left = (SCREEN_WIDTH - DRAW_DECK_INFO_WIDTH) / 2.0f;
//     float top = (SCREEN_HEIGHT - DRAW_DECK_INFO_HEIGHT) / 2.0f;

//     graphics_draw_solid_quad(left + 0, top + 0, DRAW_DECK_INFO_WIDTH, DRAW_DECK_INFO_HEIGHT, COLOR_DARK_GREY_2);
//     graphics_draw_solid_quad(left + 2, top + 2, DRAW_DECK_INFO_WIDTH - 4.0f, DRAW_DECK_INFO_HEIGHT - 4.0f, COLOR_DARK_GREY);

//     for (int i = 0; i < 4; i++)
//     {
//         for (int j = 0; j < g_game_state.deck_info.card_count[i]; j++)
//         {
//             struct Card *card = g_game_state.deck_info.cards[i][j];
//             struct DrawObject *draw = &(g_game_state.deck_info.draw[i][j]);

//             game_draw_card(card, draw);

//             if (g_game_state.deck_info.partial && !g_game_state.deck_info.in_current_deck[i][j])
//             {
//                 graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
//                 graphics_draw_quad(draw->x, draw->y, CARD_WIDTH, CARD_HEIGHT, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0xC0FFFFFF & COLOR_DARK_GREY);
//             }
//         }
//     }
// }

void game_draw_deck_info()
{
    char str[16];
    float left = (SCREEN_WIDTH - DRAW_DECK_INFO_WIDTH) / 2.0f;
    float top = (SCREEN_HEIGHT - DRAW_DECK_INFO_HEIGHT) / 2.0f;

    graphics_set_texture(-2, 0);
    graphics_draw_quad(left, top, DRAW_DECK_INFO_WIDTH, DRAW_DECK_INFO_HEIGHT, 0, 0, DRAW_DECK_INFO_WIDTH, DRAW_DECK_INFO_HEIGHT, COLOR_WHITE);

    graphics_draw_solid_quad(left + 4.0f, top + 6.0f, 80.0f, 14.0f, COLOR_DARK_GREY_2);
    if (g_game_state.deck_info.partial)
    {
        graphics_draw_text_center(font_small, "Remaining", left + 35.0f, top + 7.0f + 6.0f, 1.0f, COLOR_WHITE);
    }
    else
    {
        graphics_draw_text_center(font_small, "Full", left + 35.0f, top + 7.0f + 6.0f, 1.0f, COLOR_WHITE);
    }
    graphics_set_texture(tex_gamepad_ui, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_quad(left + 4.0f + 60.0f, top + 5.0f, 18.0f, 18.0f, g_gamepad_ui_text_coords[GAMEPAD_UI_LEFT_TRIGGER].x, g_gamepad_ui_text_coords[GAMEPAD_UI_LEFT_TRIGGER].y, 32, 32, COLOR_WHITE);

    graphics_draw_solid_quad(left + 4.0f, top + 4.0f + 20.0f, 80.0f, 14.0f, COLOR_DARK_GREY_2);
    if (g_game_state.deck_info.effective)
    {
        graphics_draw_text_center(font_small, "Effective", left + 35.0f, top + 20.0f + 4.0f + 7.0f, 1.0f, COLOR_LIGHT_BLUE);
    }
    else
    {
        graphics_draw_text_center(font_small, "Base", left + 35.0f, top + 20.0f + 4.0f + 7.0f, 1.0f, COLOR_WHITE);
    }
    graphics_set_texture(tex_gamepad_ui, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_quad(left + 4.0f + 60.0f, top + 5.0f + 18.0f, 18.0f, 18.0f, g_gamepad_ui_text_coords[GAMEPAD_UI_RIGHT_TRIGGER].x, g_gamepad_ui_text_coords[GAMEPAD_UI_RIGHT_TRIGGER].y, 32, 32, COLOR_WHITE);

    float suit_left = 60.0f;
    float suit_top = 48.0f;

    int c = 0;
    for (int i = CARD_RANK_COUNT - 1; i >= 0; i--, c++)
    {
        graphics_draw_solid_quad(left + suit_left - 7.0f, top + suit_top - 6.0f + c * 14.0f, 14.0f, 12.0f, COLOR_DARK_GREY_2);
        graphics_draw_text_center(font_small, g_short_rank_name[i], left + suit_left, top + suit_top + c * 14.0f, 1.0f, COLOR_LIGHT_GREY);
        if (g_game_state.deck_info.effective)
        {
            sprintf(str, "%d", g_game_state.deck_info.effective_rank_count[i]);
        }
        else
        {
            sprintf(str, "%d", g_game_state.deck_info.rank_count[i]);
        }
        graphics_draw_text_center(font_small, str, left + suit_left + 18.0f, top + suit_top + c * 14.0f, 1.0f, g_game_state.deck_info.effective ? COLOR_LIGHT_BLUE : COLOR_WHITE);
    }

    float other_left = 8.0f;
    float other_top = 54.0f;

    for (int i = 0; i < UI_ASSETS_COUNT - 2; i++)
    {
        graphics_draw_solid_quad(left + other_left - 2.0f, top + other_top - 2.0f + i * 28.0f, 22.0f, 22.0f, COLOR_LIGHT_GREY_2);
        graphics_set_texture(tex_ui_assets, GRAPHICS_TEXTURE_FILTER_NEAREST);

        int asset = i+2;
        if (asset == 5) { asset = 6; }
        else if (asset == 6) { asset = 5; }
        graphics_draw_quad(left + other_left, top + other_top + i * 28.0f, 18, 18, g_ui_assets_tex_coords[asset].x, g_ui_assets_tex_coords[asset].y, 18, 18, COLOR_WHITE);
        if (g_game_state.deck_info.effective)
        {
            if (i == 0) { sprintf(str, "%d", g_game_state.deck_info.effective_face_count); }
            else if (i == 1) { sprintf(str, "%d", g_game_state.deck_info.effective_number_count); }
            else { sprintf(str, "%d", g_game_state.deck_info.effective_suit_count[i - 2]); }
        }
        else
        {
            if (i == 0) { sprintf(str, "%d", g_game_state.deck_info.face_count); }
            else if (i == 1) { sprintf(str, "%d", g_game_state.deck_info.number_count); }
            else { sprintf(str, "%d", g_game_state.deck_info.suit_count[i - 2]); }
        }
        graphics_draw_text_center(font_small, str, left + other_left + 32.0f, top + other_top + i * 28.0f + 9.0f, 1.0f, g_game_state.deck_info.effective ? COLOR_LIGHT_BLUE : COLOR_WHITE);
    }

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_DECK_INFO_CARD)
    {
        g_freeze_cards = true;
        game_draw_card(&g_game_state.deck_info.current_card, NULL);
        if (g_game_state.deck_info.partial && !g_game_state.deck_info.in_current_deck[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x])
        {
            graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
            graphics_draw_quad(g_game_state.deck_info.current_card.draw.x, g_game_state.deck_info.current_card.draw.y, CARD_WIDTH, CARD_HEIGHT, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].x, g_enhancement_tex_coords[CARD_ENHANCEMENT_NONE].y, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, 0xC0FFFFFF & COLOR_DARK_GREY);
        }
        game_draw_card_frame(&(g_game_state.deck_info.current_card.draw));
        game_draw_card_hint(&g_game_state.deck_info.current_card);
    }

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE)
    {
        graphics_draw_solid_quad((SCREEN_WIDTH - 50.0f) / 2.0f - 2.0f, 237.0f - 2.0f, 50.0f + 4.0f, 15.0f + 4.0f, COLOR_WHITE);
    }
    graphics_draw_solid_quad((SCREEN_WIDTH - 50.0f) / 2.0f, 237.0f, 50.0f, 15.0f, COLOR_RED_BUTTON);
    graphics_draw_text_center(font_small, "Close", (SCREEN_WIDTH) / 2.0f, 244.0f, 1.0f, COLOR_WHITE);

}

void game_draw_run_info()
{
    float left = (SCREEN_WIDTH - DRAW_RUN_INFO_WIDTH) / 2.0f;
    float top = (SCREEN_HEIGHT - DRAW_RUN_INFO_HEIGHT) / 2.0f;

    graphics_draw_solid_quad(left, top, DRAW_RUN_INFO_WIDTH, DRAW_RUN_INFO_HEIGHT, COLOR_DARK_GREY_2);
    graphics_draw_solid_quad(left + 2, top + 2, DRAW_RUN_INFO_WIDTH - 4.0f, DRAW_RUN_INFO_HEIGHT - 4.0f, COLOR_DARK_GREY);

    graphics_draw_text_center(font_big, "Poker Hands", SCREEN_WIDTH / 2.0f, top + 12.0f, 1.0f, COLOR_WHITE);

    float y = 8.0f;
    char str[16];
    top += 20.0f;
    for (int i = 1; i < GAME_POKER_HAND_COUNT; i++)
    {
        graphics_draw_solid_quad(left + 4.0f, top + y - 2.0f, 304.0f, 12.0f, COLOR_LIGHT_GREY);

        graphics_draw_solid_quad(left + 179.0f, top + y - 2.0f, 92.0f, 12.0f, COLOR_DARK_GREY_2);
        graphics_draw_solid_quad(left + 180.0f, top + y - 1.0f, 40.0f, 10.0f, COLOR_LIGHT_BLUE);
        graphics_draw_solid_quad(left + 230.0f, top + y - 1.0f, 40.0f, 10.0f, COLOR_LIGHT_RED);

        graphics_draw_solid_quad(left + 285.0f, top + y - 1.0f, 20.0f, 10.0f, COLOR_DARK_GREY_2);

        graphics_draw_text_center(font_small, "Lvl.", left + 20.0f, top + y + 4.0f, 1.0f, COLOR_WHITE);

        sprintf(str, "%d", g_game_state.poker_hand_level[i]);
        graphics_draw_text_center(font_small, str, left + 40.0f, top + y + 4.0f, 1.0f, COLOR_WHITE);

        graphics_draw_text_center(font_small, g_poker_hand_names[i], left + 110.0f, top + y + 4.0f, 1.0f, COLOR_WHITE);

        sprintf(str, "%d", g_game_state.poker_hand_base_chips[i]);
        graphics_draw_text_center(font_small, str, left + 200.0f, top + y + 4.0f, 1.0f, COLOR_WHITE);

        graphics_draw_text_center(font_small, "x", left + 226.0f, top + y + 4.0f, 1.0f, COLOR_LIGHT_RED);

        sprintf(str, "%d", g_game_state.poker_hand_base_mult[i]);
        graphics_draw_text_center(font_small, str, left + 250.0f, top + y + 4.0f, 1.0f, COLOR_WHITE);

        graphics_draw_text_center(font_small, "#", left + 280.0f, top + y + 4.0f, 1.0f, COLOR_WHITE);
        sprintf(str, "%d", g_game_state.stats.poker_hands_played[i]);
        graphics_draw_text_center(font_small, str, left + 295.0f, top + y + 4.0f, 1.0f, COLOR_TEXT_ORANGE);

        y += 17.0f;
    }

    if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_RUN_INFO_CLOSE)
    {
        graphics_draw_solid_quad((SCREEN_WIDTH - 50.0f) / 2.0f - 2.0f, 240.0f - 2.0f, 50.0f + 4.0f, 15.0f + 4.0f, COLOR_WHITE);
    }
    graphics_draw_solid_quad((SCREEN_WIDTH - 50.0f) / 2.0f, 240.0f, 50.0f, 15.0f, COLOR_RED_BUTTON);
    graphics_draw_text_center(font_small, "Close", (SCREEN_WIDTH) / 2.0f, 247.0f, 1.0f, COLOR_WHITE);
}

uint64_t last_tick = 0;
uint32_t tick_res = 0;
float curr_ms = 1.0f;
float curr_fps = 0.0f;
int g_frame_count;

void game_draw_debug_info()
{
    g_time++;

    if (last_tick == 0) { sceRtcGetCurrentTick(&last_tick); }
    if (tick_res == 0) { tick_res = sceRtcGetTickResolution(); }

    g_frame_count++;

    uint64_t curr_tick;
	sceRtcGetCurrentTick(&curr_tick);

    if ((curr_tick-last_tick) >= tick_res)
    {
        curr_fps = 1.0f / curr_ms;

        double time_span = ((curr_tick-last_tick)) / (float)tick_res;
        curr_ms = time_span / g_frame_count;
        g_frame_count = 0;
        sceRtcGetCurrentTick(&last_tick);
    }

    #ifdef DEBUG
    char str[32];
    sprintf(str, "FPS: %0.2f", curr_fps);
    graphics_draw_text(font_small, str, 400, 0, 1.0f, COLOR_WHITE);
    if (g_settings.audio)
    {
        sprintf(str, "audio w read: %d", g_debug_info.audio_wait_read);
        graphics_draw_text(font_small, str, 370, 8, 1.0f, COLOR_WHITE);
        sprintf(str, "audio w write: %d", g_debug_info.audio_wait_write);
        graphics_draw_text(font_small, str, 364, 16, 1.0f, COLOR_WHITE);
    }
    #endif
}

void game_draw_pause_menu()
{
    // Draw a dark overlay
    graphics_draw_solid_quad(0, 0, 480, 272, 0x88000000);

    // Draw the dialog box
    float box_w = 200.0f;
    float box_h = 172.0f;
    float box_x = (480.0f - box_w) / 2.0f;
    float box_y = (272.0f - box_h) / 2.0f;

    graphics_draw_solid_quad(box_x, box_y, box_w, box_h, COLOR_BLACK);

    // Draw the border
    graphics_draw_solid_quad(box_x - 2.0f, box_y - 2.0f, box_w + 4.0f, 2.0f, COLOR_WHITE);
    graphics_draw_solid_quad(box_x - 2.0f, box_y + box_h, box_w + 4.0f, 2.0f, COLOR_WHITE);
    graphics_draw_solid_quad(box_x - 2.0f, box_y, 2.0f, box_h, COLOR_WHITE);
    graphics_draw_solid_quad(box_x + box_w, box_y, 2.0f, box_h, COLOR_WHITE);

    graphics_draw_text_center(font_big, "PAUSED", 240.0f, box_y + 10.0f, 1.5f, COLOR_WHITE);

    // Menu options
    uint32_t color_continue = (g_game_state.highlighted_item == 0) ? COLOR_TEXT_YELLOW : COLOR_WHITE;
    uint32_t color_savequit = (g_game_state.highlighted_item == 1) ? COLOR_TEXT_YELLOW : COLOR_WHITE;
    uint32_t color_load = (g_game_state.highlighted_item == 2) ? COLOR_TEXT_YELLOW : COLOR_WHITE;
    uint32_t color_settings = (g_game_state.highlighted_item == 3) ? COLOR_TEXT_YELLOW : COLOR_WHITE;
    uint32_t color_main_menu = (g_game_state.highlighted_item == 4) ? COLOR_TEXT_YELLOW : COLOR_WHITE;

    graphics_draw_text_center(font_small, "Continue", 240.0f, box_y + 40.0f, 1.0f, color_continue);
    graphics_draw_text_center(font_small, "Save", 240.0f, box_y + 64.0f, 1.0f, color_savequit);
    graphics_draw_text_center(font_small, "Load", 240.0f, box_y + 88.0f, 1.0f, color_load);
    graphics_draw_text_center(font_small, "Settings", 240.0f, box_y + 112.0f, 1.0f, color_settings);
    graphics_draw_text_center(font_small, "Main Menu", 240.0f, box_y + 136.0f, 1.0f, color_main_menu);
}

void game_draw_settings_menu()
{
    graphics_draw_solid_quad(0, 0, 480, 272, 0x88000000);

    float box_w = 300.0f;
    float box_h = 212.0f;
    float box_x = (480.0f - box_w) / 2.0f;
    float box_y = (272.0f - box_h) / 2.0f;

    graphics_draw_solid_quad(box_x, box_y, box_w, box_h, COLOR_BLACK);
    graphics_draw_solid_quad(box_x - 2.0f, box_y - 2.0f, box_w + 4.0f, 2.0f, COLOR_WHITE);
    graphics_draw_solid_quad(box_x - 2.0f, box_y + box_h, box_w + 4.0f, 2.0f, COLOR_WHITE);
    graphics_draw_solid_quad(box_x - 2.0f, box_y, 2.0f, box_h, COLOR_WHITE);
    graphics_draw_solid_quad(box_x + box_w, box_y, 2.0f, box_h, COLOR_WHITE);

    graphics_draw_text_center(font_big, "SETTINGS", 240.0f, box_y + 10.0f, 1.2f, COLOR_WHITE);

    const char *labels[] = { "Audio", "Music Vol", "SFX Vol", "Card Motion", "Debug Tools", "Score Speed", "Ante Scaling", "Overclock", "Back" };
    char values[9][24];
    strcpy(values[0], g_settings.audio ? "On" : "Off");
    sprintf(values[1], "%d", g_settings.music_volume);
    sprintf(values[2], "%d", g_settings.sfx_volume);
    strcpy(values[3], g_settings.move_cards ? "On" : "Off");
    strcpy(values[4], g_settings.debug_tools ? "On" : "Off");
    sprintf(values[5], "%d", g_settings.speed);
    sprintf(values[6], "%d", g_settings.ante_score_scaling);
    strcpy(values[7], g_settings.overclock ? "333 MHz" : "Default");
    values[8][0] = '\0';

    for (int i = 0; i < 9; i++)
    {
        float y = box_y + 36.0f + 18.0f * i;
        uint32_t color = (g_game_state.highlighted_item == i) ? COLOR_TEXT_YELLOW : COLOR_WHITE;
        graphics_draw_text(font_small, labels[i], box_x + 24.0f, y, 1.0f, color);
        if (values[i][0] != '\0')
        {
            graphics_draw_text_center(font_small, values[i], box_x + box_w - 58.0f, y + 4.0f, 1.0f, color);
        }
    }
}

#define TITLE_COLOR_PANEL_FILL    0xFF413D2C // #2C3D41
#define TITLE_COLOR_PANEL_BORDER  0xFF55503A // #3A5055
#define TITLE_COLOR_BLUE          0xFFFF9D00 // #009DFF
#define TITLE_COLOR_ORANGE        0xFF00A2FD // #FDA200
#define TITLE_COLOR_RED           0xFF555FFE // #FE5F55
#define TITLE_COLOR_GREEN         0xFF87A856 // #56A887
#define TITLE_COLOR_DARK          0xFF67634F // #4F6367

static uint32_t game_draw_title_color_alpha(uint32_t color, float alpha)
{
    if (alpha <= 0.0f) return color & 0x00FFFFFFu;
    if (alpha >= 1.0f) return color;
    return (color & 0x00FFFFFFu) | ((uint32_t)(alpha * 255.0f + 0.5f) << 24);
}

static float game_draw_title_ease(float t)
{
    t = CLAMP(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static void game_draw_title_button(float x, float y, float w, float h, const char *label,
                                   uint32_t color, bool highlighted, float alpha)
{
    uint32_t fill = game_draw_title_color_alpha(color, alpha);
    uint32_t border = highlighted ? game_draw_title_color_alpha(COLOR_WHITE, alpha) : fill;
    float scale = 1.2f;
    float text_width = (float)strlen(label) * 8.0f * scale;
    if (text_width > w - 8.0f)
    {
        scale = (w - 8.0f) / ((float)strlen(label) * 8.0f);
    }

    game_draw_rounded_box(x, y, w, h, 4.0f, fill, border, 2.0f);
    graphics_draw_text_center(font_big, label, x + w * 0.5f, y + h * 0.5f, scale,
                              game_draw_title_color_alpha(COLOR_WHITE, alpha));
}

void game_draw_title_menu()
{
    uint64_t now = sceKernelGetSystemTimeWide();
    float elapsed = 0.0f;
    if (g_title_anim_start != 0 && now > g_title_anim_start)
    {
        elapsed = (float)(now - g_title_anim_start) / 1000000.0f;
    }

    // Fundo animado: shader splash do Balatro (menu principal comeca em t=12).
    bg_splash_draw(12.0f + elapsed);

    float logo_p = game_draw_title_ease(elapsed / 0.55f);
    float card_p = game_draw_title_ease((elapsed - 0.30f) / 0.45f);
    float menu_p = game_draw_title_ease((elapsed - 0.50f) / 0.35f);

    // Logo do Balatro (asset original do archive). O centro vazio recebe o As de Espadas.
    if (tex_title_logo >= 0)
    {
        float logo_y = 20.0f + (1.0f - logo_p) * 10.0f;
        graphics_set_texture(tex_title_logo, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_quad(93.0f, logo_y, 294.0f, 191.0f,
                           0, 0, 333, 216,
                           game_draw_title_color_alpha(COLOR_WHITE, logo_p));
    }
    else
    {
        graphics_draw_text_center(font_big, "BALATRO", SCREEN_WIDTH * 0.5f, 105.0f, 3.0f,
                                  game_draw_title_color_alpha(COLOR_WHITE, logo_p));
    }

    // As de Espadas no centro do logo (mesmo card do menu original).
    if (card_p > 0.0f)
    {
        float card_scale = 0.90f + 0.10f * card_p;
        float card_w = 57.0f * card_scale;
        float card_h = 76.0f * card_scale;
        float card_x = SCREEN_WIDTH * 0.5f - card_w * 0.5f;
        float card_y = 114.0f - card_h * 0.5f;
        uint32_t card_color = game_draw_title_color_alpha(COLOR_WHITE, card_p);

        graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_quad(card_x, card_y, card_w, card_h,
                           71, 1, TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, card_color);

        graphics_set_texture(tex_deck2, GRAPHICS_TEXTURE_FILTER_LINEAR);
        graphics_draw_quad(card_x, card_y, card_w, card_h,
                           1 + (TEXTURE_CARD_WIDTH + 2) * (CARD_RANK_ACE - 7),
                           1 + (TEXTURE_CARD_HEIGHT + 2) * CARD_SUIT_SPADES,
                           TEXTURE_CARD_WIDTH, TEXTURE_CARD_HEIGHT, card_color);
    }

    if (menu_p > 0.0f)
    {
        bool highlighted_visible = (g_game_state.highlighted_item != TITLE_MENU_ITEM_CONTINUE) || save_autosave_exists();
        uint32_t panel_fill = game_draw_title_color_alpha(TITLE_COLOR_PANEL_FILL, menu_p);
        uint32_t panel_border = game_draw_title_color_alpha(TITLE_COLOR_PANEL_BORDER, menu_p);

        g_panel_shadow = menu_p >= 0.999f;
        game_draw_rounded_box(72.0f, 216.0f, 336.0f, 40.0f, 6.0f, panel_fill, panel_border, 2.0f);

        game_draw_title_button(80.0f, 222.0f, 88.0f, 28.0f, "PLAY", TITLE_COLOR_BLUE,
                               g_game_state.highlighted_item == TITLE_MENU_ITEM_PLAY, menu_p);
        game_draw_title_button(174.0f, 222.0f, 74.0f, 28.0f, "OPTIONS", TITLE_COLOR_ORANGE,
                               g_game_state.highlighted_item == TITLE_MENU_ITEM_OPTIONS, menu_p);
        game_draw_title_button(254.0f, 222.0f, 56.0f, 28.0f, "QUIT", TITLE_COLOR_RED,
                               g_game_state.highlighted_item == TITLE_MENU_ITEM_QUIT, menu_p);
        game_draw_title_button(316.0f, 222.0f, 84.0f, 28.0f, "CONTINUE", TITLE_COLOR_GREEN,
                               highlighted_visible && g_game_state.highlighted_item == TITLE_MENU_ITEM_CONTINUE, menu_p);

        // Botao esquerdo: load manual (duas linhas).
        uint32_t load_fill = game_draw_title_color_alpha(TITLE_COLOR_DARK, menu_p);
        uint32_t load_border = (g_game_state.highlighted_item == TITLE_MENU_ITEM_LOAD)
                                   ? game_draw_title_color_alpha(COLOR_WHITE, menu_p)
                                   : load_fill;
        game_draw_rounded_box(16.0f, 218.0f, 52.0f, 34.0f, 4.0f, load_fill, load_border, 2.0f);
        graphics_draw_text_center(font_small, "MANUAL", 42.0f, 228.0f, 1.0f,
                                  game_draw_title_color_alpha(COLOR_WHITE, menu_p));
        graphics_draw_text_center(font_small, "LOAD", 42.0f, 240.0f, 1.0f,
                                  game_draw_title_color_alpha(COLOR_WHITE, menu_p));

        // Pilula de versao, a direita.
        uint32_t version_fill = game_draw_title_color_alpha(TITLE_COLOR_DARK, menu_p);
        game_draw_rounded_box(412.0f, 222.0f, 46.0f, 28.0f, 4.0f, version_fill, version_fill, 2.0f);
        graphics_draw_text_center(font_small, "1.1.4", 435.0f, 236.0f, 1.0f,
                                  game_draw_title_color_alpha(COLOR_WHITE, menu_p));
        g_panel_shadow = false;
    }

    if (g_settings.audio && !audio_sfx_loader_done())
    {
        graphics_draw_text_center(font_small, "loading sounds...", SCREEN_WIDTH * 0.5f, 262.0f, 1.0f,
                                  game_draw_title_color_alpha(COLOR_LIGHT_GREY, menu_p));
    }
    else
    {
        graphics_draw_text_center(font_small, "X SELECT   O EXIT", SCREEN_WIDTH * 0.5f, 262.0f, 1.0f,
                                  game_draw_title_color_alpha(COLOR_LIGHT_GREY, menu_p));
    }
}

static void game_draw_deck_back_preview(int deck_type, float x, float y, float w, float h, uint32_t color)
{
    int u = g_deck_types[deck_type].u;
    int v = g_deck_types[deck_type].v;
    if (!profile_is_deck_unlocked(deck_type))
    {
        u = 4;
        v = 0;
    }

    graphics_set_texture(tex_enhancers, GRAPHICS_TEXTURE_FILTER_LINEAR);
    graphics_draw_quad(
        x, y, w, h,
        1 + u * (TEXTURE_CARD_WIDTH + 2),
        1 + v * (TEXTURE_CARD_HEIGHT + 2),
        TEXTURE_CARD_WIDTH,
        TEXTURE_CARD_HEIGHT,
        color);
}

static void game_get_deck_progress_text(int deck_type, char *dst)
{
    int discovered = profile_get_discovered_items_count();

    switch (deck_type)
    {
        case DECK_TYPE_RED:
            strcpy(dst, "Unlocked");
            break;
        case DECK_TYPE_BLUE:
            sprintf(dst, "Items %d/20", MIN(discovered, 20));
            break;
        case DECK_TYPE_YELLOW:
            sprintf(dst, "Items %d/50", MIN(discovered, 50));
            break;
        case DECK_TYPE_GREEN:
            sprintf(dst, "Items %d/75", MIN(discovered, 75));
            break;
        case DECK_TYPE_BLACK:
            sprintf(dst, "Items %d/100", MIN(discovered, 100));
            break;
        case DECK_TYPE_MAGIC:
            sprintf(dst, "Red win %d/1", (g_profile.won_decks & (1u << DECK_TYPE_RED)) ? 1 : 0);
            break;
        case DECK_TYPE_NEBULA:
            sprintf(dst, "Blue win %d/1", (g_profile.won_decks & (1u << DECK_TYPE_BLUE)) ? 1 : 0);
            break;
        case DECK_TYPE_GHOST:
            sprintf(dst, "Yellow win %d/1", (g_profile.won_decks & (1u << DECK_TYPE_YELLOW)) ? 1 : 0);
            break;
        case DECK_TYPE_ABANDONED:
            sprintf(dst, "Green win %d/1", (g_profile.won_decks & (1u << DECK_TYPE_GREEN)) ? 1 : 0);
            break;
        case DECK_TYPE_CHECKERED:
            sprintf(dst, "Black win %d/1", (g_profile.won_decks & (1u << DECK_TYPE_BLACK)) ? 1 : 0);
            break;
        case DECK_TYPE_ZODIAC:
            sprintf(dst, "Stake %d/1", MIN(g_profile.highest_stake_win, 1));
            break;
        case DECK_TYPE_PAINTED:
            sprintf(dst, "Stake %d/2", MIN(g_profile.highest_stake_win, 2));
            break;
        case DECK_TYPE_ANAGLYPH:
            sprintf(dst, "Stake %d/3", MIN(g_profile.highest_stake_win, 3));
            break;
        case DECK_TYPE_PLASMA:
            sprintf(dst, "Stake %d/4", MIN(g_profile.highest_stake_win, 4));
            break;
        case DECK_TYPE_ERRATIC:
            sprintf(dst, "Stake %d/5", MIN(g_profile.highest_stake_win, 5));
            break;
        default:
            strcpy(dst, "");
            break;
    }
}

void game_draw_deck_select()
{
    graphics_clear(COLOR_BACKGROUND);

    graphics_draw_solid_quad(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BACKGROUND_2);
    graphics_draw_text_center(font_big, "Choose Deck", SCREEN_WIDTH / 2.0f, 10.0f, 1.0f, COLOR_WHITE);
    char summary[32];
    sprintf(summary, "Items %d", profile_get_discovered_items_count());
    graphics_draw_text(font_small, summary, 390.0f, 12.0f, 1.0f, COLOR_LIGHT_GREY);

    int selected = CLAMP(g_game_state.highlighted_item, 0, DECK_TYPE_COUNT - 1);
    for (int i = 0; i < DECK_TYPE_COUNT; i++)
    {
        int col = i % 3;
        int row = i / 3;
        float x = 24.0f + col * 144.0f;
        float y = 34.0f + row * 30.0f;
        bool unlocked = profile_is_deck_unlocked(i);
        uint32_t box_color = (selected == i) ? COLOR_DARK_GREY : COLOR_DARK_GREY_2;
        uint32_t text_color = unlocked ? COLOR_WHITE : COLOR_LIGHT_GREY;

        graphics_draw_solid_quad(x, y, 128.0f, 24.0f, box_color);
        if (selected == i)
        {
            graphics_draw_solid_quad(x - 2.0f, y - 2.0f, 132.0f, 2.0f, COLOR_TEXT_YELLOW);
            graphics_draw_solid_quad(x - 2.0f, y + 24.0f, 132.0f, 2.0f, COLOR_TEXT_YELLOW);
            graphics_draw_solid_quad(x - 2.0f, y, 2.0f, 24.0f, COLOR_TEXT_YELLOW);
            graphics_draw_solid_quad(x + 128.0f, y, 2.0f, 24.0f, COLOR_TEXT_YELLOW);
        }
        game_draw_deck_back_preview(i, x + 5.0f, y + 3.0f, 12.0f, 16.0f, unlocked ? COLOR_WHITE : COLOR_WHITE);
        graphics_draw_text_center(font_small, g_deck_types[i].name, x + 64.0f, y + 5.0f, 1.0f, text_color);
        if (!unlocked)
        {
            graphics_draw_text_center(font_small, "Locked", x + 64.0f, y + 15.0f, 1.0f, COLOR_LIGHT_RED);
        }
    }

    float info_x = 24.0f;
    float info_y = 194.0f;
    bool unlocked = profile_is_deck_unlocked(selected);
    graphics_draw_solid_quad(info_x, info_y, SCREEN_WIDTH - 48.0f, 56.0f, COLOR_DARK_GREY_2);
    game_draw_deck_back_preview(selected, info_x + 8.0f, info_y + 5.0f, 34.0f, 46.0f, COLOR_WHITE);

    char progress[32];
    game_get_deck_progress_text(selected, progress);

    graphics_draw_text(font_small, g_deck_types[selected].name, info_x + 50.0f, info_y + 6.0f, 1.0f, unlocked ? COLOR_TEXT_YELLOW : COLOR_LIGHT_GREY);
    for (int i = 0; i < 3; i++)
    {
        if (g_deck_types[selected].effect_lines[i][0] != '\0')
        {
            graphics_draw_text(font_small, g_deck_types[selected].effect_lines[i], info_x + 50.0f, info_y + 20.0f + i * 10.0f, 1.0f, COLOR_WHITE);
        }
    }

    graphics_draw_text(font_small, progress, info_x + 238.0f, info_y + 8.0f, 1.0f, unlocked ? COLOR_LIGHT_BLUE : COLOR_LIGHT_RED);
    graphics_draw_text(font_small, unlocked ? "X Start  O Back" : g_deck_types[selected].unlock_line, info_x + 238.0f, info_y + 30.0f, 1.0f, unlocked ? COLOR_LIGHT_GREY : COLOR_LIGHT_RED);
    if (g_settings.debug_tools)
    {
        graphics_draw_text(font_small, "DBG L+R+X tier  L+R+O reset", 186.0f, 254.0f, 1.0f, COLOR_LIGHT_GREY);
    }
}

void game_draw()
{
    graphics_begin_draw();

    graphics_clear(COLOR_BACKGROUND_2);

    if (g_game_state.stage == GAME_STAGE_TITLE)
    {
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_DECK_SELECT)
        {
            game_draw_deck_select();
        }
        else
        {
            game_draw_title_menu();
        }
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SETTINGS_MENU)
        {
            game_draw_settings_menu();
        }
        game_draw_debug_info();
        graphics_end_draw();
        return;
    }

    int draw_stage = (g_game_state.stage == GAME_STAGE_PAUSE_MENU) ? g_game_state.previous_stage : g_game_state.stage;

    if (draw_stage == GAME_STAGE_INGAME)
    {
        bg_shader_draw();
    }

    switch (draw_stage)
    {
        case GAME_STAGE_BLINDS:
            game_draw_blind_select();
            break;
        case GAME_STAGE_INGAME:
            game_draw_ingame();
            break;
        case GAME_STAGE_SHOP:
        {
            game_draw_shop();
            break;
        }
    }

    if (g_game_state.sub_stage == GAME_SUBSTAGE_DECK_INFO)
    {
        game_draw_deck_info();
    }

    if (g_game_state.sub_stage == GAME_SUBSTAGE_RUN_INFO)
    {
        game_draw_run_info();
    }

    if (g_game_state.stage == GAME_STAGE_PAUSE_MENU)
    {
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_SETTINGS_MENU)
            game_draw_settings_menu();
        else
            game_draw_pause_menu();
    }

    game_draw_debug_info();

    graphics_end_draw();
}
