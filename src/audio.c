#include "global.h"

#include <vorbis/vorbisfile.h>

#define AUDIO_BUFFER_CHUNKS  8
#define AUDIO_BUFFER_SIZE   4096
#define AUDIO_VOLUME_SHIFT  8
#define AUDIO_VOLUME_ONE    (1 << AUDIO_VOLUME_SHIFT)

struct audio_buffer_data
{
    char data[AUDIO_BUFFER_SIZE];    
};

struct audio_buffer
{
    struct audio_buffer_data chunks[AUDIO_BUFFER_CHUNKS];
    int read_pos;
    int write_pos;
    int written;

    int ogg_id;

    float speed;
    int src_buffer_size;
};

volatile struct audio_buffer g_audio_buffer;

#define MAX_OGG_FILES   10
#define AUDIO_SFX_CHANNELS 6

struct OggFile
{
    bool in_use;
    char* cur_ptr;
    char* file_ptr;
    size_t file_size;

    OggVorbis_File vorbis_file;
} g_ogg_files[MAX_OGG_FILES];

struct sample_t
{
    short l, r;
};

struct AudioSfx
{
    bool in_use;
    struct sample_t *samples;
    int sample_count;
    int volume;
};

struct AudioSfxChannel
{
    bool active;
    int sfx_id;
    int position;
};

struct AudioFlameChannel
{
    int position;
    int volume;
    int target_volume;
};

struct AudioSfx g_audio_sfx[AUDIO_SFX_COUNT];
volatile struct AudioSfxChannel g_audio_sfx_channels[AUDIO_SFX_CHANNELS];
volatile struct AudioFlameChannel g_audio_flame_channel;

// Tabela unica dos SFX (boot + thread + cache usam a mesma).
// Flame encurtado 4.5s -> 2.5s: e ruido em loop, nao se percebe o corte,
// e e o maior decode do boot + ~350KB a menos de RAM.
struct SfxLoadEntry
{
    int id;
    const char *file;
    float volume;
    float seconds;
};
static const struct SfxLoadEntry s_sfx_table[] =
{
    { AUDIO_SFX_BUTTON,    "resources/sounds/button.ogg",     0.55f, 2.0f },
    { AUDIO_SFX_CANCEL,    "resources/sounds/cancel.ogg",     0.55f, 2.0f },
    { AUDIO_SFX_HIGHLIGHT, "resources/sounds/highlight1.ogg", 0.45f, 2.0f },
    { AUDIO_SFX_CARD,      "resources/sounds/card1.ogg",      0.65f, 2.0f },
    { AUDIO_SFX_CHIPS,     "resources/sounds/chips1.ogg",     0.60f, 2.0f },
    { AUDIO_SFX_MULT,      "resources/sounds/multhit1.ogg",   0.60f, 2.0f },
    { AUDIO_SFX_COIN,      "resources/sounds/coin3.ogg",      0.70f, 2.0f },
    { AUDIO_SFX_TAROT,     "resources/sounds/tarot1.ogg",     0.65f, 2.0f },
    { AUDIO_SFX_WHOOSH,    "resources/sounds/whoosh1.ogg",    0.55f, 2.0f },
    { AUDIO_SFX_WIN,       "resources/sounds/win.ogg",        0.75f, 3.0f },
    { AUDIO_SFX_FLAME,     "resources/sounds/ambientFire2.ogg", 1.75f, 2.5f },
};
#define SFX_TABLE_COUNT (sizeof(s_sfx_table) / sizeof(s_sfx_table[0]))

static short audio_clamp_sample(int sample)
{
    if (sample > 32767) return 32767;
    if (sample < -32768) return -32768;
    return sample;
}

static int audio_volume_from_float(float volume)
{
    if (volume <= 0.0f) return 0;
    return (int)(volume * (float)AUDIO_VOLUME_ONE + 0.5f);
}

static int audio_get_music_volume()
{
    return CLAMP(g_settings.music_volume, 0, 10) * AUDIO_VOLUME_ONE / 10;
}

static int audio_get_sfx_volume()
{
    return CLAMP(g_settings.sfx_volume, 0, 10) * AUDIO_VOLUME_ONE / 10;
}

