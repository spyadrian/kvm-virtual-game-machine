/// @file gfx.c
/// @author Florent Gluck
/// @date 2016-2024
/// Helper routines for super simple graphics rendering.
/// Requires the SDL2 library.

#include "gfx.h"
#include <signal.h>

/// Create a fullscreen graphic window.
/// @param title window title.
/// @param width window's width in pixels.
/// @param height window's height in pixels.
/// @return a pointer to the graphic context or NULL if it failed.
gfx_context_t* gfx_create(char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "%s", SDL_GetError());
        goto error;
    }
    // Restore SIGINT's default behavior so that we can use CTRL-C to stop the program
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_DFL;
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror("sigaction");
		exit(1);
	}

    // These a just for reference:
    // SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");  // opengl, opengles2, software
    // SDL_SetHint(SDL_HINT_RENDER_OPENGL_SHADERS, "0");
    // SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");
    // SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_PING, "0");
    // SDL_SetHint(SDL_HINT_VIDEO_X11_XVIDMODE, "0");

    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *background_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    gfx_context_t *ctxt = malloc(sizeof(gfx_context_t));

    // Retrieve the background texture's pitch
    uint8_t *unused;
    SDL_LockTexture(background_texture, NULL, (void **)&unused, &ctxt->pitch);
    SDL_UnlockTexture(background_texture);

    pixel_t *background = malloc(ctxt->pitch*height);
    if (!window || !renderer || !background_texture || !background || !ctxt) goto error;

    ctxt->renderer = renderer;
    ctxt->background_texture = background_texture;
    ctxt->window = window;
    ctxt->width = width;
    ctxt->height = height;
    ctxt->background = background;

    SDL_ShowCursor(SDL_DISABLE);
    gfx_background_clear(ctxt, GFX_COL_BLACK);
    return ctxt;

error:
    return NULL;
}

/// Draw a pixel in the background buffer.
/// @param ctxt graphic context.
/// @param x x coordinate of the pixel.
/// @param y y coordinate of the pixel.
/// @param color pixel color.
void gfx_background_putpixel(gfx_context_t *ctxt, int x, int y, pixel_t color) {
    if (x < ctxt->width && y < ctxt->height) {
        ctxt->background[ctxt->pitch/sizeof(pixel_t)*y+x] = color;
    }
}

/// Clear the background buffer.
/// @param ctxt graphic context.
/// @param color fill color.
void gfx_background_clear(gfx_context_t *ctxt, pixel_t color) {
    for (int j = 0; j < ctxt->height; j++) {
        for (int i = 0; i < ctxt->width; i++) {
            ctxt->background[ctxt->pitch/sizeof(pixel_t)*j+i] = color;
        }
    }
}

/// Copy the background buffer to the display buffer.
/// @param ctxt graphic context.
void gfx_background_update(gfx_context_t *ctxt) {
    SDL_UpdateTexture(ctxt->background_texture, NULL, ctxt->background, ctxt->width*sizeof(pixel_t));
    SDL_RenderCopy(ctxt->renderer, ctxt->background_texture, NULL, NULL);
}

/// Show the display buffer.
/// @param ctxt graphic context.
void gfx_present(gfx_context_t *ctxt) {
    SDL_RenderPresent(ctxt->renderer);
}

/// Destroy a graphic window.
/// @param ctxt graphic context.
void gfx_destroy(gfx_context_t *ctxt) {
    SDL_ShowCursor(SDL_ENABLE);
    SDL_DestroyTexture(ctxt->background_texture);
    SDL_DestroyRenderer(ctxt->renderer);
    SDL_DestroyWindow(ctxt->window);
    free(ctxt->background);
    ctxt->background_texture = NULL;
    ctxt->renderer = NULL;
    ctxt->window = NULL;
    ctxt->background = NULL;
    SDL_Quit();
    free(ctxt);
}

/// If a key was pressed, returns its key code.
/// IMPORTANT: This is a non-blocking call!
/// List of key codes: https://wiki.libsdl.org/SDL_Keycode
/// @return the key that was pressed or 0 if none was pressed.
SDL_Keycode gfx_keypressed() {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN)
            return event.key.keysym.sym;
    }
    return 0;
}

/// Load a sprite from an image file (png, jpg, etc.).
/// @param ctxt graphic context.
/// @param filename path to the file to load.
/// @return a pointer to the sprite or NULL in case of failure.
/// When not needed anymore, deallocate it with gfx_sprite_destroy.
SDL_Texture *gfx_sprite_load(gfx_context_t *ctxt, char *filename) {
    SDL_Surface *sprite_surface = IMG_Load(filename);
    // Failed loading sprite.
    if (!sprite_surface) {
        return NULL;
    }

    SDL_Texture *sprite_texture = SDL_CreateTextureFromSurface(ctxt->renderer, sprite_surface);
    if (!sprite_texture) {
        return NULL;
    }
    SDL_FreeSurface(sprite_surface);  // Only the texture is needed

    return sprite_texture;
}

/// Create a sprite from in-memory RGBA8888 pixels.
/// @param ctxt graphic context.
/// @param pixels array of pixels composing the sprite.
/// @param width sprite's width in pixels.
/// @param height sprite's height in pixels.
/// @return a pointer to the sprite or NULL in case of failure.
/// When not needed anymore, deallocate it with gfx_sprite_destroy.
SDL_Texture *gfx_sprite_create(gfx_context_t *ctxt, uint8_t *pixels, int width, int height) {
    SDL_Texture *tex = SDL_CreateTexture(ctxt->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!tex) {
        return NULL;
    }
    
    // Force renderer to use alpha blending when rendering the sprite.
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    pixel_t *dst_pixels;
    pixel_t *src_pixels = (pixel_t *)pixels;
    int pitch;
    if (SDL_LockTexture(tex, NULL, (void **)&dst_pixels, &pitch) != 0) {
        SDL_DestroyTexture(tex);
        return NULL;
    }
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            dst_pixels[pitch/sizeof(pixel_t)*j+i] = *src_pixels;
            src_pixels++;
        }
    }
    SDL_UnlockTexture(tex);

    return tex;
}

/// Destroy a sprite that was loaded/created with gfx_sprite_load/gfx_sprite_create.
/// @param ctxt graphic context.
/// @param sprite the sprite (texture) to destroy.
void gfx_sprite_destroy(SDL_Texture *sprite) {
    SDL_DestroyTexture(sprite);
}

/// Render a sprite at the specified position.
/// @param context graphic context.
/// @param sprite the sprite (texture) to render.
/// @param x sprite's x coordinate.
/// @param y sprite's y coordinate.
/// @param sprite_width sprite's display width in pixels.
/// @param sprite_height sprite's display height in pixels.
void gfx_sprite_render(gfx_context_t *ctxt, SDL_Texture *sprite, int x, int y, int sprite_width, int sprite_height) {
    SDL_Rect dst_rect = { x, y, sprite_width, sprite_height };
    SDL_RenderCopy(ctxt->renderer, sprite, NULL, &dst_rect);
}
