#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "parseToml.h"
#include "save_game.h"
#include "audio.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_SCENES 50
#define MAX_CHARACTERS 10
#define MAX_EVENTS 50
#define MAX_DIALOGUE 50
#define MAX_ITEMS 10
#define MAX_OPTIONS 10
#define MAX_COLLECTED_ITEMS 50 // 新增

SDL_Texture *current_background_texture = NULL;
SDL_Texture *old_background_texture = NULL;
SDL_Texture *current_character_texture = NULL;
TTF_Font *font = NULL;
int show_overlay = 0;
char *saveFilePath = "save_game.json"; // 保存檔案地址
char current_background_path[512];
int current_sound = 0;
SDL_Texture *item_textures[MAX_ITEMS];
int items_count = 0;
int mood = 50; // 心情初始值
Item items[MAX_ITEMS]; // 添加 items 数组以存储物品信息

// 新增
char collected_items[MAX_COLLECTED_ITEMS][50];
int collected_items_count = 0;

int init_sdl(SDL_Window **window, SDL_Renderer **renderer)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL無法初始化！SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    *window = SDL_CreateWindow("背景顯示", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (*window == NULL)
    {
        printf("無法創建視窗！SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL)
    {
        printf("無法創建渲染器！SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    if (TTF_Init() == -1)
    {
        printf("TTF無法初始化！TTF_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48); // 调整字体大小
    if (font == NULL)
    {
        printf("無法加載字體！TTF_Error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    return 1;
}

void cleanup_sdl(SDL_Window *window, SDL_Renderer *renderer)
{
    if (current_background_texture)
    {
        SDL_DestroyTexture(current_background_texture);
    }
    if (current_character_texture)
    {
        SDL_DestroyTexture(current_character_texture);
    }
    for (int i = 0; i < items_count; i++)
    {
        if (item_textures[i])
        {
            SDL_DestroyTexture(item_textures[i]);
        }
    }
    if (font)
    {
        TTF_CloseFont(font);
    }
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path)
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
        printf("無法從 %s 創建紋理！SDL Error: %s\n", path, SDL_GetError());
    }
    SDL_FreeSurface(loaded_surface);
    return texture;
}

SDL_Texture *render_text(SDL_Renderer *renderer, const char *text, SDL_Color color)
{
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text, color);
    if (text_surface == NULL)
    {
        printf("無法渲染文字表面！TTF_Error: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (texture == NULL)
    {
        printf("無法從渲染的文字創建紋理！SDL_Error: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(text_surface);
    return texture;
}

void fade_out(SDL_Renderer *renderer, SDL_Texture *texture)
{
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    for (int alpha = 255; alpha >= 0; alpha -= 5)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (texture != NULL)
        {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(texture, alpha);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(10); // 調整延遲以控制動畫速度
    }
}

void fade_in(SDL_Renderer *renderer, SDL_Texture *texture)
{
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    for (int alpha = 0; alpha <= 255; alpha += 5)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (texture != NULL)
        {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(texture, alpha);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(10); // 調整延遲以控制動畫速度
    }
}

void display_image(SDL_Renderer *renderer, SDL_Texture *background, SDL_Texture *character, SDL_Texture *text_texture, Dialogue *current_dialogue)
{
    SDL_RenderClear(renderer);

    // 繪製背景
    if (background != NULL)
    {
        SDL_RenderCopy(renderer, background, NULL, NULL);
    }

    // 獲取視窗大小
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    // 繪製角色（縮放並居中在上方）
    SDL_Rect dest_rect_cpy = {0};
    if (character != NULL)
    {
        int char_w, char_h;
        SDL_QueryTexture(character, NULL, NULL, &char_w, &char_h);
        char_w += 350;
        char_h += 350;
        SDL_Rect dest_rect = {(w - char_w) / 2, h * 3 / 4 - char_h, char_w, char_h};
        SDL_RenderCopy(renderer, character, NULL, &dest_rect);
        dest_rect_cpy = dest_rect;
    }

    // 繪製台詞區域（全寬半透明白色）
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect text_box = {0, h * 3 / 4, w, h / 4};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180); // 白色，透明度180
    SDL_RenderFillRect(renderer, &text_box);

    // 繪製台詞文字
    SDL_Rect text_rect_cpy = {0};
    if (text_texture != NULL)
    {
        int text_w, text_h;
        SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
        if (text_w > w)
        {
            text_w = w - 10;
        }
        SDL_Rect text_rect = {10, h * 3 / 4 + 10, text_w, text_h};
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        for (int alpha = 0; alpha <= 255; alpha += 5)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, background, NULL, NULL);
            // 繪製台詞區域（全寬半透明白色）
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_Rect text_box = {0, h * 3 / 4, w, h / 4};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180); // 白色，透明度180
            SDL_RenderFillRect(renderer, &text_box);
            if (character != NULL)
            {
                SDL_SetTextureBlendMode(character, SDL_BLENDMODE_BLEND);
                SDL_SetTextureAlphaMod(character, alpha);
                SDL_RenderCopy(renderer, character, NULL, &dest_rect_cpy);
            }
            if (text_texture != NULL)
            {
                SDL_SetTextureBlendMode(text_texture, SDL_BLENDMODE_BLEND);
                SDL_SetTextureAlphaMod(text_texture, alpha);
                SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(10); // 調整延遲以控制動畫速度
        }
        text_rect_cpy = text_rect;
    }

    // 繪製選項文字
    if (current_dialogue->options_count > 0)
    {
        for (uint8_t i = 0; i < current_dialogue->options_count; i++)
        {
            char option_text[512];
            snprintf(option_text, sizeof(option_text), "%c. %s", 'A' + i, current_dialogue->options[i].text);
            SDL_Texture *option_texture = render_text(renderer, option_text, (SDL_Color){0, 0, 0, 255});
            if (option_texture != NULL)
            {
                int opt_w, opt_h;
                SDL_QueryTexture(option_texture, NULL, NULL, &opt_w, &opt_h);
                SDL_Rect opt_rect = {10, h * 3 / 4 + 10 + (i + 1) * (opt_h + 5), opt_w, opt_h};
                SDL_RenderCopy(renderer, option_texture, NULL, &opt_rect);
                SDL_DestroyTexture(option_texture);
            }
        }
    }

    // 绘制独立透明白框
    if (show_overlay)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect overlay_rect = {w / 4, h / 4, w / 2, h / 2};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220); // 白色，透明度220
        SDL_RenderFillRect(renderer, &overlay_rect);

        // 繪製圖標
        int icon_size = 100;                                                      // 圖標大小
        int margin = 10;                                                          // 圖標間距
        int icons_per_row = (overlay_rect.w - 2 * margin) / (icon_size + margin); // 每行圖標數量
        int x = overlay_rect.x + margin;
        int y = overlay_rect.y + margin;
        for (int i = 0; i < items_count; i++)
        {
            SDL_Rect icon_rect = {x, y, icon_size, icon_size};
            SDL_RenderCopy(renderer, item_textures[i], NULL, &icon_rect);
            x += icon_size + margin;
            if ((i + 1) % icons_per_row == 0)
            {
                x = overlay_rect.x + margin;
                y += icon_size + margin;
            }
        }
    }

    // 绘制右上角心情数值
    char mood_text[50];
    snprintf(mood_text, sizeof(mood_text), "Mood: %d", mood);
    SDL_Texture *mood_texture = render_text(renderer, mood_text, (SDL_Color){255, 0, 0, 255}); // 红色
    if (mood_texture != NULL)
    {
        int mood_w, mood_h;
        SDL_QueryTexture(mood_texture, NULL, NULL, &mood_w, &mood_h);
        SDL_Rect mood_rect = {w - mood_w - 10, 10, mood_w, mood_h};
        SDL_RenderCopy(renderer, mood_texture, NULL, &mood_rect);
        SDL_DestroyTexture(mood_texture);
    }

    // 绘制左上角背包内容
    SDL_Texture *backpack_texture = render_text(renderer, "Backpack:", (SDL_Color){255, 0, 0, 255}); // 红色
    if (backpack_texture != NULL)
    {
        int bp_w, bp_h;
        SDL_QueryTexture(backpack_texture, NULL, NULL, &bp_w, &bp_h);
        SDL_Rect bp_rect = {10, 10, bp_w, bp_h};
        SDL_RenderCopy(renderer, backpack_texture, NULL, &bp_rect);
        SDL_DestroyTexture(backpack_texture);
    }

    int bp_y_offset = 60; // 位置下移
    for (int i = 0; i < collected_items_count; i++)
    {
        SDL_Texture *item_name_texture = render_text(renderer, collected_items[i], (SDL_Color){255, 0, 0, 255}); // 红色
        if (item_name_texture != NULL)
        {
            int item_w, item_h;
            SDL_QueryTexture(item_name_texture, NULL, NULL, &item_w, &item_h);
            SDL_Rect item_rect = {10, bp_y_offset, item_w, item_h};
            SDL_RenderCopy(renderer, item_name_texture, NULL, &item_rect);
            bp_y_offset += item_h + 5; // 间隔
            SDL_DestroyTexture(item_name_texture);
        }
    }

    SDL_RenderPresent(renderer);
}

void display_backpack_items(SDL_Renderer *renderer, SDL_Texture *background, Item items[], const char **inventory, int inventory_count)
{
    SDL_RenderClear(renderer);

    // 绘制背景
    if (background != NULL)
    {
        SDL_RenderCopy(renderer, background, NULL, NULL);
    }

    // 获取窗口大小
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    int x_offset = 50;
    int y_offset = 50;

    for (int i = 0; i < inventory_count; i++)
    {
        for (int j = 0; j < items_count; j++)
        {
            if (strcmp(inventory[i], items[j].key) == 0)
            {
                SDL_Texture *item_texture = item_textures[j];
                if (item_texture != NULL)
                {
                    int item_w, item_h;
                    SDL_QueryTexture(item_texture, NULL, NULL, &item_w, &item_h);
                    SDL_Rect item_rect = {x_offset, y_offset, item_w, item_h};
                    SDL_RenderCopy(renderer, item_texture, NULL, &item_rect);
                    y_offset += item_h + 10; // 间隔

                    if (y_offset + item_h > h)
                    {
                        y_offset = 50;
                        x_offset += item_w + 10; // 换列
                    }
                }
            }
        }
    }

    SDL_RenderPresent(renderer);
}

Dialogue *find_dialogue_by_key(Dialogue dialogues[], uint16_t dialogues_count, const char *key)
{
    for (uint16_t i = 0; i < dialogues_count; i++)
    {
        if (strcmp(dialogues[i].key, key) == 0)
        {
            return &dialogues[i];
        }
    }
    return NULL;
}

Event *find_event_by_key(Event events[], uint16_t events_count, const char *key)
{
    for (uint16_t i = 0; i < events_count; i++)
    {
        if (strcmp(events[i].key, key) == 0)
        {
            return &events[i];
        }
    }
    return NULL;
}

Scene *find_scene_by_key(Scene scenes[], uint16_t scenes_count, const char *key)
{
    for (uint16_t i = 0; i < scenes_count; i++)
    {
        if (strcmp(scenes[i].key, key) == 0)
        {
            return &scenes[i];
        }
    }
    return NULL;
}

Character *find_character_by_key(Character characters[], uint16_t characters_count, const char *key)
{
    for (uint16_t i = 0; i < characters_count; i++)
    {
        if (strcmp(characters[i].key, key) == 0)
        {
            return &characters[i];
        }
    }
    return NULL;
}

void print_all_data(Scene scenes[], uint16_t scenes_count, Character characters[], uint16_t characters_count, Event events[], uint16_t events_count, Dialogue dialogues[], uint16_t dialogues_count, Item items[], uint16_t items_count)
{
    char current_path[256];
    if (getcwd(current_path, sizeof(current_path)) != NULL)
    {
        // 打印場景資料
        for (uint16_t i = 0; i < scenes_count; i++)
        {
            printf("Scene[%d]: Key: %s, Name: %s, Background: %s/%s\n", i, scenes[i].key, scenes[i].name, current_path, scenes[i].background);
        }

        // 打印角色資料
        for (uint16_t i = 0; i < characters_count; i++)
        {
            printf("Character[%d]: Key: %s, Name: %s, Tachie: %s/%s\n", i, characters[i].key, characters[i].name, current_path, characters[i].tachie);
        }

        // 打印事件資料
        for (uint16_t i = 0; i < events_count; i++)
        {
            printf("Event[%d]: Key: %s, Scene: %s, Dialogue: %s\n", i, events[i].key, events[i].scene, events[i].dialogue);
        }

        // 打印對話資料
        for (uint16_t i = 0; i < dialogues_count; i++)
        {
            printf("Dialogue[%d]: Key: %s, Character: %s, Item: %s, Text: %s\n", i, dialogues[i].key, dialogues[i].character, dialogues[i].item, dialogues[i].text);
            for (uint8_t j = 0; j < dialogues[i].options_count; j++)
            {
                printf("  Option[%d]: Text: %s, Next: %s, Event: %s, Effect: %d\n", j, dialogues[i].options[j].text, dialogues[i].options[j].next, dialogues[i].options[j].event, dialogues[i].options[j].effect); // 修改这里以打印effect字段
            }
        }

        // 打印物品資料
        for (uint16_t i = 0; i < items_count; i++)
        {
            printf("Item[%d]: Key: %s, Name: %s, Icon: %s\n", i, items[i].key, items[i].name, items[i].icon);
        }
    }
    else
    {
        perror("getcwd() error");
    }
}

Dialogue *process_dialogue(Dialogue *current_dialogue, Dialogue dialogues[], uint16_t dialogues_count, Event events[], uint16_t events_count, Event **next_event, Scene scenes[], uint16_t scenes_count, Character characters[], uint16_t characters_count, SDL_Renderer *renderer, int *ending)
{
    Character *character = find_character_by_key(characters, characters_count, current_dialogue->character);
    if (character != NULL)
    {
        char current_path[256];
        if (getcwd(current_path, sizeof(current_path)) != NULL)
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, character->tachie);
            SDL_Texture *new_character_texture = load_texture(renderer, full_path);
            if (new_character_texture != NULL)
            {
                if (current_character_texture != NULL)
                {
                    if (strcmp(current_dialogue->character, "witch") == 0)
                    {
                        stop_sound(3);
                    }
                    SDL_DestroyTexture(current_character_texture);
                }
                current_character_texture = new_character_texture;
                if (strcmp(current_dialogue->character, "witch") == 0)
                {
                    stop_sound(current_sound);
                    play_sound(3);
                    current_sound = 3;
                }
            }
        }
        else
        {
            perror("getcwd() error");
        }
    }

    SDL_Color textColor = {0, 0, 0, 255}; // 黑色
    SDL_Texture *text_texture = render_text(renderer, current_dialogue->text, textColor);

    display_image(renderer, current_background_texture, current_character_texture, text_texture, current_dialogue);

    printf("Dialogue: %s\n", current_dialogue->text);

    // 打印對話中的物品
    if (current_dialogue->item != NULL)
    {
        printf("Item in Dialogue: %s\n", current_dialogue->item);
        int item_exists = 0;
        for (int i = 0; i < collected_items_count; i++)
        {
            if (strcmp(collected_items[i], current_dialogue->item) == 0)
            {
                item_exists = 1;
                break;
            }
        }
        if (!item_exists && collected_items_count < MAX_COLLECTED_ITEMS)
        {
            strcpy(collected_items[collected_items_count], current_dialogue->item);
            collected_items_count++;
        }
    }

    if (text_texture != NULL)
    {
        SDL_DestroyTexture(text_texture);
    }

    if (current_dialogue->options_count > 1)
    {
        printf("Options:\n");
        for (uint8_t i = 0; i < current_dialogue->options_count; i++)
        {
            printf("%c. %s\n", 'A' + i, current_dialogue->options[i].text);
        }

        SDL_Event e;
        int option_index = -1;
        while (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                save_game(&saveFilePath, *next_event, current_dialogue, show_overlay, current_background_path, current_sound);
                exit(0);
            }
            else if (e.type == SDL_KEYDOWN)
            {
                char choice = e.key.keysym.sym;
                if (choice >= SDLK_a && choice <= SDLK_c)
                {
                    option_index = choice - SDLK_a;
                    break;
                }
                else if (choice == SDLK_t)
                {
                    show_overlay = !show_overlay;
                    display_image(renderer, current_background_texture, current_character_texture, text_texture, current_dialogue); // 更新畫面以顯示或隱藏白框
                }
            }
        }

        if (option_index >= 0 && option_index < current_dialogue->options_count)
        {
            mood += current_dialogue->options[option_index].effect; // 修改mood值
            if (current_dialogue->options[option_index].next && current_dialogue->options[option_index].next[0] != '\0')
            {
                Dialogue *next_dialogue = find_dialogue_by_key(dialogues, dialogues_count, current_dialogue->options[option_index].next);
                if (next_dialogue != NULL)
                {
                    return next_dialogue;
                }
                else
                {
                    *next_event = find_event_by_key(events, events_count, current_dialogue->options[option_index].next);
                    return NULL;
                }
            }
            else if (current_dialogue->options[option_index].event && current_dialogue->options[option_index].event[0] != '\0')
            {
                *next_event = find_event_by_key(events, events_count, current_dialogue->options[option_index].event);
                return NULL;
            }
        }
        else
        {
            printf("Invalid choice. Exiting.\n");
            *ending = 0;
            return NULL;
        }
    }
    else if (current_dialogue->options_count == 1)
    {
        printf("Options:\n");
        for (uint8_t i = 0; i < current_dialogue->options_count; i++)
        {
            printf("%c. %s\n", 'A' + i, current_dialogue->options[i].text);
        }

        SDL_Event e;
        int option_index = -1;
        while (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                save_game(&saveFilePath, *next_event, current_dialogue, show_overlay, current_background_path, current_sound);
                exit(0);
            }
            else if (e.type == SDL_KEYDOWN)
            {
                char choice = e.key.keysym.sym;
                if (choice >= SDLK_a && choice <= SDLK_c)
                {
                    option_index = choice - SDLK_a;
                    break;
                }
                else if (choice == SDLK_t)
                {
                    show_overlay = !show_overlay;
                    display_image(renderer, current_background_texture, current_character_texture, text_texture, current_dialogue); // 更新畫面以顯示或隱藏白框
                }
            }
        }
        if (option_index >= 0 && option_index < current_dialogue->options_count)
        {
            mood += current_dialogue->options[0].effect; // 修改mood值
            if (current_dialogue->options[0].next && current_dialogue->options[0].next[0] != '\0')
            {
                Dialogue *next_dialogue = find_dialogue_by_key(dialogues, dialogues_count, current_dialogue->options[0].next);
                if (next_dialogue != NULL)
                {
                    return next_dialogue;
                }
                else
                {
                    *next_event = find_event_by_key(events, events_count, current_dialogue->options[0].next);
                    return NULL;
                }
            }
            else if (current_dialogue->options[0].event && current_dialogue->options[0].event[0] != '\0')
            {
                *next_event = find_event_by_key(events, events_count, current_dialogue->options[0].event);
                return NULL;
            }
        }
        else
        {
            printf("Invalid choice. Exiting.\n");
            *ending = 0;
            return NULL;
        }
    }
    else
    {
        SDL_Event e;
        while (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                exit(0);
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
            {
                return NULL;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_t)
            {
                show_overlay = !show_overlay;
                display_image(renderer, current_background_texture, current_character_texture, text_texture, current_dialogue); // 更新畫面以顯示或隱藏白框
            }
        }
    }

    return NULL;
}

