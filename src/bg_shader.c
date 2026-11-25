// Fundo animado das partidas: porte do shader canvas_item original do Balatro.
// Render em baixa resolucao na CPU (120x68) + upscale bilinear na GU.
// Desenho direto na GU; ao final invalida o cache de textura do batch (g_current_set_texture)
// para o restante da cena voltar ao estado correto via graphics_set_texture().
#include <pspgu.h>
#include <pspkernel.h>
#include <math.h>
#include <stdint.h>

extern int g_current_set_texture;

#define BG_TEX_W 128
#define BG_TEX_H 128
#define BG_FB_W  120
#define BG_FB_H  68
#define BG_PAINT_ITERS 3

#define BG_SPIN_T_SCALE 0.0f  // spin congelado (fundo de partida nao gira)
#define BG_PAINT_T_SCALE 1.5f // velocidade da "pintura"

// Paleta: 5 verdes do feltro (escuro -> claro)
static const float BG_PAL[5][3] = {
    { 0.2745f, 0.4510f, 0.3490f }, // #467359
    { 0.2902f, 0.5490f, 0.4039f }, // #4A8C67
    { 0.3373f, 0.5490f, 0.4275f }, // #568C6D
    { 0.2863f, 0.6510f, 0.4784f }, // #49A67A
    { 0.3373f, 0.6510f, 0.5059f }, // #56A681
};
#define BG_CONTRAST 1.0f
#define BG_SPIN_AMOUNT 0.05f
#define BG_SPIN_EASE 0.5f

static uint32_t bg_tex[BG_TEX_W * BG_TEX_H] __attribute__((aligned(16)));
static float bg_base_ang[BG_FB_W * BG_FB_H];
static float bg_base_len[BG_FB_W * BG_FB_H];
static float bg_contrast_mod = 0.0f;
static uint32_t bg_pal_lut[1025];

// Fundo da tela de titulo (shader "splash" do Balatro: vortice vermelho/azul)
static uint32_t bg_splash_tex[BG_TEX_W * BG_TEX_H] __attribute__((aligned(16)));
static float bg_splash_ang[BG_FB_W * BG_FB_H];
static float bg_splash_len[BG_FB_W * BG_FB_H];

// Os shaders de fundo mudam devagar; regenerar a 30 Hz em vez de 60 Hz
// corta o custo pela metade sem diferenca visivel.
#define BG_RENDER_INTERVAL_US 33000

static uint64_t bg_last_render_us = 0;
static bool bg_tex_valid = false;
static uint64_t bg_splash_last_render_us = 0;
static bool bg_splash_tex_valid = false;

// Escreve de volta apenas as linhas de cache usadas pela textura recem-gerada.
static void bg_writeback_texture(const void *data, unsigned int bytes)
{
    unsigned int start = (unsigned int)data;
    unsigned int end = start + bytes;
    start &= ~63u;
    end = (end + 63u) & ~63u;
    sceKernelDcacheWritebackRange((void *)start, end - start);
}

static void bg_build_palette_lut(void)
{
    for (int i = 0; i <= 1024; i++)
    {
        float f = (float)i / 256.0f;
        int ps = (int)f;
        if (ps > 3) ps = 3;

        float ft = f - (float)ps;
        if (ft < 0.0f) ft = 0.0f;
        if (ft > 1.0f) ft = 1.0f;

        float r = BG_PAL[ps][0] + (BG_PAL[ps + 1][0] - BG_PAL[ps][0]) * ft;
        float g = BG_PAL[ps][1] + (BG_PAL[ps + 1][1] - BG_PAL[ps][1]) * ft;
        float b = BG_PAL[ps][2] + (BG_PAL[ps + 1][2] - BG_PAL[ps][2]) * ft;

        unsigned int ri = (unsigned int)(r * 255.0f);
        unsigned int gi = (unsigned int)(g * 255.0f);
        unsigned int bi = (unsigned int)(b * 255.0f);
        if (ri > 255) ri = 255;
        if (gi > 255) gi = 255;
        if (bi > 255) bi = 255;

        bg_pal_lut[i] = 0xFF000000u | (bi << 16) | (gi << 8) | ri;
    }
}

#define BG_SIN_BITS 10
#define BG_SIN_N (1 << BG_SIN_BITS)
static float bg_sin_lut[BG_SIN_N];

