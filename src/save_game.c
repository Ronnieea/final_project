#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "parseToml.h"

void save_game(char **filename, Event *current_event, Dialogue *current_dialogue, int show_overlay, char current_background_path[512])
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

void load_game(const char *filename, Event **current_event, Dialogue **current_dialogue, int *show_overlay, char current_background_path[512], Event events[], uint16_t events_count, Dialogue dialogues[], uint16_t dialogues_count)
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
                // strcpy((*current_event)->key, cJSON_GetObjectItem(current_event_root, "key")->valuestring);
                // strcpy((*current_event)->dialogue, cJSON_GetObjectItem(current_event_root, "dialogue")->valuestring);
                // strcpy((*current_event)->scene, cJSON_GetObjectItem(current_event_root, "scene")->valuestring);
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
                // strcpy((*current_dialogue)->key, cJSON_GetObjectItem(current_dialogue_root, "key")->valuestring);
                // strcpy((*current_dialogue)->character, cJSON_GetObjectItem(current_dialogue_root, "character")->valuestring);
                // strcpy((*current_dialogue)->item, cJSON_GetObjectItem(current_dialogue_root, "item")->valuestring);
                // strcpy((*current_dialogue)->text, cJSON_GetObjectItem(current_dialogue_root, "text")->valuestring);
                // (*current_dialogue)->options_count = (uint8_t)cJSON_GetObjectItem(current_dialogue_root, "options_count")->valueint;
                // cJSON *current_dialogue_options_root = cJSON_GetObjectItem(current_dialogue_root, "options");
                // if (current_dialogue_options_root != NULL)
                // {
                //     int options_count = cJSON_GetArraySize(current_dialogue_options_root);
                //     (*current_dialogue)->options_count = options_count;
                //     for (int i = 0; i < options_count; i++)
                //     {
                //         cJSON *json_option = cJSON_GetArrayItem(current_dialogue_options_root, i);
                //         strcpy((*current_dialogue)->options[i].event, cJSON_GetObjectItem(json_option, "event")->valuestring);
                //         strcpy((*current_dialogue)->options[i].next, cJSON_GetObjectItem(json_option, "next")->valuestring);
                //         strcpy((*current_dialogue)->options[i].text, cJSON_GetObjectItem(json_option, "text")->valuestring);
                //         (*current_dialogue)->options[i].effect = (int32_t)cJSON_GetObjectItem(json_option, "effect")->valueint;
                //     }
                // }
                // else
                // {
                //     fprintf(stderr, "Can't find the current_dialogue options object.\n");
                //     return;
                // }
                // cJSON_Delete(current_dialogue_options_root);
            }
            else
            {
                fprintf(stderr, "Can't find the current_dialogue object.\n");
                return;
            }

            // json to overlay
            *show_overlay = cJSON_GetObjectItem(root, "show_overlay")->valueint;
            strncpy(current_background_path, cJSON_GetObjectItem(root, "current_background_path")->valuestring, 512);

            // cJSON_Delete(current_dialogue_root);
            // cJSON_Delete(current_event_root);
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
