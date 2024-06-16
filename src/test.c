#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parseToml.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_ITEMS 10

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;

int show_overlay = 0;

int init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL 無法初始化！SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("Item Icons", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == NULL)
    {
        printf("無法創建視窗！SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("無法創建渲染器！SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    if (TTF_Init() == -1)
    {
        printf("TTF 無法初始化！TTF_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48); // 使用系統默認字體路徑
    if (font == NULL)
    {
        printf("無法加載字體！TTF_Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    return 1;
}

void cleanup_sdl()
{
    if (font)
    {
        TTF_CloseFont(font);
    }
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_Texture *load_texture(const char *path)
{
    SDL_Surface *loaded_surface = IMG_Load(path);
    if (loaded_surface == NULL)
    {
        printf("無法載入圖片 %s！SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if (texture == NULL)
    {
        printf("無法創建紋理 %s！SDL_Error: %s\n", path, SDL_GetError());
    }
    SDL_FreeSurface(loaded_surface);
    return texture;
}

void display_image(SDL_Texture **textures, int texture_count)
{
    SDL_RenderClear(renderer);

    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    // 绘制背包白框
    if (show_overlay)
    {
        SDL_Rect overlay_rect = {w / 4, h / 4, w / 2, h / 2};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220); // 白色，透明度220
        SDL_RenderFillRect(renderer, &overlay_rect);

        int icon_size = 150; // 增加圖像大小
        int x_offset = w / 4 + 10;
        int y_offset = h / 4 + 10;
        int icons_per_row = (w / 2 - 20) / icon_size; // 每行排列的圖像數量

        for (int i = 0; i < texture_count; i++)
        {
            if (textures[i] != NULL)
            {
                if (i > 0 && i % icons_per_row == 0) // 換行
                {
                    y_offset += icon_size + 10;
                    x_offset = w / 4 + 10;
                }
                SDL_Rect dest_rect = {x_offset, y_offset, icon_size, icon_size};
                SDL_RenderCopy(renderer, textures[i], NULL, &dest_rect);
                x_offset += icon_size + 10;
            }
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    char base_path[256];
    if (getcwd(base_path, sizeof(base_path)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }

    char filePath[512];
    snprintf(filePath, sizeof(filePath), "%s/example_game/script.toml", base_path);

    Item items[MAX_ITEMS];
    uint16_t items_count = 0;

    // 初始化項目
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        items[i] = initial_item(items[i]);
    }

    FILE *pFile = NULL;
    if ((pFile = fopen(filePath, "r")) == NULL)
    {
        printf("無法打開文件：%s\n", filePath);
        return 1;
    }

    char errbuf[200] = {0};
    toml_table_t *config = toml_parse_file(pFile, errbuf, sizeof(errbuf));
    fclose(pFile);
    if (!config)
    {
        printf("解析錯誤: %s\n", errbuf);
        return 1;
    }

    if (config != NULL)
    {
        // 調用load_data函數
        Player player;
        Scene scenes[MAX_SCENES];
        Character characters[MAX_CHARACTERS];
        Event events[MAX_EVENTS];
        Dialogue dialogues[MAX_DIALOGUE];
        Option options[MAX_OPTIONS];
        uint16_t inventory_count = 0;
        uint16_t scenes_count = 0;
        uint16_t characters_count = 0;
        uint16_t events_count = 0;
        uint16_t dialogues_count = 0;
        uint16_t options_count = 0;

        // 初始化其它結構體
        player = initial_player(player);
        for (int i = 0; i < MAX_SCENES; i++)
        {
            scenes[i] = initial_scenes(scenes[i]);
        }
        for (int i = 0; i < MAX_CHARACTERS; i++)
        {
            characters[i] = initial_character(characters[i]);
        }
        for (int i = 0; i < MAX_EVENTS; i++)
        {
            events[i] = initial_event(events[i]);
        }
        for (int i = 0; i < MAX_DIALOGUE; i++)
        {
            dialogues[i] = initial_dialogue(dialogues[i]);
        }

        load_data(config, &player, scenes, characters, events, dialogues, items, options, &inventory_count, &scenes_count, &characters_count, &events_count, &dialogues_count, &items_count, &options_count);

        // 初始化 SDL
        if (!init_sdl())
        {
            return 1;
        }

        // 加載所有的圖標
        SDL_Texture *textures[MAX_ITEMS];
        for (int i = 0; i < items_count; i++)
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", base_path, items[i].icon);
            textures[i] = load_texture(full_path);
            if (textures[i] == NULL)
            {
                printf("無法載入圖片 %s\n", full_path);
            }
        }

        // 處理事件循環
        SDL_Event e;
        int quit = 0;

        while (!quit)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = 1;
                }
                else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_t:
                        show_overlay = !show_overlay;
                        break;
                    }
                }
            }

            display_image(textures, items_count);

            SDL_Delay(100); // 調整更新頻率
        }

        // 釋放已分配的記憶體
        free_player(player);
        for (uint16_t i = 0; i < scenes_count; i++)
        {
            free_scene(scenes[i]);
        }
        for (uint16_t i = 0; i < characters_count; i++)
        {
            free_character(characters[i]);
        }
        for (uint16_t i = 0; i < events_count; i++)
        {
            free_event(events[i]);
        }
        for (uint16_t i = 0; i < dialogues_count; i++)
        {
            free_dialogue(dialogues[i]);
        }
        for (uint16_t i = 0; i < items_count; i++)
        {
            free_item(items[i]);
        }

        // 釋放解析的 TOML 結構
        toml_free(config);

        // 清理 SDL
        cleanup_sdl();
    }
    else
    {
        printf("解析 TOML 文件失敗。\n");
    }

    return 0;
}

