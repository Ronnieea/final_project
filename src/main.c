#include <SDL2/SDL.h>
#include <string.h>
#include "parseToml.h"
#include "toml.h"

int main()
{
    FILE *pFile = NULL;
    if ((pFile = fopen("script.toml", "r")) == NULL)
    {
        printf("File could not be opened!\n");
        return 1;
    }

    char errbuf[200] = {0};
    toml_table_t *table = toml_parse_file(pFile, errbuf, sizeof(errbuf));
    fclose(pFile);
    if (!table)
    {
        printf("Parsing error: %s\n", errbuf);
        return 1;
    }

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

    for (int32_t i = 0; i < MAX_SCENES; i++)
    {
        scenes[i] = initial_scenes(scenes[i]);
    }
    for (int32_t i = 0; i < MAX_CHARACTERS; i++)
    {
        characters[i] = initial_character(characters[i]);
    }
    for (int32_t i = 0; i < MAX_EVENTS; i++)
    {
        events[i] = initial_event(events[i]);
    }
    for (int32_t i = 0; i < MAX_ITEMS; i++)
    {
        items[i] = initial_item(items[i]);
    }
    for (int32_t i = 0; i < MAX_DIALOGUE; i++)
    {
        dialogues[i] = initial_dialogue(dialogues[i]);
    }

    load_data(table, scenes, characters, events, dialogues, items, options, &scenes_count, &characters_count, &events_count, &dialogues_count, &items_count, &options_count);
    toml_free(table);

    // 顯示解析結果
    for (int i = 0; i < scenes_count; i++)
    {
        printf("scene %s: %s, %s\n", scenes[i].key, scenes[i].name, scenes[i].background);
    }
    for (int i = 0; i < characters_count; i++)
    {
        printf("角色 %s: %s, %s, %s\n", characters[i].key, characters[i].name, characters[i].avatar, characters[i].tachie);
    }
    for (int i = 0; i < events_count; i++)
    {
        printf("事件 %s: 對話=%s, 場景=%s\n", events[i].key, events[i].dialogue, events[i].scene);
    }
    for (int i = 0; i < dialogues_count; i++)
    {
        printf("對話 %s: 角色=%s, 文字=%s\n", dialogues[i].key, dialogues[i].character, dialogues[i].text);
        for (int j = 0; j < dialogues[i].options_count; j++)
        {
            printf("  選項 %d: %s, 下一步=%s\n", j + 1, dialogues[i].options[j].text, dialogues[i].options[j].next);
        }
    }

    for (int32_t i = 0; i < MAX_SCENES; i++)
    {
        free_scene(scenes[i]);
    }
    for (int32_t i = 0; i < MAX_CHARACTERS; i++)
    {
        free_character(characters[i]);
    }
    for (int32_t i = 0; i < MAX_EVENTS; i++)
    {
        free_event(events[i]);
    }
    for (int32_t i = 0; i < MAX_ITEMS; i++)
    {
        free_item(items[i]);
    }
    for (int32_t i = 0; i < MAX_DIALOGUE; i++)
    {
        free_dialogue(dialogues[i]);
    }
    return 0;
}