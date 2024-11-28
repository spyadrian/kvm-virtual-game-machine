#ifndef _GFX_H_
#define _GFX_H_

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <semaphore.h>
#include <SDL2/SDL_image.h>

// --- DEFINE ---

#define GFX_RGB(r,g,b) ((pixel_t){b,g,r,0})

#define GFX_COL_BLACK  GFX_RGB(0,0,0)
#define GFX_COL_RED    GFX_RGB(0,0,255)
#define GFX_COL_GREEN  GFX_RGB(0,255,0)
#define GFX_COL_BLUE   GFX_RGB(255,0,0)
#define GFX_COL_CYAN   GFX_RGB(255,255,0)
#define GFX_COL_PURPLE GFX_RGB(255,0,255)
#define GFX_COL_YELLOW GFX_RGB(0,255,255)
#define GFX_COL_WHITE  GFX_RGB(255,255,255)

#define MAX_SPRITES 8

// Structure of a pixel: 32-bits (ARGB)
typedef struct __attribute__ ((__packed__)) {

    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} pixel_t;

typedef struct {

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background_texture;
    pixel_t *background;
    int pitch;
    int width;
    int height;
} gfx_context_t;

typedef struct {

    SDL_Texture *tex;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
    uint8_t *data;
    uint32_t toggle; // (1 = visible , 0 = not visible )
} sprite_t;

// - extern variable for the main thread -
extern sem_t sem_gfx;
extern uint32_t width_gfx;
extern uint32_t height_gfx;
extern gfx_context_t *ctxt;
extern sprite_t sprites[MAX_SPRITES];

// --- FUNCTION ---

gfx_context_t* gfx_create(char *text, int width, int height);

void gfx_destroy(gfx_context_t *ctxt);

void gfx_background_putpixel(gfx_context_t *ctxt, int x, int y, pixel_t color);

void gfx_background_clear(gfx_context_t *ctxt, pixel_t color);

void gfx_background_update(gfx_context_t *ctxt);

SDL_Texture *gfx_sprite_load(gfx_context_t *ctxt, char *filename);

SDL_Texture *gfx_sprite_create(gfx_context_t *ctxt, uint8_t *pixels, int width, int height);

void gfx_sprite_destroy(SDL_Texture *sprite);

void gfx_sprite_render(gfx_context_t *ctxt, SDL_Texture *sprite, int x, int y, int sprite_width, int sprite_height);

void gfx_present(gfx_context_t *ctxt);

SDL_Keycode gfx_keypressed();

#endif
