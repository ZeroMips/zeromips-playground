#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xosera_m68k_defs.h"

const uint16_t vram_base_a      = 0x0000;
const uint16_t vram_base_b      = 0xA000;
const uint16_t vram_base_blank  = 0xB800;
const uint16_t vram_base_ball   = 0xBC00;
const uint16_t tile_base_b      = 0xC000;

#define PI  3.1415926f
#define PAU (1.5f * PI)
#define TAU (2.0f * PI)

#define WIDTH   640
#define HEIGHT  480

#define WIDTH_A     (WIDTH / 2)
#define HEIGHT_A    (HEIGHT / 2)
#define WIDTH_B     (WIDTH / 1)
#define HEIGHT_B    (HEIGHT / 1)

#define TILE_WIDTH_B    8
#define TILE_HEIGHT_B   8

#define PIXELS_PER_WORD_A   8
#define PIXELS_PER_WORD_B   4

#define COLS_PER_WORD_A 8
#define ROWS_PER_WORD_A 1
#define COLS_PER_WORD_B TILE_WIDTH_B
#define ROWS_PER_WORD_B TILE_HEIGHT_B

#define WIDTH_WORDS_A   (WIDTH_A / COLS_PER_WORD_A)
#define HEIGHT_WORDS_A  (HEIGHT_A / ROWS_PER_WORD_A)
#define WIDTH_WORDS_B   (WIDTH_B / COLS_PER_WORD_B)
#define HEIGHT_WORDS_B  (HEIGHT_B / ROWS_PER_WORD_B)

#define BALL_BITMAP_WIDTH   256
#define BALL_BITMAP_HEIGHT  256

#define BALL_TILES_WIDTH    (BALL_BITMAP_WIDTH / TILE_WIDTH_B)
#define BALL_TILES_HEIGHT   (BALL_BITMAP_WIDTH / TILE_HEIGHT_B)

#define SHADOW_OFFSET_X 36
#define SHADOW_OFFSET_Y 0

#define BALL_SHIFT_X    (-SHADOW_OFFSET_X / 2)
#define BALL_SHIFT_Y    (-SHADOW_OFFSET_Y / 2)

#define BALL_CENTER_X   (BALL_BITMAP_WIDTH / 2 + BALL_SHIFT_X)
#define BALL_CENTER_Y   (BALL_BITMAP_HEIGHT / 2 + BALL_SHIFT_Y)

#define BALL_RADIUS 80

#define BALL_THETA_START    (+PI)
#define BALL_THETA_STOP     (0.0f)
#define BALL_THETA_STEP     ((BALL_THETA_STOP - BALL_THETA_START) / 8.0f)

#define BALL_PHI_START      (-PI)
#define BALL_PHI_STOP       (0.0f)
#define BALL_PHI_STEP       ((BALL_PHI_STOP - BALL_PHI_START) / 8.0f)

#define WALL_DIST   32
#define WALL_LEFT   (WALL_DIST)
#define WALL_RIGHT  (320 - WALL_DIST)
#define WALL_BOTTOM (WALL_DIST)
#define WALL_TOP    (240 - WALL_DIST)

#define WIDESCREEN  true
#define PAINT_BALL  true
// USE_COPPER must currently be true for proper operation
#define USE_COPPER  true

uint8_t bg_bitmap[HEIGHT_A][WIDTH_A] = {0};
uint8_t bg_bitmap_real[2 * sizeof(bg_bitmap) / 8] = {0};
uint8_t ball_bitmap[BALL_BITMAP_HEIGHT][BALL_BITMAP_WIDTH] = {0};
uint16_t ball_tiles[BALL_TILES_HEIGHT][BALL_TILES_WIDTH][TILE_HEIGHT_B][TILE_WIDTH_B / PIXELS_PER_WORD_B] = {0};
uint16_t palettes[14*16] = {0};

