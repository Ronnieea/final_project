#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "parseToml.h"

void save_game(char **filename, Event *current_event, Dialogue *current_dialogue, int show_overlay, char current_background_path[512], int current_sound)
{
    cJSON *current_event_root = cJSON_CreateObject();
    if (current_event != NULL)
    {
        cJSON_AddStringToObject(current_event_root, "key", current_event->key);
        cJSON_AddStringToObject(current_event_root, "dialogue", current_event->dialogue);
        cJSON_AddStringToObject(current_event_root, "scene", current_event->scene);
    }

    cJSON *current_dialogue_root = cJSON_CreateObject();
    if (current_dialogue != NULL)
    {
        cJSON_AddStringToObject(current_dialogue_root, "key", current_dialogue->key);
        cJSON_AddStringToObject(current_dialogue_root, "character", current_dialogue->character);
        cJSON_AddStringToObject(current_dialogue_root, "item", current_dialogue->item);
        cJSON_AddStringToObject(current_dialogue_root, "text", current_dialogue->text);
        cJSON_AddNumberToObject(current_dialogue_root, "options_count", current_dialogue->options_count);

        // create json array
        cJSON *json_options = cJSON_CreateArray();
        for (int i = 0; i < current_dialogue->options_count; i++)
        {
            cJSON *current_dialogue_options_root = cJSON_CreateObject();
            cJSON_AddStringToObject(current_dialogue_options_root, "event", current_dialogue->options[i].event);
            cJSON_AddStringToObject(current_dialogue_options_root, "next", current_dialogue->options[i].next);
            cJSON_AddStringToObject(current_dialogue_options_root, "text", current_dialogue->options[i].text);
            cJSON_AddNumberToObject(current_dialogue_options_root, "effect", current_dialogue->options[i].effect);
            cJSON_AddItemToArray(json_options, current_dialogue_options_root);
        }

        cJSON_AddItemToObject(current_dialogue_root, "options", json_options);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "current_event", current_event_root);
    cJSON_AddItemToObject(root, "current_dialogue", current_dialogue_root);
    cJSON_AddNumberToObject(root, "show_overlay", show_overlay);
    cJSON_AddStringToObject(root, "current_background_path", current_background_path);
    cJSON_AddNumberToObject(root, "current_sound", current_sound);

    *filename = "save_game.json";
    FILE *pFile = NULL;
    if ((pFile = fopen(*filename, "w")) != NULL)
    {
        char *json_str = cJSON_Print(root);
        fprintf(pFile, "%s\n", json_str);
        free(json_str);
        fclose(pFile);
    }
    else
    {
        fprintf(stderr, "File %s could not be opened!\n", *filename);
        return;
    }
    cJSON_Delete(root);
}

void load_game(const char *filename, Event **current_event, Dialogue **current_dialogue, int *show_overlay, int *current_sound, char current_background_path[512], Event events[], uint16_t events_count, Dialogue dialogues[], uint16_t dialogues_count)
{
    FILE *pFile = NULL;
    if ((pFile = fopen(filename, "r")) != NULL)
    {
        char buffer[1024] = {0};
        fread(buffer, 1, 1024, pFile);
        fclose(pFile);

        cJSON *root = cJSON_Parse(buffer);
        if (root != NULL)
        {
            // json to event
            cJSON *current_event_root = cJSON_GetObjectItem(root, "current_event");
            if (current_event_root != NULL)
            {
                if (cJSON_GetObjectItem(current_event_root, "key"))
                {
                    for (uint16_t i = 0; i < events_count; i++)
                    {
                        if (strcmp(events[i].key, cJSON_GetObjectItem(current_event_root, "key")->valuestring) == 0)
                        {
                            *current_event = &events[i];
                            break;
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "Can't find the current_event object.\n");
                return;
            }

            // json to character
            cJSON *current_dialogue_root = cJSON_GetObjectItem(root, "current_dialogue");
            if (current_dialogue_root != NULL)
            {
                if (cJSON_GetObjectItem(current_dialogue_root, "key"))
                {
                    for (uint16_t i = 0; i < dialogues_count; i++)
                    {
                        if (strcmp(dialogues[i].key, cJSON_GetObjectItem(current_dialogue_root, "key")->valuestring) == 0)
                        {
                            *current_dialogue = &dialogues[i];
                            break;
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "Can't find the current_dialogue object.\n");
                return;
            }

            // json to overlay
            *show_overlay = cJSON_GetObjectItem(root, "show_overlay")->valueint;
            *current_sound = cJSON_GetObjectItem(root, "current_sound")->valueint;
            strncpy(current_background_path, cJSON_GetObjectItem(root, "current_background_path")->valuestring, 512);

            cJSON_Delete(root);
        }
        else
        {
            fprintf(stderr, "Unable to parse JSON.\n");
            return;
        }
    }
    else
    {
        fprintf(stderr, "File %s could not be opened!\n", filename);
        return;
    }
}
