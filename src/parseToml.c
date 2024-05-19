#include "toml.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "parseToml.h"

Scene initial_scenes(Scene scene)
{
    scene.key = malloc(50 * sizeof(char));
    scene.name = malloc(50 * sizeof(char));
    scene.background = malloc(100 * sizeof(char));
    return scene;
}

Character initial_character(Character character)
{
    character.key = malloc(50 * sizeof(char));
    character.name = malloc(50 * sizeof(char));
    character.avatar = malloc(100 * sizeof(char));
    character.tachie = malloc(100 * sizeof(char));
    return character;
}

Event initial_event(Event event)
{
    event.key = malloc(50 * sizeof(char));
    event.scene = malloc(50 * sizeof(char));
    event.dialogue = malloc(50 * sizeof(char));
    return event;
}

Dialogue initial_dialogue(Dialogue dialogue)
{
    dialogue.key = malloc(50 * sizeof(char));
    dialogue.character = malloc(50 * sizeof(char));
    dialogue.item = malloc(50 * sizeof(char));
    dialogue.text = malloc(200 * sizeof(char));
    dialogue.event = malloc(50 * sizeof(char));
    dialogue.options = malloc(10 * sizeof(Option));
    dialogue.options->event = malloc(50 * sizeof(char));
    dialogue.options->next = malloc(50 * sizeof(char));
    dialogue.options->text = malloc(200 * sizeof(char));
    return dialogue;
}

Item initial_item(Item item)
{
    item.key = malloc(50 * sizeof(char));
    item.name = malloc(50 * sizeof(char));
    item.icon = malloc(100 * sizeof(char));
    return item;
}

void free_scene(Scene scene)
{
    free(scene.key);
    free(scene.name);
    free(scene.background);
}

void free_character(Character character)
{
    free(character.key);
    free(character.name);
    free(character.avatar);
    free(character.tachie);
}

void free_event(Event event)
{
    free(event.key);
    free(event.dialogue);
    free(event.scene);
}

void free_dialogue(Dialogue dialogue)
{
    free(dialogue.key);
    free(dialogue.item);
    free(dialogue.character);
    free(dialogue.text);
    free(dialogue.event);
    free(dialogue.options->event);
    free(dialogue.options->next);
    free(dialogue.options->text);
    free(dialogue.options);
}

void free_item(Item item)
{
    free(item.key);
    free(item.name);
    free(item.icon);
}

void load_data(toml_table_t *config, Scene scenes[MAX_SCENES], Character characters[MAX_CHARACTERS], Event events[MAX_EVENTS], Dialogue dialogues[MAX_DIALOGUE], Item items[MAX_ITEMS], Option options[MAX_OPTIONS], uint16_t *scenes_count, uint16_t *characters_count, uint16_t *events_count, uint16_t *dialogues_count, uint16_t *items_count, uint16_t *options_count)
{
    toml_table_t *table = NULL;
    const char *key = NULL;
    char *value = NULL;

    // parse scenes
    table = toml_table_in(config, "scene");
    if (table)
    {
        for (int32_t i = 0; table == NULL || (key = toml_key_in(table, i)) != NULL; i++)
        {
            toml_table_t *subtable = toml_table_in(table, key);
            strcpy(scenes[*scenes_count].key, key);
            toml_rtos(toml_raw_in(subtable, "name"), &scenes[*scenes_count].name);
            toml_rtos(toml_raw_in(subtable, "background"), &scenes[*scenes_count].background);
            (*scenes_count) = *(scenes_count) + 1;
        }
    }

    // parse character
    table = toml_table_in(config, "character");
    if (table)
    {
        for (int32_t i = 0; (key = toml_key_in(table, i)) != NULL; i++)
        {
            toml_table_t *subtable = toml_table_in(table, key);
            strcpy(characters[*characters_count].key, key);
            toml_rtos(toml_raw_in(subtable, "name"), &characters[*characters_count].name);
            toml_rtos(toml_raw_in(subtable, "avatar"), &characters[*characters_count].avatar);
            toml_rtos(toml_raw_in(subtable, "tachie"), &characters[*characters_count].tachie);
            *(characters_count) = *(characters_count) + 1;
        }
    }

    // parse item
    table = toml_table_in(config, "item");
    if (table)
    {
        for (int32_t i = 0; (key = toml_key_in(table, i)) != NULL; i++)
        {
            toml_table_t *subtable = toml_table_in(table, key);
            strcpy(items[*items_count].key, key);
            toml_rtos(toml_raw_in(subtable, "name"), &items[*items_count].name);
            toml_rtos(toml_raw_in(subtable, "icon"), &items[*items_count].icon);
            *(items_count) = *(items_count) + 1;
        }
    }

    // parse event
    table = toml_table_in(config, "event");
    if (table)
    {
        for (int32_t i = 0; (key = toml_key_in(table, i)) != NULL; i++)
        {
            toml_table_t *subtable = toml_table_in(table, key);
            strcpy(events[*events_count].key, key);
            toml_rtos(toml_raw_in(subtable, "dialogue"), &events[*events_count].dialogue);
            toml_rtos(toml_raw_in(subtable, "scene"), &events[*events_count].scene);
            *(events_count) = *(events_count) + 1;
        }
    }

    // parse dialogue
    table = toml_table_in(config, "dialogue");
    if (table)
    {
        for (int32_t i = 0; (key = toml_key_in(table, i)) != NULL; i++)
        {
            toml_table_t *subtable = toml_table_in(table, key);
            strcpy(dialogues[*dialogues_count].key, key);
            toml_rtos(toml_raw_in(subtable, "character"), &dialogues[*dialogues_count].character);
            toml_rtos(toml_raw_in(subtable, "text"), &dialogues[*dialogues_count].text);
            toml_rtos(toml_raw_in(subtable, "event"), &dialogues[*dialogues_count].event);
            toml_rtos(toml_raw_in(subtable, "item"), &dialogues[*dialogues_count].item);

            // parse options
            toml_array_t *options = toml_array_in(subtable, "options");
            if (options)
            {
                uint8_t options_count = toml_array_nelem(options);
                dialogues[*dialogues_count].options_count = options_count;
                for (int32_t j = 0; j < options_count; j++)
                {
                    toml_table_t *option = toml_table_at(options, j);
                    toml_rtos(toml_raw_in(option, "text"), &dialogues[*dialogues_count].options[j].text);
                    toml_rtos(toml_raw_in(option, "next"), &dialogues[*dialogues_count].options[j].next);
                    toml_rtos(toml_raw_in(option, "event"), &dialogues[*dialogues_count].options[j].event);
                }
            }
            else
            {
                dialogues[*dialogues_count].options_count = 0;
            }

            *(dialogues_count) = *(dialogues_count) + 1;
        }
    }

    return;
}