// Publicacao segura do SFX: preenche tudo e so entao marca in_use.
// A barreira impede o compilador de reordenar (o callback de audio roda
// em outro contexto e testa in_use primeiro). Mesma CPU, sem reorder de HW.
static void audio_sfx_install(int sfx_id, struct sample_t *samples, long sample_count, float volume)
{
    if (sfx_id < 0 || sfx_id >= AUDIO_SFX_COUNT) return;
    if (g_audio_sfx[sfx_id].in_use)
    {
        free(g_audio_sfx[sfx_id].samples);
    }
    g_audio_sfx[sfx_id].samples = samples;
    g_audio_sfx[sfx_id].sample_count = (int)sample_count;
    g_audio_sfx[sfx_id].volume = audio_volume_from_float(volume);
    __asm__ volatile ("" ::: "memory");
    g_audio_sfx[sfx_id].in_use = true;
}

void audio_callback(void* buf, unsigned int length, void *userdata)
{    
    unsigned int buffer_size = length * sizeof(struct sample_t);
    struct sample_t *out = (struct sample_t *)buf;
    int sample_count = length;

    if (!g_settings.audio)
    {
        memset(buf, 0, buffer_size);
        return;
    }

    if (g_audio_buffer.ogg_id > -1 && !g_debug_info.force_score_flames)
    {
        if (g_audio_buffer.written > 0)
        {
            unsigned int copy_size = buffer_size < AUDIO_BUFFER_SIZE ? buffer_size : AUDIO_BUFFER_SIZE;
            memcpy((void*)buf, (void*)(g_audio_buffer.chunks[g_audio_buffer.read_pos].data), copy_size);
            if (buffer_size > copy_size)
            {
                memset((uint8_t*)buf + copy_size, 0, buffer_size - copy_size);
            }
            g_audio_buffer.read_pos = (g_audio_buffer.read_pos + 1) % AUDIO_BUFFER_CHUNKS;
            g_audio_buffer.written--;
        }
        else
        {
            g_debug_info.audio_wait_read++;
            memset(buf, 0, buffer_size);
        }
    }
    else
    {
        memset(buf, 0, buffer_size);
    }

    int music_volume = audio_get_music_volume();
    if (music_volume < AUDIO_VOLUME_ONE)
    {
        for (int i = 0; i < sample_count; i++)
        {
            out[i].l = (short)((out[i].l * music_volume) >> AUDIO_VOLUME_SHIFT);
            out[i].r = (short)((out[i].r * music_volume) >> AUDIO_VOLUME_SHIFT);
        }
    }

    int sfx_master_volume = audio_get_sfx_volume();
    for (int channel_index = 0; channel_index < AUDIO_SFX_CHANNELS; channel_index++)
    {
        volatile struct AudioSfxChannel *channel = &g_audio_sfx_channels[channel_index];
        if (!channel->active) continue;
        if (channel->sfx_id < 0 || channel->sfx_id >= AUDIO_SFX_COUNT)
        {
            channel->active = false;
            continue;
        }

        struct AudioSfx *sfx = &g_audio_sfx[channel->sfx_id];
        if (!sfx->in_use)
        {
            channel->active = false;
            continue;
        }

        int volume = (sfx->volume * sfx_master_volume) >> AUDIO_VOLUME_SHIFT;
        if (volume <= 0)
        {
            continue;
        }

        for (int i = 0; i < sample_count; i++)
        {
            if (channel->position >= sfx->sample_count)
            {
                channel->active = false;
                break;
            }

            struct sample_t sample = sfx->samples[channel->position++];
            out[i].l = audio_clamp_sample(out[i].l + ((sample.l * volume) >> AUDIO_VOLUME_SHIFT));
            out[i].r = audio_clamp_sample(out[i].r + ((sample.r * volume) >> AUDIO_VOLUME_SHIFT));
        }
    }

    struct AudioSfx *flame_sfx = &g_audio_sfx[AUDIO_SFX_FLAME];
    if (flame_sfx->in_use && flame_sfx->sample_count > 0 &&
        (g_audio_flame_channel.volume > 0 || g_audio_flame_channel.target_volume > 0))
    {
        int volume = g_audio_flame_channel.volume;
        int target = g_audio_flame_channel.target_volume;
        volume += ((target - volume) * 46) >> AUDIO_VOLUME_SHIFT;
        int position = g_audio_flame_channel.position;
        int volume_fixed = (flame_sfx->volume * volume * sfx_master_volume) >> (AUDIO_VOLUME_SHIFT * 2);

        if (volume_fixed > 0)
        {
            for (int i = 0; i < sample_count; i++)
            {
                if (position >= flame_sfx->sample_count) position = 0;

                struct sample_t sample = flame_sfx->samples[position++];
                out[i].l = audio_clamp_sample(out[i].l + ((sample.l * volume_fixed) >> AUDIO_VOLUME_SHIFT));
                out[i].r = audio_clamp_sample(out[i].r + ((sample.r * volume_fixed) >> AUDIO_VOLUME_SHIFT));
            }
        }

        g_audio_flame_channel.volume = volume;
        g_audio_flame_channel.position = position;
    }
}

