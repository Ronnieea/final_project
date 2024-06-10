#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_COUNT 4

const char *background_files[] = {
    "example game/assets/market.bmp",
    "example game/assets/castle.bmp",
    "example game/assets/forest.bmp",
    "example game/assets/cave.bmp"
};

const char *character_files[] = {
    "example game/assets/merchant_tachie.bmp",
    "example game/assets/knight_tachie.bmp",
    "example game/assets/guard_tachie.bmp",
    "example game/assets/witch_tachie.bmp"
};

void get_full_path(char *full_path, const char *base_path, const char *relative_path) {
    strcpy(full_path, base_path);
    strcat(full_path, relative_path);
}

SDL_Texture* load_texture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* loaded_surface = IMG_Load(path);
    if (loaded_surface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if (texture == NULL) {
        printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
    }
    SDL_FreeSurface(loaded_surface);
    return texture;
}

SDL_Texture* render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
    if (text_surface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (texture == NULL) {
        printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(text_surface);
    return texture;
}

void display_image(SDL_Renderer* renderer, SDL_Texture* background, SDL_Texture* character, SDL_Texture* text_texture, int show_inventory) {
    SDL_RenderClear(renderer);

    // Render background without changing its alpha
    SDL_RenderCopy(renderer, background, NULL, NULL);

    // Get window size
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    // Render character (scaled and centered at upper middle)
    int char_w, char_h;
    SDL_QueryTexture(character, NULL, NULL, &char_w, &char_h);
    SDL_Rect dest_rect = { (w - char_w) / 2, (h - char_h) / 4, char_w, char_h };
    SDL_RenderCopy(renderer, character, NULL, &dest_rect);

    // Render text box (semi-transparent white)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect text_box = { 50, h - 150, w - 100, 100 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);  // White with 150 alpha
    SDL_RenderFillRect(renderer, &text_box);

    // Render text inside the text box
    int text_w, text_h;
    SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
    SDL_Rect text_rect = { 60, h - 140, text_w, text_h };
    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

    // Render inventory (semi-transparent white)
    if (show_inventory) {
        SDL_Rect inventory_box = { 100, 100, w - 200, h - 200 };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 230);  // White with 230 alpha
        SDL_RenderFillRect(renderer, &inventory_box);
    }

    SDL_RenderPresent(renderer);
}

void update_image(SDL_Renderer* renderer, const char* base_path, const char* background_file, const char* character_file, SDL_Texture* text_texture, int show_inventory) {
    char background_path[512];
    char character_path[512];

    get_full_path(background_path, base_path, background_file);
    get_full_path(character_path, base_path, character_file);

    SDL_Texture* background_texture = load_texture(renderer, background_path);
    SDL_Texture* character_texture = load_texture(renderer, character_path);

    if (background_texture && character_texture) {
        display_image(renderer, background_texture, character_texture, text_texture, show_inventory);
    }

    if (background_texture) {
        SDL_DestroyTexture(background_texture);
    }
    if (character_texture) {
        SDL_DestroyTexture(character_texture);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("背景切換", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_FULLSCREEN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Replace this with a valid path to a font file on your system
    // For example:
    // Linux: "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
    // macOS: "/Library/Fonts/Arial.ttf" or "/System/Library/Fonts/Supplemental/Arial.ttf"
    // Windows: "C:\\Windows\\Fonts\\arial.ttf"
    const char* font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";

    TTF_Font* font = TTF_OpenFont(font_path, 28);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Color textColor = { 0, 0, 0, 255 };  // Black color
    SDL_Texture* text_texture = render_text(renderer, font, "hahaha", textColor);
    if (text_texture == NULL) {
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    char base_path[256];
    if (realpath(argv[0], base_path) == NULL) {
        printf("Error resolving base path!\n");
        SDL_DestroyTexture(text_texture);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Remove executable name from base_path to get directory path
    char *last_slash = strrchr(base_path, '/');
    if (last_slash != NULL) {
        *(last_slash + 1) = '\0'; // Keep the trailing slash
    }

    SDL_Event e;
    int quit = 0;
    int current_image = 0;
    int show_inventory = 0;

    update_image(renderer, base_path, background_files[current_image], character_files[current_image], text_texture, show_inventory);

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    current_image++;
                    if (current_image >= IMAGE_COUNT) {
                        quit = 1;
                        break;
                    }
                    update_image(renderer, base_path, background_files[current_image], character_files[current_image], text_texture, show_inventory);
                } else if (e.key.keysym.sym == SDLK_b) {
                    show_inventory = !show_inventory;
                    update_image(renderer, base_path, background_files[current_image], character_files[current_image], text_texture, show_inventory);
                }
            }
        }
    }

    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
