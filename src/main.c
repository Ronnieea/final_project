#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "parseToml.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "toml.h"

#define MAX_SCENES 50
#define MAX_CHARACTERS 10
#define MAX_EVENTS 50
#define MAX_DIALOGUE 50
#define MAX_ITEMS 10
#define MAX_OPTIONS 10

SDL_Texture *current_background_texture = NULL;
SDL_Texture *current_character_texture = NULL;
TTF_Font *font = NULL;

// SDL初始化
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

    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24); // 調整字體路徑和大小
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

// SDL清理
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

void display_image(SDL_Renderer *renderer, SDL_Texture *background, SDL_Texture *character, SDL_Texture *text_texture)
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
    if (character != NULL)
    {
        int char_w, char_h;
        SDL_QueryTexture(character, NULL, NULL, &char_w, &char_h);
        SDL_Rect dest_rect = {(w - char_w) / 2, h / 4 - char_h / 2, char_w, char_h};
        SDL_RenderCopy(renderer, character, NULL, &dest_rect);
    }

    // 繪製台詞區域（半透明白色）
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect text_box = {w / 8, h * 3 / 4, w * 3 / 4, h / 4};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180); // 白色，透明度180
    SDL_RenderFillRect(renderer, &text_box);

    // 繪製台詞文字
    if (text_texture != NULL)
    {
        int text_w, text_h;
        SDL_QueryTexture(text_texture, NULL, NULL, &text_w, &text_h);
        SDL_Rect text_rect = {w / 8 + 10, h * 3 / 4 + 10, text_w, text_h};
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
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
                printf("  Option[%d]: Text: %s, Next: %s, Event: %s\n", j, dialogues[i].options[j].text, dialogues[i].options[j].next, dialogues[i].options[j].event);
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

Dialogue *process_dialogue(Dialogue *current_dialogue, Dialogue dialogues[], uint16_t dialogues_count, Event events[], uint16_t events_count, Event **next_event, Scene scenes[], uint16_t scenes_count, Character characters[], uint16_t characters_count, SDL_Renderer *renderer)
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
                    SDL_DestroyTexture(current_character_texture);
                }
                current_character_texture = new_character_texture;
            }
        }
        else
        {
            perror("getcwd() error");
        }
    }

    SDL_Color textColor = {0, 0, 0, 255}; // 黑色
    SDL_Texture *text_texture = render_text(renderer, current_dialogue->text, textColor);

    display_image(renderer, current_background_texture, current_character_texture, text_texture);

    printf("Dialogue: %s\n", current_dialogue->text);

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
            }
        }

        if (option_index >= 0 && option_index < current_dialogue->options_count)
        {
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
            return NULL;
        }
    }
    else if (current_dialogue->options_count == 1)
    {
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
            SDL_Texture *new_background_texture = load_texture(renderer, full_path);
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
    Player player;
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
    uint16_t inventory_count = 0;

    // Initialize SDL
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (!init_sdl(&window, &renderer))
    {
        return 1;
    }

    // Initialize structures
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
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        items[i] = initial_item(items[i]);
    }

    toml_table_t *config = parseToml(filePath);
    if (config != NULL)
    {
        load_data(config, &player, scenes, characters, events, dialogues, items, options, &inventory_count, &scenes_count, &characters_count, &events_count, &dialogues_count, &items_count, &options_count);

        // Print all data with labels
        print_all_data(scenes, scenes_count, characters, characters_count, events, events_count, dialogues, dialogues_count, items, items_count);

        // Start from the event with key "start"
        Event *current_event = find_event_by_key(events, events_count, "start");
        Dialogue *current_dialogue = NULL;
        Event *next_event = NULL;

        if (current_event != NULL)
        {
            current_dialogue = process_event(current_event, dialogues, dialogues_count, scenes, scenes_count, renderer);
        }

        while (current_dialogue != NULL || next_event != NULL)
        {
            if (current_dialogue != NULL)
            {
                current_dialogue = process_dialogue(current_dialogue, dialogues, dialogues_count, events, events_count, &next_event, scenes, scenes_count, characters, characters_count, renderer);
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
                break;
            }
        }

        // Free allocated memory
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

        // Free the parsed TOML structure
        toml_free(config);
    }
    else
    {
        printf("Failed to parse the TOML file.\n");
    }

    // Cleanup SDL
    cleanup_sdl(window, renderer);

    return 0;
}