void audio_init()
{
    pspAudioInit();
    pspAudioSetChannelCallback(0, audio_callback, NULL);

    g_audio_buffer.ogg_id = -1;

    g_debug_info.audio_wait_read = 0;
    g_debug_info.audio_wait_write = 0;

    for (int i = 0; i < AUDIO_SFX_COUNT; i++)
    {
        g_audio_sfx[i].in_use = false;
        g_audio_sfx[i].samples = NULL;
        g_audio_sfx[i].sample_count = 0;
        g_audio_sfx[i].volume = AUDIO_VOLUME_ONE;
    }

    for (int i = 0; i < AUDIO_SFX_CHANNELS; i++)
    {
        g_audio_sfx_channels[i].active = false;
    }

    g_audio_flame_channel.position = 0;
    g_audio_flame_channel.volume = 0;
    g_audio_flame_channel.target_volume = 0;
}
char temp_buffer[AUDIO_BUFFER_SIZE];

static int audio_get_free_ogg_slot()
{
    for (int i = 0; i < MAX_OGG_FILES; i++)
    {
        if (!g_ogg_files[i].in_use) return i;
    }

    return -1;
}

void audio_update()
{
    if (g_audio_buffer.ogg_id > -1 && g_audio_buffer.written < AUDIO_BUFFER_CHUNKS)
    {
        int current_section;
        long ret = 0, total = 0;
        int eof_count = 0;
        char *buf_ptr = temp_buffer;
        while (total < g_audio_buffer.src_buffer_size)
        {
            ret = ov_read(&(g_ogg_files[g_audio_buffer.ogg_id].vorbis_file),buf_ptr,g_audio_buffer.src_buffer_size - total,0,2,1,&current_section);
            if (ret > 0)
            {
                total += ret;
                buf_ptr += ret;
                eof_count = 0;
                continue;
            }

            if (ret == 0)
            {
                // Fim do arquivo: tenta voltar ao inicio. Se o seek falhar,
                // ou se o arquivo nao tiver samples, preenche o resto com
                // silencio para nao travar a thread principal num loop.
                eof_count++;
                if (eof_count > 1 ||
                    ov_pcm_seek(&(g_ogg_files[g_audio_buffer.ogg_id].vorbis_file), 0) != 0)
                {
                    memset(buf_ptr, 0, g_audio_buffer.src_buffer_size - total);
                    total = g_audio_buffer.src_buffer_size;
                    break;
                }
                continue;
            }

            // Erro de decodificacao (ret < 0): nao insistir; silencio.
            memset(buf_ptr, 0, g_audio_buffer.src_buffer_size - total);
            total = g_audio_buffer.src_buffer_size;
            break;
        }

        struct sample_t *dst = (struct sample_t *)g_audio_buffer.chunks[g_audio_buffer.write_pos].data;
        struct sample_t *src = (struct sample_t *)temp_buffer;
        float i_src = 0.0f;
        float inc = g_audio_buffer.speed;

        int size = (AUDIO_BUFFER_SIZE / 4);

        int index/*, index2*/ = 0;
        for (int i = 0; i < size; i++)
        {
            // index = floor(i_src);
            // index2 = index + 1;
            // if (index2 >= size)
            // {
            //     dst[i].l = src[index].l;
            //     dst[i].r = src[index].r;
            // }
            // else
            // {
            //     float f = i_src - (float)index;
            //     dst[i].l = src[index].l * (1.0f - f) + src[index2].l * f;
            //     dst[i].r = src[index].r * (1.0f - f) + src[index2].r * f;
            // }

            index = roundf(i_src);
            if (index < 0) index = 0;
            if (index >= size) index = size - 1;
            dst[i].l = src[index].l;
            dst[i].r = src[index].r;

            i_src += inc;
        }

        g_audio_buffer.write_pos = (g_audio_buffer.write_pos + 1) % AUDIO_BUFFER_CHUNKS;
        g_audio_buffer.written++;
    }
    else
    {
        g_debug_info.audio_wait_write++;
    }
}