static void bg_build_sin_lut(void) {
    for (int i = 0; i < BG_SIN_N; i++)
        bg_sin_lut[i] = sinf((float)i * (2.0f * 3.14159265f / (float)BG_SIN_N));
}

static inline float bg_fsin(float x) {
    int i = (int)(x * 162.9747f);
    return bg_sin_lut[i & (BG_SIN_N - 1)];
}

static inline float bg_fcos(float x) {
    return bg_fsin(x + 1.5707963f);
}

void bg_shader_init(void) {
    bg_build_sin_lut();
    bg_build_palette_lut();

    const float scr_len = sqrtf((float)(SCREEN_WIDTH * SCREEN_WIDTH + SCREEN_HEIGHT * SCREEN_HEIGHT));
    for (int y = 0; y < BG_FB_H; y++) {
        for (int x = 0; x < BG_FB_W; x++) {
            float sx = ((float)x + 0.5f) * (float)SCREEN_WIDTH / (float)BG_FB_W;
            float sy = ((float)y + 0.5f) * (float)SCREEN_HEIGHT / (float)BG_FB_H;
            float ux = (sx - (float)SCREEN_WIDTH * 0.5f) / scr_len - 0.12f;
            float uy = (sy - (float)SCREEN_HEIGHT * 0.5f) / scr_len;
            bg_base_ang[y * BG_FB_W + x] = atan2f(uy, ux);
            bg_base_len[y * BG_FB_W + x] = sqrtf(ux * ux + uy * uy);
        }
    }
    bg_contrast_mod = 0.25f * BG_CONTRAST + 0.5f * BG_SPIN_AMOUNT + 1.2f;
}

typedef struct {
    float u, v;
    uint32_t color; // obrigatorio: o jogo usa blending global; sem cor o alpha=0 e o quad some
    float x, y, z;
} BgTexVertex;

void bg_shader_draw(void) {
    uint64_t now_us = sceKernelGetSystemTimeWide();

    if (!bg_tex_valid || now_us - bg_last_render_us >= BG_RENDER_INTERVAL_US) {
        bg_tex_valid = true;
        bg_last_render_us = now_us;

        float t = (float)now_us / 1000000.0f;

        float speed = (t * BG_SPIN_T_SCALE * BG_SPIN_EASE * 0.2f) + 302.2f;
        float spd = t * BG_PAINT_T_SCALE * 2.0f;
        float spd_rot = spd * 0.131121f;
        float spd_lin = 0.113f * spd;

        for (int i = 0; i < BG_FB_W * BG_FB_H; i++) {
            float uv_len = bg_base_len[i];
            float ang = bg_base_ang[i] + speed
                      - BG_SPIN_EASE * 20.0f * (BG_SPIN_AMOUNT * uv_len + (1.0f - BG_SPIN_AMOUNT));
            float uvx = uv_len * bg_fcos(ang);
            float uvy = uv_len * bg_fsin(ang);

            uvx *= 30.0f;
            uvy *= 30.0f;
            float s = uvx + uvy;
            float u2x = s, u2y = s;
            for (int k = 0; k < BG_PAINT_ITERS; k++) {
                float mx = uvx > uvy ? uvx : uvy;
                float sn = bg_fsin(mx);
                u2x += sn + uvx;
                u2y += sn + uvy;
                uvx += 0.5f * bg_fcos(5.1123314f + 0.353f * u2y + spd_rot);
                uvy += 0.5f * bg_fsin(u2x - spd_lin);
                float kk = bg_fcos(uvx + uvy) - bg_fsin(uvx * 0.711f - uvy);
                uvx -= kk;
                uvy -= kk;
            }

            float paint_res = sqrtf(uvx * uvx + uvy * uvy) * 0.035f * bg_contrast_mod;
            if (paint_res < 0.0f) paint_res = 0.0f;
            if (paint_res > 2.0f) paint_res = 2.0f;

            int pi = (int)(paint_res * 512.0f);
            if (pi > 1024) pi = 1024;

            int x = i % BG_FB_W;
            int y = i / BG_FB_W;
            bg_tex[y * BG_TEX_W + x] = bg_pal_lut[pi];
        }

        bg_writeback_texture(bg_tex, BG_TEX_W * BG_FB_H * sizeof(uint32_t));
    }

    // Descarrega o batch pendente e desenha em tela cheia com estado proprio
    graphics_flush_quads();

    BgTexVertex *v = (BgTexVertex *)sceGuGetMemory(2 * sizeof(BgTexVertex));
    v[0].u = 0.0f;          v[0].v = 0.0f;
    v[0].color = 0xFFFFFFFFu;
    v[0].x = 0.0f;          v[0].y = 0.0f;              v[0].z = 0.0f;
    v[1].u = (float)BG_FB_W; v[1].v = (float)BG_FB_H;
    v[1].color = 0xFFFFFFFFu;
    v[1].x = (float)SCREEN_WIDTH; v[1].y = (float)SCREEN_HEIGHT; v[1].z = 0.0f;

    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
    sceGuTexImage(0, BG_TEX_W, BG_TEX_H, BG_TEX_W, bg_tex);
    sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
    sceGuDisable(GU_TEXTURE_2D);

    // Forca o proximo graphics_set_texture() a reemitir o estado do batch
    g_current_set_texture = -2;
    graphics_set_no_texture();
}

