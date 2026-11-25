#include "debug.c"
#include "global.h"

PSP_MODULE_INFO("PSPalatro", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#include "callbacks.c"
#include "archive.c"
#include "ini.c"
#include "audio.c"
#include "graphics.c"
#include "input.c"
#include "bg_shader.c"
#include "draw.c"
#include "game_util.c"
#include "entity_event.c"
#include "automated_events.c"
#include "game_input.c"
#include "save_utility.c"
#include "game.c"

struct DebugInfo g_debug_info;

int ogg_id = -1;

// A libcglue chama sceUtilityGetSystemParam* dentro de __timezone_update.
// Isso puxa stubs de sceUtility em dois blocos separados e o psp-fixup-imports
// nao consegue montar a tabela de imports (no PSP real isso corrompe stubs
// vizinhos, inclusive sceCtrl, e o jogo fica sem input). O jogo nao usa fuso
// horario, entao neutralizamos a funcao.
void __timezone_update(void) { }

bool init_more_stuff()
{
    if (!game_init_load_file_values()) return false;
    profile_load();

    if (g_settings.overclock)
    {
        game_draw_loading_text("Setting CPU clock to 333Mhz", COLOR_WHITE, COLOR_BLACK);
        scePowerSetClockFrequency(333, 333, 166);
    }
    else
    {
        // Já subimos para 333MHz no início do main(); volta ao clock padrão
        // se o usuário desativou o overclock, senão o boot rápido deixaria
        // o jogo preso em 333MHz.
        scePowerSetClockFrequency(222, 222, 111);
    }
    
    if (!archive_open(g_settings.archive_file_name)) return false;

    game_init_logic();    

    if (!game_init_draw()) return false;
    
    if (g_settings.audio)
    {
        game_draw_loading_text("Loading audio file", COLOR_WHITE, COLOR_BLACK);
        ogg_id = audio_load_ogg_from_archive("resources/sounds/music1.ogg");
        if (ogg_id < 0)
        {
            game_draw_loading_text("Error loading file \"resources/sounds/music1.ogg\" from archive", COLOR_TEXT_RED, COLOR_BLACK);
            return false;
        }
        audio_play_ogg(ogg_id, 0.8f);
    }

    archive_close();

    // SFX em background (cache PCM ou decode OGG na thread): o titulo
    // aparece na hora em vez de travar o boot no decode Vorbis.
    audio_start_sfx_loader();

    event_init();
    game_show_title_menu();

    return true;
}

int main(int argc, char **argv)
{
    DEBUG_PRINTF("PSPalatro");

    // Otimização: sobe para 333MHz antes de qualquer init/loading (fontes,
    // archive scan, decode PNG/Vorbis). O valor final do settings.ini é
    // reaplicado em init_more_stuff().
    scePowerSetClockFrequency(333, 333, 166);

    setup_callbacks();

    sceKernelDcacheWritebackAll();

    audio_init();

    graphics_init();

    input_init();

    game_init_draw_load_fonts();

    if (init_more_stuff())
    {
        while(running())
        {
            if (g_settings.audio) audio_update();
            input_update();
            game_update();
            game_draw();
        }
    }
    else
    {
        while(running())
        {
            graphics_begin_draw();
            graphics_end_draw();
        }
    }

    if (g_settings.audio)
    {
        audio_stop();
        audio_destroy_ogg(ogg_id);
        audio_destroy_sfx();
    }

    audio_end();    

    sceGuTerm();
	sceKernelExitGame();

    return 0;
}