void audio_end()
{
    pspAudioSetChannelCallback(0, NULL, NULL);
}

void audio_resume()
{
pspAudioSetChannelCallback(0, audio_callback, NULL);
}

size_t audio_ogg_callback_read_ogg(void* dst, size_t size1, size_t size2, void* fh)
{
    struct OggFile* of = (struct OggFile*)(fh);
    size_t len = size1 * size2;
    if ( of->cur_ptr + len > of->file_ptr + of->file_size )
    {
        len = of->file_ptr + of->file_size - of->cur_ptr;
    }
    memcpy( dst, of->cur_ptr, len );
    of->cur_ptr += len;
    return len;
}

int audio_ogg_callback_seek_ogg( void *fh, ogg_int64_t to, int type ) {
    struct OggFile* of = (struct OggFile*)(fh);

    switch( type ) {
        case SEEK_CUR:
            of->cur_ptr += to;
            break;
        case SEEK_END:
            of->cur_ptr = of->file_ptr + of->file_size - to;
            break;
        case SEEK_SET:
            of->cur_ptr = of->file_ptr + to;
            break;
        default:
            return -1;
    }
    if ( of->cur_ptr < of->file_ptr ) {
        of->cur_ptr = of->file_ptr;
        return -1;
    }
    if ( of->cur_ptr > of->file_ptr + of->file_size ) {
        of->cur_ptr = of->file_ptr + of->file_size;
        return -1;
    }
    return 0;
}

int audio_ogg_callback_close_ogg(void* fh)
{
    return 0;
}

long audio_ogg_callback_tell_ogg( void *fh )
{
    struct OggFile* of = (struct OggFile*)(fh);
    return (of->cur_ptr - of->file_ptr);
}