Dialogue *process_event(Event *current_event, Dialogue dialogues[], uint16_t dialogues_count, Scene scenes[], uint16_t scenes_count, SDL_Renderer *renderer)
{
    printf("Event: %s\n", current_event->key);
    Scene *scene = find_scene_by_key(scenes, scenes_count, current_event->scene);
    if (scene != NULL)
    {
        char current_path[256];
        if (getcwd(current_path, sizeof(current_path)) != NULL)
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, scene->background);
            strncpy(current_background_path, full_path, 512);
            SDL_Texture *new_background_texture = load_texture(renderer, current_background_path);
            if (new_background_texture != NULL)
            {
                if (current_background_texture != NULL)
                {
                    stop_sound(current_sound);
                    fade_out(renderer, current_background_texture);
                    SDL_DestroyTexture(current_background_texture);
                }
                current_background_texture = new_background_texture;
                fade_in(renderer, current_background_texture);
                if (strcmp(current_event->scene, "forest") == 0)
                {
                    stop_sound(current_sound);
                    play_sound(1);
                    current_sound = 1;
                }
                else if (strcmp(current_event->scene, "market") == 0)
                {
                    stop_sound(current_sound);
                    play_sound(0);
                    current_sound = 0;
                }
                else if (strcmp(current_event->scene, "cave") == 0)
                {
                    stop_sound(current_sound);
                    play_sound(2);
                    current_sound = 2;
                }
                else if (strcmp(current_event->scene, "castle") == 0)
                {
                    stop_sound(current_sound);
                    play_sound(4);
                    current_sound = 4;
                }
            }
        }
        else
        {
            perror("getcwd() error");
        }
    }
    return find_dialogue_by_key(dialogues, dialogues_count, current_event->dialogue);
}