uint32_t copper_list[] = {
    [0x0000] =  COP_MOVEC(vram_base_b, 0x0041 << 1 | 0x1),                                      // Fill dst
    [0x0001] =  COP_MOVEC(0, 0x0106 << 1 | 0x1),                                                // Fill scroll
    [0x0002] =  COP_MOVEC(MAKE_GFX_CTRL(0x00, 0, XR_GFX_BPP_4, 0, 0, 0), 0x0107 << 1 | 0x1),    // Fill gfx_ctrl with colorbase
                COP_JUMP(0x0040 << 1),

    [0x0040] =  COP_MOVEC(COP_MOVEC(0, 0x0104 << 1 | 0x1) >> 16, 0x0041 << 1 | 0x0),    // Make following move point to dst
    [0x0041] =  COP_MOVEC(0, 0),
                COP_MOVEC(COP_MOVEC(0, 0x0101 << 1 | 0x1) >> 16, 0x0041 << 1 | 0x0),    // Make preceding move point to prev_dst

    [0x0043] =  COP_JUMP(0x0080 << 1),  // Jumps either to blitter load or to wait_f
    [0x0044] =  COP_MOVEC(COP_JUMP(0x0080 << 1) >> 16, 0x0043 << 1 | 0x0),  // Make branching jump go to 0x0080 << 1
                COP_WAIT_F(),

    [0x0080] =  COP_MOVEC(COP_JUMP(0x0044 << 1) >> 16, 0x0043 << 1 | 0x0),  // Make branching jump go to 0x0080 << 1
                COP_JUMP(0x00C0 << 1),

                // Load fixed blitter settings
    [0x00C0] =  COP_MOVER(XB_(0x00, 8, 8) | XB_(0, 5, 1) | XB_(0, 4, 1) | XB_(0, 3, 1) | XB_(0, 2, 1) | XB_(1, 1, 1) | XB_(0, 0, 1), BLIT_CTRL),
                COP_MOVER(0x0000, BLIT_MOD_A),
                COP_MOVER(0x0000, BLIT_MOD_B),
                COP_MOVER(0xFFFF, BLIT_SRC_B),
                COP_MOVER(0x0000, BLIT_MOD_C),
                COP_MOVER(0x0000, BLIT_VAL_C),
                COP_MOVER(WIDTH_WORDS_B - BALL_TILES_WIDTH, BLIT_MOD_D),
                COP_MOVER(XB_(0xF, 12, 4) | XB_(0xF, 8, 4) | XB_(0, 0, 2), BLIT_SHIFT),
                COP_MOVER(BALL_TILES_HEIGHT - 1, BLIT_LINES),

                COP_WAIT_V(480),
                COP_JUMP(0x0100 << 1),

                // Blank existing ball
    [0x0100] =  COP_MOVER(vram_base_blank, BLIT_SRC_A),
    [0x0101] =  COP_MOVER(vram_base_b, BLIT_DST_D),         // Fill in prev_dst
    [0x0102] =  COP_MOVER(BALL_TILES_WIDTH - 1, BLIT_WORDS),

                // Draw ball
    [0x0103] =  COP_MOVER(vram_base_ball, BLIT_SRC_A),
    [0x0104] =  COP_MOVER(0, BLIT_DST_D),                   // Fill in dst
    [0x0105] =  COP_MOVER(BALL_TILES_WIDTH - 1, BLIT_WORDS),

    [0x0106] =  COP_MOVER(0, PB_HV_SCROLL),                 // Fill scroll

    [0x0107] =  COP_MOVER(0, PB_GFX_CTRL),                  // Fill gfx_ctrl with colorbase

                COP_JUMP(0x0003 << 1),
};

int abs(int x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

static inline int interpolate(int x0, int x1, int y0, int y1, int x) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    if (dx == 0) {
       return y0 + dy / 2;
    } else {
        return y0 + dy * (x - x0) / dx;
    }
}

static inline uint8_t interpolate_colour(int x0, int x1, uint8_t c0, uint8_t c1, int x) {
    c0 -= 2;
    c1 -= 2;

    if (c1 < c0) {
        c1 += 14;
    }

    return interpolate(x0, x1, c0, c1, x) % 14 + 2;
}