int audio_load_ogg(char *filename)
{
    int ogg_id = audio_get_free_ogg_slot();
    if (ogg_id < 0) return -1;

    FILE *fp_ogg = fopen(filename, "rb");
    if (!fp_ogg) return -1;

    fseek(fp_ogg, 0, SEEK_END);
    long fsize = ftell(fp_ogg);
    fseek(fp_ogg, 0, SEEK_SET);
    g_ogg_files[ogg_id].file_ptr = malloc(fsize + 1);
    if (!g_ogg_files[ogg_id].file_ptr)
    {
        fclose(fp_ogg);
        return -1;
    }
    fread(g_ogg_files[ogg_id].file_ptr, fsize, 1, fp_ogg);
    fclose(fp_ogg);

    ov_callbacks callbacks;
    
    g_ogg_files[ogg_id].cur_ptr = g_ogg_files[ogg_id].file_ptr;
    g_ogg_files[ogg_id].file_size = fsize;
    g_ogg_files[ogg_id].in_use = true;

    callbacks.read_func = audio_ogg_callback_read_ogg;
    callbacks.seek_func = audio_ogg_callback_seek_ogg;
    callbacks.close_func = audio_ogg_callback_close_ogg;
    callbacks.tell_func = audio_ogg_callback_tell_ogg;

    if (ov_open_callbacks((void *)&(g_ogg_files[ogg_id]), &(g_ogg_files[ogg_id].vorbis_file), NULL, -1, callbacks) < 0)
    {
        free(g_ogg_files[ogg_id].file_ptr);
        g_ogg_files[ogg_id].in_use = false;
        return -1;
    }

#ifdef DEBUG    
    {
        DEBUG_PRINTF("Ogg file \"%s\" loaded.\n", filename);

        char **ptr=ov_comment(&(g_ogg_files[ogg_id].vorbis_file),-1)->user_comments;
        vorbis_info *vi=ov_info(&(g_ogg_files[ogg_id].vorbis_file),-1);
        while(*ptr){
          DEBUG_PRINTF("\t%s\n",*ptr);
          ++ptr;
        }
        DEBUG_PRINTF("\n\tBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
        DEBUG_PRINTF("\n\tDecoded length: %ld samples\n", (long)ov_pcm_total(&(g_ogg_files[ogg_id].vorbis_file),-1));
        DEBUG_PRINTF("\tEncoded by: %s\n\n",ov_comment(&(g_ogg_files[ogg_id].vorbis_file),-1)->vendor);
    }
#endif

    return ogg_id;
}

int audio_load_ogg_from_archive(char *filename)
{
    int ogg_id = audio_get_free_ogg_slot();
    if (ogg_id < 0) return -1;

    size_t file_size = 0;
    g_ogg_files[ogg_id].file_ptr = (char *)archive_load_file_entry(filename, &file_size);
    if (!g_ogg_files[ogg_id].file_ptr) return -1;

    ov_callbacks callbacks;
    
    g_ogg_files[ogg_id].cur_ptr = g_ogg_files[ogg_id].file_ptr;
    g_ogg_files[ogg_id].file_size = file_size;
    g_ogg_files[ogg_id].in_use = true;

    callbacks.read_func = audio_ogg_callback_read_ogg;
    callbacks.seek_func = audio_ogg_callback_seek_ogg;
    callbacks.close_func = audio_ogg_callback_close_ogg;
    callbacks.tell_func = audio_ogg_callback_tell_ogg;

    if (ov_open_callbacks((void *)&(g_ogg_files[ogg_id]), &(g_ogg_files[ogg_id].vorbis_file), NULL, -1, callbacks) < 0)
    {
        free(g_ogg_files[ogg_id].file_ptr);
        g_ogg_files[ogg_id].in_use = false;
        return -1;
    }

#ifdef DEBUG    
    {
        DEBUG_PRINTF("Ogg file \"%s\" loaded.\n", filename);

        char **ptr=ov_comment(&(g_ogg_files[ogg_id].vorbis_file),-1)->user_comments;
        vorbis_info *vi=ov_info(&(g_ogg_files[ogg_id].vorbis_file),-1);
        while(*ptr){
          DEBUG_PRINTF("\t%s\n",*ptr);
          ++ptr;
        }
        DEBUG_PRINTF("\n\tBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
        DEBUG_PRINTF("\n\tDecoded length: %ld samples\n", (long)ov_pcm_total(&(g_ogg_files[ogg_id].vorbis_file),-1));
        DEBUG_PRINTF("\tEncoded by: %s\n\n",ov_comment(&(g_ogg_files[ogg_id].vorbis_file),-1)->vendor);
    }
#endif

    return ogg_id;
}

// Buffer maior so para o decode dos SFX no boot: menos chamadas ov_read
// (cada chamada tem custo fixo no libvorbis) e copia direta no caso stereo.
#define SFX_DECODE_BUF_SIZE 16384
static char s_sfx_decode_buf[SFX_DECODE_BUF_SIZE];

int audio_load_sfx_from_archive_limited(int sfx_id, char *filename, float volume, float seconds)
{
    if (sfx_id < 0 || sfx_id >= AUDIO_SFX_COUNT) return -1;

    int ogg_id = audio_load_ogg_from_archive(filename);
    if (ogg_id < 0) return -1;

    long total_pcm = (long)ov_pcm_total(&(g_ogg_files[ogg_id].vorbis_file), -1);
    vorbis_info *info = ov_info(&(g_ogg_files[ogg_id].vorbis_file), -1);
    if (total_pcm <= 0 || !info || info->channels <= 0)
    {
        audio_destroy_ogg(ogg_id);
        return -1;
    }

    long sample_limit = total_pcm;
    if (seconds > 0.0f && info->rate > 0)
    {
        sample_limit = MIN(total_pcm, (long)((float)info->rate * seconds));
    }

    struct sample_t *samples = malloc(sample_limit * sizeof(struct sample_t));
    if (!samples)
    {
        audio_destroy_ogg(ogg_id);
        return -1;
    }

    int current_section = 0;
    long total_samples = 0;
    int channels = info->channels;
    if (channels == 2)
    {
        // Caso comum: ov_read ja devolve LRLR intercalado, igual ao layout
        // de struct sample_t. memcpy em vez de loop por sample.
        while (total_samples < sample_limit)
        {
            long ret = ov_read(&(g_ogg_files[ogg_id].vorbis_file), s_sfx_decode_buf, SFX_DECODE_BUF_SIZE, 0, 2, 1, &current_section);
            if (ret <= 0) break;

            long frames = ret / ((long)sizeof(struct sample_t));
            if (frames <= 0) continue;
            if (total_samples + frames > sample_limit) frames = sample_limit - total_samples;
            memcpy(&samples[total_samples], s_sfx_decode_buf, (size_t)frames * sizeof(struct sample_t));
            total_samples += frames;
        }
    }
    else
    {
        // Mono (duplica L/R) ou >2 canais (pega os 2 primeiros).
        // Branch do canal fica fora do loop por sample.
        while (total_samples < sample_limit)
        {
            long ret = ov_read(&(g_ogg_files[ogg_id].vorbis_file), s_sfx_decode_buf, SFX_DECODE_BUF_SIZE, 0, 2, 1, &current_section);
            if (ret <= 0) break;

            short *decoded = (short *)s_sfx_decode_buf;
            long frames = ret / ((long)sizeof(short) * channels);
            if (frames <= 0) continue;
            if (total_samples + frames > sample_limit) frames = sample_limit - total_samples;
            if (channels == 1)
            {
                for (long i = 0; i < frames; i++)
                {
                    samples[total_samples].l = decoded[i];
                    samples[total_samples].r = decoded[i];
                    total_samples++;
                }
            }
            else
            {
                for (long i = 0; i < frames; i++)
                {
                    samples[total_samples].l = decoded[i * channels];
                    samples[total_samples].r = decoded[i * channels + 1];
                    total_samples++;
                }
            }
        }
    }

    audio_destroy_ogg(ogg_id);

    if (total_samples <= 0)
    {
        free(samples);
        return -1;
    }

    audio_sfx_install(sfx_id, samples, total_samples, volume);

    return sfx_id;
}

int audio_load_sfx_from_archive(int sfx_id, char *filename, float volume)
{
    return audio_load_sfx_from_archive_limited(sfx_id, filename, volume, 0.0f);
}

// ---- Cache PCM: o formato mais rapido de carregar ----
// Na primeira vez decodifica os OGG (lento, Vorbis em float no MIPS) e
// grava os PCM crus em saves/sfxcache.pcm. Nos boots seguintes e so
// fread + memcpy, sem decode nenhum (~10x mais rapido que OGG).
// Layout: magic[4]='SFXP', u32 version, u32 count,
//   por entrada: u8 id, u8 pad, u16 pad, u32 frames,
//   depois os dados: frames * sizeof(struct sample_t) (stereo s16).
#define SFX_CACHE_MAGIC 0x58504653u
#define SFX_CACHE_VERSION 2u
#define SFX_CACHE_FILE "saves/sfxcache.pcm"
#define SFX_CACHE_MAX_FRAMES (44100u * 6u)
#define SFX_CACHE_MAX_FILE (8u * 1024u * 1024u)

static bool audio_sfx_cache_save(void)
{
    for (unsigned i = 0; i < SFX_TABLE_COUNT; i++)
    {
        if (!g_audio_sfx[s_sfx_table[i].id].in_use) return false;
    }

    FILE *f = fopen(SFX_CACHE_FILE, "wb");
    if (!f) return false;
    static char cache_buf[32768];
    setvbuf(f, cache_buf, _IOFBF, sizeof(cache_buf));

    uint32_t magic = SFX_CACHE_MAGIC;
    uint32_t version = SFX_CACHE_VERSION;
    uint32_t count = (uint32_t)SFX_TABLE_COUNT;
    if (fwrite(&magic, 4, 1, f) != 1 || fwrite(&version, 4, 1, f) != 1 ||
        fwrite(&count, 4, 1, f) != 1)
    {
        fclose(f);
        return false;
    }
    for (unsigned i = 0; i < SFX_TABLE_COUNT; i++)
    {
        struct AudioSfx *sfx = &g_audio_sfx[s_sfx_table[i].id];
        uint8_t id = (uint8_t)s_sfx_table[i].id;
        uint8_t pad8 = 0;
        uint16_t pad16 = 0;
        uint32_t frames = (uint32_t)sfx->sample_count;
        if (fwrite(&id, 1, 1, f) != 1 || fwrite(&pad8, 1, 1, f) != 1 ||
            fwrite(&pad16, 2, 1, f) != 1 || fwrite(&frames, 4, 1, f) != 1)
        {
            fclose(f);
            return false;
        }
    }
    for (unsigned i = 0; i < SFX_TABLE_COUNT; i++)
    {
        struct AudioSfx *sfx = &g_audio_sfx[s_sfx_table[i].id];
        size_t bytes = (size_t)sfx->sample_count * sizeof(struct sample_t);
        if (fwrite(sfx->samples, 1, bytes, f) != bytes)
        {
            fclose(f);
            return false;
        }
    }
    fclose(f);
    return true;
}

static bool audio_sfx_cache_load(void)
{
    FILE *f = fopen(SFX_CACHE_FILE, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (file_size <= 0 || file_size > (long)SFX_CACHE_MAX_FILE)
    {
        fclose(f);
        return false;
    }

    uint32_t magic = 0, version = 0, count = 0;
    if (fread(&magic, 4, 1, f) != 1 || fread(&version, 4, 1, f) != 1 ||
        fread(&count, 4, 1, f) != 1)
    {
        fclose(f);
        return false;
    }
    if (magic != SFX_CACHE_MAGIC || version != SFX_CACHE_VERSION ||
        count != (uint32_t)SFX_TABLE_COUNT)
    {
        fclose(f);
        return false;
    }

    // Le o TOC e valida tudo antes de instalar qualquer sample.
    uint8_t ids[32];
    uint32_t frames[32];
    long data_off = 12 + (long)count * 8;
    long off = data_off;
    for (uint32_t i = 0; i < count; i++)
    {
        uint8_t id = 0, pad8 = 0;
        uint16_t pad16 = 0;
        uint32_t fr = 0;
        (void)pad8; (void)pad16;
        if (fread(&id, 1, 1, f) != 1 || fread(&pad8, 1, 1, f) != 1 ||
            fread(&pad16, 2, 1, f) != 1 || fread(&fr, 4, 1, f) != 1)
        {
            fclose(f);
            return false;
        }
        if (id >= AUDIO_SFX_COUNT || fr == 0 || fr > SFX_CACHE_MAX_FRAMES)
        {
            fclose(f);
            return false;
        }
        off += (long)fr * (long)sizeof(struct sample_t);
        if (off > file_size)
        {
            fclose(f);
            return false;
        }
        ids[i] = id;
        frames[i] = fr;
    }

    // TOC ok: copia os dados (fread direto no buffer final, sem temporario).
    static char cache_in[32768];
    setvbuf(f, cache_in, _IOFBF, sizeof(cache_in));
    for (uint32_t i = 0; i < count; i++)
    {
        size_t bytes = (size_t)frames[i] * sizeof(struct sample_t);
        struct sample_t *samples = (struct sample_t *)malloc(bytes);
        if (!samples)
        {
            fclose(f);
            return false;
        }
        if (fread(samples, 1, bytes, f) != bytes)
        {
            free(samples);
            fclose(f);
            return false;
        }
        float vol = 1.0f;
        for (unsigned k = 0; k < SFX_TABLE_COUNT; k++)
        {
            if (s_sfx_table[k].id == ids[i]) { vol = s_sfx_table[k].volume; break; }
        }
        audio_sfx_install(ids[i], samples, (long)frames[i], vol);
    }
    fclose(f);
    return true;
}

static void audio_load_all_sfx_from_ogg(void)
{
    for (unsigned i = 0; i < SFX_TABLE_COUNT; i++)
    {
        audio_load_sfx_from_archive_limited(s_sfx_table[i].id, (char *)s_sfx_table[i].file,
                                            s_sfx_table[i].volume, s_sfx_table[i].seconds);
    }
}

static void audio_load_all_sfx_sync(void)
{
    if (audio_sfx_cache_load()) return;
    audio_load_all_sfx_from_ogg();
    audio_sfx_cache_save();
}

// ---- Loading multithread ----
// O PSP tem 1 nucleo com FPU (o Media Engine nao tem FPU e nao roda Vorbis),
// entao 2 threads nao decodificam mais rapido. O ganho aqui e outro: a thread
// carrega os SFX em paralelo com o jogo (titulo/menus), em prioridade baixa,
// em vez de travar o boot. Sons ainda nao carregados sao ignorados pelo
// audio_play_sfx (checa in_use), entao e seguro.
static SceUID s_sfx_loader_thid = -1;
static volatile bool s_sfx_loader_done = false;

static int audio_sfx_loader_thread(SceSize args, void *argp)
{
    (void)args;
    (void)argp;
    if (audio_sfx_cache_load())
    {
        s_sfx_loader_done = true;
        return 0;
    }
    // Sem cache: decode OGG com handle proprio do zip (o da main ja fechou).
    // Versao quiet: nada de draw/GU fora da thread principal.
    if (archive_open_quiet(g_settings.archive_file_name))
    {
        audio_load_all_sfx_from_ogg();
        archive_close();
        audio_sfx_cache_save();
    }
    s_sfx_loader_done = true;
    return 0;
}

void audio_start_sfx_loader(void)
{
    if (!g_settings.audio)
    {
        s_sfx_loader_done = true;
        return;
    }
    s_sfx_loader_done = false;
    s_sfx_loader_thid = sceKernelCreateThread("sfx_loader", audio_sfx_loader_thread,
                                              64, 64 * 1024, 0, NULL);
    if (s_sfx_loader_thid < 0)
    {
        s_sfx_loader_thid = -1;
        audio_load_all_sfx_sync();
        s_sfx_loader_done = true;
        return;
    }
    sceKernelStartThread(s_sfx_loader_thid, 0, NULL);
}

bool audio_sfx_loader_done(void)
{
    return s_sfx_loader_done;
}

void audio_play_ogg(int ogg_id, float speed)
{
    g_audio_buffer.ogg_id = ogg_id;
    g_audio_buffer.read_pos = 0;
    g_audio_buffer.write_pos = 0;
    g_audio_buffer.written = 0;
    g_audio_buffer.speed = (speed > 1.0f ? 1.0f : (speed <= 0.0f ? 1.0f : speed));
    if (g_audio_buffer.speed == 1.0f)
    {
        g_audio_buffer.src_buffer_size = AUDIO_BUFFER_SIZE;
    }
    else
    {
        g_audio_buffer.src_buffer_size = (int)((float)(AUDIO_BUFFER_SIZE / 4) * (float)g_audio_buffer.speed) * 4;
    }
}

void audio_stop()
{
    g_audio_buffer.ogg_id = -1;
    g_audio_buffer.read_pos = 0;
    g_audio_buffer.write_pos = 0;
    g_audio_buffer.written = 0;
}

void audio_destroy_ogg(int ogg_id)
{
    if (ogg_id < 0 || ogg_id >= MAX_OGG_FILES || !g_ogg_files[ogg_id].in_use) return;

    if (g_audio_buffer.ogg_id == ogg_id)
    {
        audio_stop();
    }

    ov_clear(&(g_ogg_files[ogg_id].vorbis_file));
    free(g_ogg_files[ogg_id].file_ptr);
    g_ogg_files[ogg_id].in_use = false;
}

void audio_play_sfx(int sfx_id)
{
    if (!g_settings.audio) return;
    if (sfx_id < 0 || sfx_id >= AUDIO_SFX_COUNT || !g_audio_sfx[sfx_id].in_use) return;

    int channel_index = -1;
    for (int i = 0; i < AUDIO_SFX_CHANNELS; i++)
    {
        if (!g_audio_sfx_channels[i].active)
        {
            channel_index = i;
            break;
        }
    }

    if (channel_index < 0) channel_index = 0;

    g_audio_sfx_channels[channel_index].sfx_id = sfx_id;
    g_audio_sfx_channels[channel_index].position = 0;
    g_audio_sfx_channels[channel_index].active = true;
}

void audio_set_score_flame_intensity(float intensity)
{
    if (!g_settings.audio)
    {
        g_audio_flame_channel.target_volume = 0;
        return;
    }

    float target_volume = CLAMP(intensity / 4.0f, 0.0f, 1.0f) * 2.35f;
    g_audio_flame_channel.target_volume = audio_volume_from_float(target_volume);
}

void audio_destroy_sfx()
{
    // Se a thread de loading ainda roda, espera antes de liberar os buffers.
    if (s_sfx_loader_thid >= 0)
    {
        sceKernelWaitThreadEnd(s_sfx_loader_thid, NULL);
        sceKernelDeleteThread(s_sfx_loader_thid);
        s_sfx_loader_thid = -1;
    }

    for (int i = 0; i < AUDIO_SFX_CHANNELS; i++)
    {
        g_audio_sfx_channels[i].active = false;
    }

    g_audio_flame_channel.target_volume = 0;
    g_audio_flame_channel.volume = 0;
    g_audio_flame_channel.position = 0;

    for (int i = 0; i < AUDIO_SFX_COUNT; i++)
    {
        if (g_audio_sfx[i].in_use)
        {
            free(g_audio_sfx[i].samples);
            g_audio_sfx[i].samples = NULL;
            g_audio_sfx[i].sample_count = 0;
            g_audio_sfx[i].in_use = false;
        }
    }
}