// Fundo da tela de titulo: porte fiel do shader "splash" do Balatro.
// O jogo original usa vort_speed 0.4, colour_1 vermelho, colour_2 azul,
// 5 iteracoes de fumaca e um "mid_flash" que fica em 0 no menu principal.
void bg_splash_init(void)
{
    const float scr_len = sqrtf((float)(SCREEN_WIDTH * SCREEN_WIDTH + SCREEN_HEIGHT * SCREEN_HEIGHT));
    const float pixel_size = scr_len / 700.0f;

    for (int y = 0; y < BG_FB_H; y++) {
        for (int x = 0; x < BG_FB_W; x++) {
            float sx = ((float)x + 0.5f) * (float)SCREEN_WIDTH / (float)BG_FB_W;
            float sy = ((float)y + 0.5f) * (float)SCREEN_HEIGHT / (float)BG_FB_H;

            // Efeito de pixel do shader original: floor(screen_coords / pixel_size)
            sx = floorf(sx / pixel_size) * pixel_size;
            sy = floorf(sy / pixel_size) * pixel_size;

            float ux = (sx - (float)SCREEN_WIDTH * 0.5f) / scr_len;
            float uy = (sy - (float)SCREEN_HEIGHT * 0.5f) / scr_len;
            bg_splash_ang[y * BG_FB_W + x] = atan2f(uy, ux);
            bg_splash_len[y * BG_FB_W + x] = sqrtf(ux * ux + uy * uy);
        }
    }
}