int main(int argc, char *argv[])
{
    const char *filePath;
    if (argc > 1)
    {
        filePath = argv[1];
    }
    else
    {
        filePath = getenv("TOML_FILE_PATH");
        if (filePath == NULL)
        {
            filePath = "script.toml"; // Default path
        }
    }

    // Initialize arrays and counters
    Player player; // 新增
    Scene scenes[MAX_SCENES];
    Character characters[MAX_CHARACTERS];
    Event events[MAX_EVENTS];
    Dialogue dialogues[MAX_DIALOGUE];
    Item items[MAX_ITEMS];
    Option options[MAX_OPTIONS];
    uint16_t scenes_count = 0;
    uint16_t characters_count = 0;
    uint16_t events_count = 0;
    uint16_t dialogues_count = 0;
    uint16_t items_count = 0;
    uint16_t options_count = 0;
    uint16_t inventory_count = 0; // 新增
    char current_path[256];
    char full_path[512];

    // Initialize SDL
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (!init_sdl(&window, &renderer))
    {
        return 1;
    }

    // Initialize OpenAL
    if (!init_openal())
    {
        cleanup_sdl(window, renderer);
        return 1;
    }

    // Initialize structures
    player = initial_player(player); // 新增
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
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        items[i] = initial_item(items[i]);
    }

    FILE *pFile = NULL;
    getcwd(current_path, sizeof(current_path));
    snprintf(full_path, sizeof(full_path), "%s/%s", current_path, "example_game/script.toml");
    if ((pFile = fopen(full_path, "r")) == NULL)
    {
        printf("File could not be opened!\n");
        return 1;
    }

    char errbuf[200] = {0};
    toml_table_t *config = toml_parse_file(pFile, errbuf, sizeof(errbuf));
    fclose(pFile);
    if (!config)
    {
        printf("Parsing error: %s\n", errbuf);
        return 1;
    }

    FILE *fpp = NULL;
    int ending = -1;
    if (config != NULL)
    {
        load_data(config, &player, scenes, characters, events, dialogues, items, options, &inventory_count, &scenes_count, &characters_count, &events_count, &dialogues_count, &items_count, &options_count); // 新增&player和&inventory_count

        // Print all data with labels
        print_all_data(scenes, scenes_count, characters, characters_count, events, events_count, dialogues, dialogues_count, items, items_count);

        // 加载图标纹理
        if (getcwd(current_path, sizeof(current_path)) != NULL)
        {
            for (int i = 0; i < items_count; i++)
            {
                snprintf(full_path, sizeof(full_path), "%s/%s", current_path, items[i].icon);
                item_textures[i] = load_texture(renderer, full_path);
                if (item_textures[i] == NULL)
                {
                    printf("无法加载图标纹理 %s\n", full_path);
                }
            }
        }

        // Start from the event with key "start"
        Event *current_event = find_event_by_key(events, events_count, "start");
        Dialogue *current_dialogue = NULL;
        Event *next_event = NULL;

        // 加載遊戲狀態
        if ((fpp = fopen(saveFilePath, "r")) != NULL)
        {
            fclose(fpp);
            load_game(saveFilePath, &current_event, &current_dialogue, &show_overlay, &current_sound, current_background_path, events, events_count, dialogues, dialogues_count);
            play_sound(current_sound);
            SDL_Texture *new_background_texture = load_texture(renderer, current_background_path);
            if (new_background_texture != NULL)
            {
                if (current_background_texture != NULL)
                {
                    SDL_DestroyTexture(current_background_texture);
                }
                current_background_texture = new_background_texture;
            }
        }
        else
        {
            // Before game started
            int w, h;
            SDL_GetRendererOutputSize(renderer, &w, &h);
            getcwd(current_path, sizeof(current_path));
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, "src/assets/Interative.bmp");
            SDL_Texture *start_background_texture = load_texture(renderer, full_path);
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, "src/assets/start_button.bmp");
            SDL_Texture *start_button = load_texture(renderer, full_path);
            SDL_RenderClear(renderer);
            int button_w, button_h;
            SDL_QueryTexture(start_button, NULL, NULL, &button_w, &button_h);
            SDL_Rect button_rect = {(w - button_w) / 2, h * 7 / 8 - button_h, button_w, button_h};
            for (int alpha = 0; alpha <= 255; alpha += 5)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                if (start_background_texture != NULL)
                {
                    SDL_SetTextureBlendMode(start_background_texture, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureAlphaMod(start_background_texture, alpha);
                    SDL_RenderCopy(renderer, start_background_texture, NULL, NULL);
                }

                if (start_button != NULL)
                {
                    SDL_SetTextureBlendMode(start_button, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureAlphaMod(start_button, alpha);
                    SDL_RenderCopy(renderer, start_button, NULL, &button_rect);
                }

                SDL_RenderPresent(renderer);
                SDL_Delay(10); // 調整延遲以控制動畫速度
            }
            play_sound(5);
            SDL_Event e;
            int quit = -1;
            while (quit != 1)
            {
                while (SDL_WaitEvent(&e))
                {
                    if (e.type == SDL_QUIT)
                    {
                        quit = 1;
                        break;
                    }
                    else if (e.type == SDL_MOUSEBUTTONDOWN)
                    {
                        int x, y;
                        SDL_GetMouseState(&x, &y);
                        if (x >= button_rect.x && x <= button_rect.x + button_rect.w && y >= button_rect.y && y <= button_rect.y + button_rect.h)
                        {
                            printf("Button clicked!\n");
                            quit = 1;
                            break;
                        }
                    }
                    else if (e.type == SDL_KEYDOWN)
                    {
                        switch (e.key.keysym.sym)
                        {
                        case SDLK_q:
                            quit = 1;
                            break;
                        }
                    }
                }
            }
            for (int alpha = 255; alpha >= 0; alpha -= 5)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                if (start_background_texture != NULL)
                {
                    SDL_SetTextureBlendMode(start_background_texture, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureAlphaMod(start_background_texture, alpha);
                    SDL_RenderCopy(renderer, start_background_texture, NULL, NULL);
                }

                if (start_button != NULL)
                {
                    SDL_SetTextureBlendMode(start_button, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureAlphaMod(start_button, alpha);
                    SDL_RenderCopy(renderer, start_button, NULL, &button_rect);
                }

                SDL_RenderPresent(renderer);
                SDL_Delay(10); // 調整延遲以控制動畫速度
            }
            SDL_DestroyTexture(start_background_texture);
            SDL_DestroyTexture(start_button);
            stop_sound(5);

            if (current_event != NULL)
            {
                current_dialogue = process_event(current_event, dialogues, dialogues_count, scenes, scenes_count, renderer);
            }
        }

        while (current_dialogue != NULL || next_event != NULL)
        {
            if (current_dialogue != NULL)
            {
                save_game(&saveFilePath, next_event, current_dialogue, show_overlay, current_background_path, current_sound);
                current_dialogue = process_dialogue(current_dialogue, dialogues, dialogues_count, events, events_count, &next_event, scenes, scenes_count, characters, characters_count, renderer, &ending);
                if (next_event != NULL)
                {
                    current_dialogue = NULL;
                }
            }
            else if (next_event != NULL)
            {
                current_dialogue = process_event(next_event, dialogues, dialogues_count, scenes, scenes_count, renderer);
                next_event = NULL;
            }
            else
            {
                printf("No more dialogues or events. Exiting.\n");
                SDL_Event e;
                while (SDL_WaitEvent(&e))
                {
                    if (e.type == SDL_QUIT)
                    {
                        // 在退出之前打印已收集的物品名稱
                        printf("Collected Items:\n");
                        for (int i = 0; i < collected_items_count; i++)
                        {
                            for (int j = 0; j < items_count; j++)
                            {
                                if (strcmp(collected_items[i], items[j].key) == 0)
                                {
                                    printf("%s\n", items[j].name);
                                    break;
                                }
                            }
                        }
                        cleanup_sdl(window, renderer);
                        getcwd(current_path, sizeof(current_path));
                        snprintf(full_path, sizeof(full_path), "%s/%s", current_path, "save_game.json");
                        if ((fpp = fopen(full_path, "r")) != NULL)
                        {
                            fclose(fpp);
                            remove(full_path);
                        }
                        return 0;
                    }
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
                    {
                        display_backpack_items(renderer, current_background_texture, items, (const char **)player.inventory, inventory_count);
                        while (SDL_WaitEvent(&e))
                        {
                            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
                            {
                                // 在退出之前打印已收集的物品名稱
                                printf("Collected Items:\n");
                                for (int i = 0; i < collected_items_count; i++)
                                {
                                    for (int j = 0; j < items_count; j++)
                                    {
                                        if (strcmp(collected_items[i], items[j].key) == 0)
                                        {
                                            printf("%s\n", items[j].name);
                                            break;
                                        }
                                    }
                                }
                                cleanup_sdl(window, renderer);
                                getcwd(current_path, sizeof(current_path));
                                snprintf(full_path, sizeof(full_path), "%s/%s", current_path, "save_game.json");
                                if ((fpp = fopen(full_path, "r")) != NULL)
                                {
                                    fclose(fpp);
                                    remove(full_path);
                                }
                                return 0;
                            }
                        }
                    }
                }
            }
        }

        // Free allocated memory
        free_player(player); // 新增
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

        // Free the parsed TOML structure
        toml_free(config);
    }
    else
    {
        printf("Failed to parse the TOML file.\n");
    }

    // Cleanup SDL
    cleanup_sdl(window, renderer);
    cleanup_openal();
    if (ending != 0)
    {
        getcwd(current_path, sizeof(current_path));
        snprintf(full_path, sizeof(full_path), "%s/%s", current_path, "save_game.json");
        if ((fpp = fopen(full_path, "r")) != NULL)
        {
            fclose(fpp);
            remove(full_path);
        }
    }

    return 0;
}
