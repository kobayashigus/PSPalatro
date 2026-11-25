#include "global.h"
#include <string.h>
#include <zip.h>

zip_source_t *g_archive_source = NULL;
zip_t *g_archive_file = NULL;

static bool archive_open_impl(const char *filename, bool quiet)
{
    char str[64];
    if (!quiet)
    {
        sprintf(str, "Opening archive \"%s\"", filename);
        game_draw_loading_text(str, COLOR_WHITE, COLOR_BLACK);
    }
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        if (!quiet)
        {
            sprintf(str, "Can't open \"%s\"", filename);
            game_draw_loading_text(str, COLOR_TEXT_RED, COLOR_BLACK);
        }
        return false;
    }

    // Look for start of archive in the file.
    // Otimização: scan bufferizado (64KB) em RAM em vez de fgetc() por byte
    // (56MB via fgetc = dezenas de milhões de syscalls no MS).
    long found_offset = -1;
    {
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        #define ARCHIVE_SCAN_BUF 65536
        uint8_t *scan_buf = (uint8_t *)malloc(ARCHIVE_SCAN_BUF + 3);
        if (scan_buf != NULL && file_size > 0)
        {
            long base = 0;      // offset em arquivo de scan_buf[0]
            size_t carry = 0;   // bytes reaproveitados do chunk anterior (overlap)
            while (1)
            {
                size_t n = fread(scan_buf + carry, 1, ARCHIVE_SCAN_BUF, fp);
                if (n == 0) break;
                size_t total = carry + n;
                for (size_t i = 0; i + 4 <= total; i++)
                {
                    if (scan_buf[i] == 'P' && scan_buf[i+1] == 'K' &&
                        scan_buf[i+2] == 0x03 && scan_buf[i+3] == 0x04)
                    {
                        found_offset = base + (long)i;
                        break;
                    }
                }
                if (found_offset >= 0) break;
                if (feof(fp)) break;
                // preserva últimos 3 bytes para assinatura dividida entre chunks
                if (total >= 3)
                {
                    scan_buf[0] = scan_buf[total - 3];
                    scan_buf[1] = scan_buf[total - 2];
                    scan_buf[2] = scan_buf[total - 1];
                    base += (long)(total - 3);
                    carry = 3;
                }
                else
                {
                    base += (long)total;
                    carry = total;
                }
            }
            free(scan_buf);
        }
        #undef ARCHIVE_SCAN_BUF
    }

    if (found_offset < 0)
    {
        if (!quiet)
        {
            char str2[64];
            sprintf(str2, "Error finding archive header in file \"%s\"", filename);
            game_draw_loading_text(str2, COLOR_TEXT_RED, COLOR_BLACK);
        }
        fclose(fp);
        return false;
    }
    long count = found_offset;
    fclose(fp);

    DEBUG_PRINTF("Archive header found in %ld\n", count);

    zip_error_t error;
    g_archive_source = zip_source_file_create(filename, count, -1, &error);
    g_archive_file = zip_open_from_source(g_archive_source, 0, &error);
    if (!g_archive_file)
    {
        if (!quiet)
        {
            sprintf(str, "Error opening archive in file \"%s\"", filename);
            game_draw_loading_text(str, COLOR_TEXT_RED, COLOR_BLACK);
        }
        return false;
    }

    return true;
}

bool archive_open(const char *filename)
{
    return archive_open_impl(filename, false);
}

// Abertura sem nenhum draw/GU: unica segura fora da thread principal
// (a thread de loading dos SFX usa essa).
bool archive_open_quiet(const char *filename)
{
    return archive_open_impl(filename, true);
}

void archive_close()
{
    if (g_archive_file)
    {
        zip_close(g_archive_file);
        g_archive_source = NULL;
        g_archive_file = NULL;
    }
}

uint8_t *archive_load_file_entry(const char *filename, size_t *file_size)
{
    if (g_archive_file == NULL || filename == NULL || file_size == NULL) return NULL;

    *file_size = 0;

    size_t size = archive_get_file_entry_size(filename);
    if (size == 0) return NULL;

    uint8_t *buffer = (uint8_t *)malloc(size);
    if (buffer == NULL) return NULL;

    zip_file_t *f = zip_fopen(g_archive_file, filename, 0);
    if (f == NULL)
    {
        free(buffer);
        return NULL;
    }

    // zip_fread pode retornar menos que o pedido; ler ate completar ou EOF.
    size_t total = 0;
    while (total < size)
    {
        zip_int64_t n = zip_fread(f, buffer + total, size - total);
        if (n < 0)
        {
            zip_fclose(f);
            free(buffer);
            return NULL;
        }
        if (n == 0) break;
        total += (size_t)n;
    }

    zip_fclose(f);

    if (total != size)
    {
        free(buffer);
        return NULL;
    }

    *file_size = size;
    return buffer;
}

size_t archive_get_file_entry_size(const char *filename)
{
    zip_stat_t stat;
    if (g_archive_file == NULL) return 0;
    if (zip_stat(g_archive_file, filename, 0, &stat) != 0) return 0;
    
    return stat.size;
}