void bg_splash_draw(float time)
{
    // Constantes do menu principal (game.lua -> Game:main_menu)
    const float vort_speed = 0.4f;
    const float c1r = 254.0f / 255.0f, c1g = 95.0f / 255.0f, c1b = 85.0f / 255.0f;   // #FE5F55
    const float c2r = 0.0f, c2g = 157.0f / 255.0f, c2b = 1.0f;                          // #009DFF
    const float black_r = 0.6f * 79.0f / 255.0f;
    const float black_g = 0.6f * 99.0f / 255.0f;
    const float black_b = 0.6f * 103.0f / 255.0f;

    uint64_t now_us = sceKernelGetSystemTimeWide();
    if (!bg_splash_tex_valid || now_us - bg_splash_last_render_us >= BG_RENDER_INTERVAL_US)
    {
        bg_splash_tex_valid = true;
        bg_splash_last_render_us = now_us;

        float t = time;
        float speed0 = t * vort_speed;
        float min_speed = speed0 < 6.0f ? speed0 : 6.0f;
        float swirl = 2.2f + 0.4f * min_speed - 1.0f - speed0 * 0.05f - min_speed * speed0 * 0.02f;
        float speed = t * 6.0f * vort_speed + 1033.0f;
        float smoke_ramp = t * 1.2f - 4.0f;
        if (smoke_ramp > 10.0f) smoke_ramp = 10.0f;

        for (int i = 0; i < BG_FB_W * BG_FB_H; i++) {
            float uv_len = bg_splash_len[i];
            float ang = bg_splash_ang[i] + swirl * uv_len;

            float svx = uv_len * bg_fcos(ang) * 30.0f;
            float svy = uv_len * bg_fsin(ang) * 30.0f;
            float uv2x = svx + svy;
            float uv2y = uv2x;

            for (int k = 0; k < 5; k++) {
                float mx = svx > svy ? svx : svy;
                float sn = bg_fsin(mx);
                uv2x += sn + svx;
                uv2y += sn + svy;
                svx += 0.5f * bg_fcos(5.1123314f + 0.353f * uv2y + speed * 0.131121f);
                svy += 0.5f * bg_fsin(uv2x - 0.113f * speed);
                float kk = bg_fcos(svx + svy) - bg_fsin(svx * 0.711f - svy);
                svx -= kk;
                svy -= kk;
            }

            float smoke = 1.5f + sqrtf(svx * svx + svy * svy) * 0.12f - 0.17f * smoke_ramp;
            if (smoke < -2.0f) smoke = -2.0f;
            if (smoke > 2.0f) smoke = 2.0f;
            if (smoke < 0.2f) smoke = (smoke - 0.2f) * 0.6f + 0.2f;

            float c1p = 1.0f - 2.0f * fabsf(1.0f - smoke);
            float c2p = 1.0f - 2.0f * smoke;
            if (c1p < 0.0f) c1p = 0.0f;
            if (c2p < 0.0f) c2p = 0.0f;
            float cb = 1.0f - (c1p + c2p);
            if (cb < 0.0f) cb = 0.0f;

            float r = c1r * c1p + c2r * c2p + cb * black_r;
            float g = c1g * c1p + c2g * c2p + cb * black_g;
            float b = c1b * c1p + c2b * c2p + cb * black_b;

            float mxp = c1p > c2p ? c1p : c2p;
            float flash = mxp * 5.0f - 4.4f;
            if (flash < 0.0f) flash = 0.0f;
            r = r * (1.0f - flash) + flash;
            g = g * (1.0f - flash) + flash;
            b = b * (1.0f - flash) + flash;

            unsigned int ri = (unsigned int)(r * 255.0f);
            unsigned int gi = (unsigned int)(g * 255.0f);
            unsigned int bi = (unsigned int)(b * 255.0f);
            if (ri > 255) ri = 255;
            if (gi > 255) gi = 255;
            if (bi > 255) bi = 255;

            int x = i % BG_FB_W;
            int y = i / BG_FB_W;
            bg_splash_tex[y * BG_TEX_W + x] = 0xFF000000u | (bi << 16) | (gi << 8) | ri;
        }
        bg_writeback_texture(bg_splash_tex, BG_TEX_W * BG_FB_H * sizeof(uint32_t));
    }

    graphics_flush_quads();

    BgTexVertex *v = (BgTexVertex *)sceGuGetMemory(2 * sizeof(BgTexVertex));
    v[0].u = 0.0f;          v[0].v = 0.0f;
    v[0].color = 0xFFFFFFFFu;
    v[0].x = 0.0f;          v[0].y = 0.0f;              v[0].z = 0.0f;
    v[1].u = (float)BG_FB_W; v[1].v = (float)BG_FB_H;
    v[1].color = 0xFFFFFFFFu;
    v[1].x = (float)SCREEN_WIDTH; v[1].y = (float)SCREEN_HEIGHT; v[1].z = 0.0f;

    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
    sceGuTexImage(0, BG_TEX_W, BG_TEX_H, BG_TEX_W, bg_splash_tex);
    sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
    sceGuDisable(GU_TEXTURE_2D);

    g_current_set_texture = -2;
    graphics_set_no_texture();
}

// ---------------- Chamas procedurais (shader de score flames do Balatro) ----------------
// Renderiza a regiao ACIMA da caixa (x, y, w, h); o corpo translucido sobre a caixa
// continua sendo desenhado pelo chamador.
#define FLAME_TEX_W 64
#define FLAME_TEX_H 32
#define FLAME_ITERS 3
#define FLAME_SLOTS 2

// Textura separada por chama: a GU le a textura depois que o comando foi
// enfileirado, entao as duas chamas nao podem compartilhar o mesmo buffer.
static uint32_t flame_tex[FLAME_SLOTS][FLAME_TEX_W * FLAME_TEX_H] __attribute__((aligned(16)));
static uint64_t flame_last_render_us[FLAME_SLOTS];
static int flame_last_rw[FLAME_SLOTS] = { -1, -1 };
static int flame_last_rh[FLAME_SLOTS] = { -1, -1 };
static bool flame_tex_valid[FLAME_SLOTS];