void draw_line(int width, int height, uint8_t bitmap[height][width], int x0, int y0, int x1, int y1, uint8_t colour) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? +1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? +1 : -1;

    int err = dx + dy;

    while (true) {
        bitmap[height - 1 - y0][x0] = colour;
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_line_chunky(int width, int height, uint8_t bitmap[height][width], int x0, int y0, int x1, int y1, uint8_t colour) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? +1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? +1 : -1;

    int err = dx + dy;

    while (true) {
        bitmap[height - 1 - y0][x0] = colour;
        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        } else if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static inline long scale_coord(long v, long scale, long v_center, long scale_center) {
    return (v - v_center) * scale / scale_center + v_center;
}

void draw_line_scale(int width, int height, uint8_t bitmap[height][width], int x0, int y0, int scale0, int x1, int y1, int scale1, int scale_base, uint8_t colour) {
    x0 = scale_coord(x0, scale0, width / 2, scale_base);
    y0 = scale_coord(y0, scale0, height / 2, scale_base);
    x1 = scale_coord(x1, scale1, width / 2, scale_base);
    y1 = scale_coord(y1, scale1, height / 2, scale_base);
    draw_line(width, height, bitmap, x0, y0, x1, y1, colour);
}

void draw_bg(void) {
    const int scale_base = 16;
    const int scale_front = 18;
    const int scale_back = 14;
    const int dx = 16;
    const int dy = 16;
    const int dscale = 1;

    // Draw back
    for (int x = WALL_LEFT; x <= WALL_RIGHT; x += dx) {
        draw_line_scale(WIDTH_A, HEIGHT_A, bg_bitmap, x, WALL_BOTTOM, scale_back, x, WALL_TOP, scale_back, scale_base, 1);
    }
    for (int y = WALL_BOTTOM; y <= WALL_TOP; y += dy) {
        draw_line_scale(WIDTH_A, HEIGHT_A, bg_bitmap, WALL_LEFT, y, scale_back, WALL_RIGHT, y, scale_back, scale_base, 1);
    }

    // Draw floor
    for (int x = WALL_LEFT; x <= WALL_RIGHT; x += dx) {
        draw_line_scale(WIDTH_A, HEIGHT_A, bg_bitmap, x, WALL_BOTTOM, scale_back, x, WALL_BOTTOM, scale_front, scale_base, 1);
    }
    for (int scale = scale_back; scale <= scale_front; scale += dscale) {
        draw_line_scale(WIDTH_A, HEIGHT_A, bg_bitmap, WALL_LEFT, WALL_BOTTOM, scale, WALL_RIGHT, WALL_BOTTOM, scale, scale_base, 1);
    }
    uint8_t *p = (uint8_t*)bg_bitmap;
    unsigned int wp = 0;
    for (int k = 0; k < WIDTH_A * HEIGHT_A; ++k)
    {
        uint8_t val = 0;
        if (k && !(k % WIDTH_A))
            wp+=WIDTH_A;
        if (p[k])
            val = 1<<(7-k%8);
        bg_bitmap_real[wp/8] = bg_bitmap_real[wp/8] | val;
        wp++;
    }
}

void do_tiles(void) {   // Replace with generic bitmap to tilemap
    for (uint16_t tile_row = 0; tile_row < BALL_TILES_HEIGHT; ++tile_row) {
        for (uint16_t tile_col = 0; tile_col < BALL_TILES_WIDTH; ++tile_col) {
            for (uint16_t row_in_tile = 0; row_in_tile < TILE_HEIGHT_B; ++row_in_tile) {
                for (uint16_t word_in_tile_row = 0; word_in_tile_row < TILE_WIDTH_B / PIXELS_PER_WORD_B; ++word_in_tile_row) {
                    int pixel_row = tile_row * TILE_HEIGHT_B + row_in_tile;
                    int pixel_col = tile_col * TILE_WIDTH_B + word_in_tile_row * PIXELS_PER_WORD_B;

                    uint16_t word = 0;
                    for (int nibble = 0; nibble < PIXELS_PER_WORD_B; ++nibble) {
                        word = word << PIXELS_PER_WORD_B | ball_bitmap[pixel_row][pixel_col + nibble];
                    }

//                    word = ((word & 0xff) << 8) | ((word >> 8) & 0xff);

                    ball_tiles[tile_row][tile_col][row_in_tile][word_in_tile_row] = word;
                }
            }
        }
    }
}

void draw_face_line(int x_bl, int y_bl, int x_br, int y_br, int x_tl, int y_tl, int x_tr, int y_tr, int x_b, int x_t, uint8_t colour) {
    if (x_b < x_bl || x_b > x_br || x_t < x_tl || x_t > x_tr) {
        return;
    }

    int y_b = interpolate(x_bl, x_br, y_bl, y_br, x_b);
    int y_t = interpolate(x_tl, x_tr, y_tl, y_tr, x_t);

    draw_line_chunky(BALL_BITMAP_WIDTH, BALL_BITMAP_HEIGHT, ball_bitmap, x_b, y_b, x_t, y_t, colour);
}

void draw_face(int x_bl, int y_bl, int x_br, int y_br, int x_tl, int y_tl, int x_tr, int y_tr, uint8_t colour_start, uint8_t colour_end) {
    if (x_tr - x_tl < x_br - x_bl) {
        for (int x_b = x_bl; x_b <= x_br; ++x_b) {
            int x_t = interpolate(x_bl, x_br, x_tl, x_tr, x_b);
            int colour = interpolate_colour(x_bl, x_br, colour_start, colour_end, x_b);
            draw_face_line(x_bl, y_bl, x_br, y_br, x_tl, y_tl, x_tr, y_tr, x_b, x_t, colour);
        }
    } else {
        for (int x_t = x_tl; x_t <= x_tr; ++x_t) {
            int x_b = interpolate(x_tl, x_tr, x_bl, x_br, x_t);
            int colour = interpolate_colour(x_tl, x_tr, colour_start, colour_end, x_t);
            draw_face_line(x_bl, y_bl, x_br, y_br, x_tl, y_tl, x_tr, y_tr, x_b, x_t, colour);
        }
    }
}

void fill_ball(void) {
    memset(ball_bitmap, 0, sizeof(ball_bitmap));

    uint8_t colour = 3;

    for (float theta = BALL_THETA_START; theta > BALL_THETA_STOP - BALL_THETA_STEP / 2; theta += BALL_THETA_STEP) {
        for (float phi = BALL_PHI_START; phi < BALL_PHI_STOP + BALL_PHI_STEP / 2; phi += BALL_PHI_STEP) {
            float theta_b = theta;
            float theta_t = theta + BALL_THETA_STEP;
            float phi_l = phi;
            float phi_r = phi + BALL_PHI_STEP;

            float s_theta_b = sinf(theta_b);
            float c_theta_b = cosf(theta_b);
            float s_theta_t = sinf(theta_t);
            float c_theta_t = cosf(theta_t);
            float c_phi_l = cosf(phi_l);
            float c_phi_r = cosf(phi_r);

            float x_bl = BALL_RADIUS * c_phi_l * s_theta_b;
            float y_bl = BALL_RADIUS * c_theta_b;
            float x_br = BALL_RADIUS * c_phi_r * s_theta_b;
            float y_br = BALL_RADIUS * c_theta_b;
            float x_tl = BALL_RADIUS * c_phi_l * s_theta_t;
            float y_tl = BALL_RADIUS * c_theta_t;
            float x_tr = BALL_RADIUS * c_phi_r * s_theta_t;
            float y_tr = BALL_RADIUS * c_theta_t;

            float rot = -0.28;

            float s_rot = sinf(rot);
            float c_rot = cosf(rot);

            float x_bl_r = x_bl * c_rot - y_bl * s_rot;
            float y_bl_r = y_bl * c_rot + x_bl * s_rot;
            float x_br_r = x_br * c_rot - y_br * s_rot;
            float y_br_r = y_br * c_rot + x_br * s_rot;
            float x_tl_r = x_tl * c_rot - y_tl * s_rot;
            float y_tl_r = y_tl * c_rot + x_tl * s_rot;
            float x_tr_r = x_tr * c_rot - y_tr * s_rot;
            float y_tr_r = y_tr * c_rot + x_tr * s_rot;

            uint8_t next_colour = (colour - 2 + 7) % 14 + 2;

            draw_face(
                BALL_CENTER_X + roundf(x_bl_r),
                BALL_CENTER_Y + roundf(y_bl_r),
                BALL_CENTER_X + roundf(x_br_r),
                BALL_CENTER_Y + roundf(y_br_r),
                BALL_CENTER_X + roundf(x_tl_r),
                BALL_CENTER_Y + roundf(y_tl_r),
                BALL_CENTER_X + roundf(x_tr_r),
                BALL_CENTER_Y + roundf(y_tr_r),
                colour, next_colour
            );

            colour = next_colour;
        }
    }
}

void shadow_ball(void) {
    for (int row = 0; row < BALL_BITMAP_HEIGHT; ++row) {
        if (row < SHADOW_OFFSET_Y || row >= BALL_BITMAP_HEIGHT - SHADOW_OFFSET_Y) {
            continue;
        }
        for (int col = 0; col < BALL_BITMAP_WIDTH; ++col) {
            if (col < -SHADOW_OFFSET_X || col >= BALL_BITMAP_WIDTH + SHADOW_OFFSET_X) {
                continue;
            }
            if (ball_bitmap[row][col] == 0 && 
                ball_bitmap[row + SHADOW_OFFSET_Y][col - SHADOW_OFFSET_X] > 1) {
                ball_bitmap[row][col] = 1;
            }
        }
    }
}

void do_palettes(void) {

    for (uint16_t palette_index = 0; palette_index < 14; ++palette_index) {
        uint8_t colour_base = palette_index * 16;

        palettes[colour_base + 0] = 0x0000; // Transparent Black
        palettes[colour_base + 1] = 0x0060; // Translucent Black
        for (uint16_t colour = 0; colour < 7; ++colour) {
            palettes[colour_base + 2 + (palette_index + colour) % 14] = 0xFFFF;   // White
        }
        for (uint16_t colour = 7; colour < 14; ++colour) {
            palettes[colour_base + 2 + (palette_index + colour) % 14] = 0x00FF;   // Red
        }
    }
}

static uint16_t swab16(uint16_t x)
{
    return x<<8 | x>>8;
}

static uint32_t swab32(uint32_t x)
{
    return x<<24 | x>>24 |
        (x & (uint32_t)0x0000ff00UL)<<8 |
        (x & (uint32_t)0x00ff0000UL)>>8;
}

int main() {
    FILE *f;
    printf("Rendering background\n");
    draw_bg();

    f = fopen("bg.bin", "w");
    fwrite(bg_bitmap, sizeof(bg_bitmap), 1, f);
    fclose(f);

    f = fopen("bg_real.bin", "w");
    fwrite(bg_bitmap_real, sizeof(bg_bitmap_real), 1, f);
    fclose(f);

    printf("Rendering ball\n");
    fill_ball();
    printf("Rendering shadow\n");
    shadow_ball();

    f = fopen("ball.bin", "w");
    fwrite(ball_bitmap, sizeof(ball_bitmap), 1, f);
    fclose(f);

    printf("Converting to tiles\n");
    do_tiles();

    for (unsigned int i = 0; i <sizeof(ball_tiles)/2; ++i)
        ((uint16_t*)ball_tiles)[i] = swab16(((uint16_t*)ball_tiles)[i]);

    f = fopen("tiles.bin", "w");
    fwrite(ball_tiles, sizeof(ball_tiles), 1, f);
    fclose(f);

    printf("Generating palettes\n");
    do_palettes();

    f = fopen("palettes.bin", "w");
    fwrite(palettes, sizeof(palettes), 1, f);
    fclose(f);

    printf("Write copperlist\n");

    for (unsigned int i = 0; i <sizeof(copper_list)/4; ++i)
        copper_list[i] = swab32(copper_list[i]);

    f = fopen("copperlist.bin", "w");
    fwrite(copper_list, sizeof(copper_list), 1, f);
    fclose(f);
}