// Coordenadas de pixel nao dependem do tempo; so mudam quando o tamanho da
// chama muda. Ficam em tabelas para evitar divisoes e floorf por pixel.
static float flame_uvx[FLAME_TEX_W];
static float flame_uvy[FLAME_TEX_H];
static float flame_fx[FLAME_TEX_W];
static float flame_fy[FLAME_TEX_H];
static int flame_tab_rw = -1;
static int flame_tab_rh = -1;

void bg_draw_flames(float x, float y, float w, float h, uint32_t colour_1, uint32_t colour_2, float intensity, float id)
{
    (void)h;
    uint64_t now_us = sceKernelGetSystemTimeWide();
    if (intensity > 10.0f) intensity = 10.0f;
    if (intensity < 0.1f) return;

    float c1r = (float)(colour_1 & 0xFF) / 255.0f;
    float c1g = (float)((colour_1 >> 8) & 0xFF) / 255.0f;
    float c1b = (float)((colour_1 >> 16) & 0xFF) / 255.0f;
    float c2r = (float)(colour_2 & 0xFF) / 255.0f;
    float c2g = (float)((colour_2 >> 8) & 0xFF) / 255.0f;
    float c2b = (float)((colour_2 >> 16) & 0xFF) / 255.0f;

    int rw = (int)(w + 2.0f);
    if (rw > FLAME_TEX_W) rw = FLAME_TEX_W;
    int rh = (int)(8.0f + intensity * 1.8f + 4.0f);
    if (rh > FLAME_TEX_H) rh = FLAME_TEX_H;
    float ox = x - 1.0f;
    float oy = y - (float)rh;
    if (oy < 0.0f) oy = 0.0f;

    // Os chamadores usam id 1 e 2; cada um mantem sua propria textura.
    int slot = (id >= 1.5f) ? 1 : 0;

    if (rw != flame_tab_rw) {
        for (int i = 0; i < rw; i++) {
            float uvx = ((float)i + 0.5f) / (float)rw - 0.5f;
            flame_uvx[i] = uvx;
            flame_fx[i] = floorf(uvx * 60.0f) / 60.0f;
        }
        flame_tab_rw = rw;
    }
    if (rh != flame_tab_rh) {
        for (int j = 0; j < rh; j++) {
            float uvy = ((float)j + 0.5f) / (float)rh - 0.5f;
            flame_uvy[j] = uvy;
            flame_fy[j] = floorf(uvy * 60.0f) / 60.0f;
        }
        flame_tab_rh = rh;
    }

    if (!flame_tex_valid[slot] || rw != flame_last_rw[slot] || rh != flame_last_rh[slot] ||
        now_us - flame_last_render_us[slot] >= BG_RENDER_INTERVAL_US)
    {
        flame_tex_valid[slot] = true;
        flame_last_render_us[slot] = now_us;
        flame_last_rw[slot] = rw;
        flame_last_rh[slot] = rh;

        float t = (float)now_us / 1000000.0f;
        float flame_scroll = fmodf(4.0f * t, 10000.0f) - 5000.0f + fmodf(1.781f * id, 1000.0f);
        float scale_fac = 7.5f + 3.0f / (2.0f + 2.0f * intensity);
        float speed = fmodf(20.781f * id, 100.0f) + bg_fsin(t + id) * bg_fcos(t * 0.151f + id);
        float speed_a = speed * 0.0812f;
        float speed_b = speed * 0.1531f;
        float speed_c = 0.113785f * speed;
        float speed_d = 0.13582f * speed;
        float dens_k = 2.0f / (2.0f + intensity * 0.2f);
        float e2 = 2.0f - 0.3f * intensity;
        float extra_k = e2 > 0.0f ? e2 : 0.0f;
        float hole_lim = intensity * 0.5f < 0.1f ? intensity * 0.5f : 0.1f;
        float hole_k = intensity * 10.0f < 8.5f ? intensity * 10.0f : 8.5f;

        for (int j = 0; j < rh; j++) {
            for (int i = 0; i < rw; i++) {
                float uvx = flame_uvx[i];
                float uvy = flame_uvy[j];

                float fx = flame_fx[i];
                float fy = flame_fy[j];

                float dw = bg_fsin(-1.123f * fx + 0.2f * t) * bg_fcos(5.3332f * fy + t * 0.931f);
                float scx = fx + fx * 0.01f * dw;
                float scy = fy + fy * 0.01f * dw;

                float svx = scx * scale_fac;
                float svy = scy * scale_fac + flame_scroll;

                float sv2x = 0.0f, sv2y = 0.0f;
                for (int k = 0; k < FLAME_ITERS; k++) {
                    float im = (k & 1) ? -1.0f : 1.0f;
                    float L = sqrtf(svx * svx + svy * svy);
                    float kk = bg_fcos(L * 0.411f) + 0.3344f * bg_fsin(L) - 0.23f * bg_fcos(L);
                    float n2x = sv2x + svx + 0.05f * sv2y * im + 0.3f * kk;
                    float n2y = sv2y + svy + 0.05f * sv2x * im + 0.3f * kk;
                    sv2x = n2x;
                    sv2y = n2y;
                    svx += 0.5f * bg_fcos(bg_fcos(sv2y) + speed_a) * bg_fsin(3.22f + sv2x - speed_b);
                    svy += 0.5f * bg_fsin(-sv2x * 1.21222f + speed_c) * bg_fcos(sv2y * 0.91213f - speed_d);
                }

                float dx = svx / scale_fac * 5.0f;
                float dy = (svy - flame_scroll) / scale_fac * 5.0f;
                float smoke = (sqrtf(dx * dx + dy * dy)
                               + 0.1f * (sqrtf(scx * scx + scy * scy) - 0.5f)) * dens_k;
                float dy2 = uvy - 0.5f;
                float base = 2.0f * dy2 * dy2;
                if (base > 0.0f) smoke += extra_k * base;
                float ax = uvx < 0.0f ? -uvx : uvx;
                if (ax > 0.4f) smoke += 10.0f * (ax - 0.4f);
                float ex = uvx * 0.19f;
                float ey = uvy - 0.1f;
                float el = sqrtf(ex * ex + ey * ey);
                if (el < hole_lim && smoke > 1.0f)
                    smoke += hole_k * (el - 0.1f);

                float r = c1r, g = c1g, b = c1b, a = 0.0f;
                if (smoke <= 1.0f) {
                    if (uvy < 0.12f) {
                        float yf = 0.12f - uvy;
                        r = r * (1.0f - 0.5f * yf) + 2.5f * yf * c2r;
                        g = g * (1.0f - 0.5f * yf) + 2.5f * yf * c2g;
                        b = b * (1.0f - 0.5f * yf) + 2.5f * yf * c2b;
                        float k2 = (-2.0f + 0.5f * intensity * smoke) * yf;
                        r += r * k2;
                        g += g * k2;
                        b += b * k2;
                    }
                    a = 1.0f;
                }

                unsigned int ri = (unsigned int)(r * 255.0f);
                unsigned int gi = (unsigned int)(g * 255.0f);
                unsigned int bi = (unsigned int)(b * 255.0f);
                if (ri > 255) ri = 255;
                if (gi > 255) gi = 255;
                if (bi > 255) bi = 255;
                flame_tex[slot][j * FLAME_TEX_W + i] = (a >= 1.0f ? 0xFF000000u : 0x00000000u) | (bi << 16) | (gi << 8) | ri;
            }
        }
        bg_writeback_texture(flame_tex[slot], FLAME_TEX_W * rh * sizeof(uint32_t));
    }

    graphics_flush_quads();

    BgTexVertex *v = (BgTexVertex *)sceGuGetMemory(2 * sizeof(BgTexVertex));
    v[0].u = 0.0f;     v[0].v = 0.0f;
    v[0].color = 0xFFFFFFFFu;
    v[0].x = ox;       v[0].y = oy;        v[0].z = 0.0f;
    v[1].u = (float)rw; v[1].v = (float)rh;
    v[1].color = 0xFFFFFFFFu;
    v[1].x = ox + (float)rw; v[1].y = oy + (float)rh; v[1].z = 0.0f;

    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
    sceGuTexImage(0, FLAME_TEX_W, FLAME_TEX_H, FLAME_TEX_W, flame_tex[slot]);
    sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, v);
    sceGuDisable(GU_TEXTURE_2D);

    g_current_set_texture = -2;
    graphics_set_no_texture();
}
