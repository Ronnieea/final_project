#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "parseToml.h"

#define MAX_SCENES 50
#define MAX_CHARACTERS 10
#define MAX_EVENTS 50
#define MAX_DIALOGUE 50
#define MAX_ITEMS 10
#define MAX_OPTIONS 10

// Function to find dialogue by key
Dialogue* find_dialogue_by_key(Dialogue dialogues[], uint16_t dialogues_count, const char* key) {
    for (uint16_t i = 0; i < dialogues_count; i++) {
        if (strcmp(dialogues[i].key, key) == 0) {
            return &dialogues[i];
        }
    }
    return NULL;
}

// Function to find event by key
Event* find_event_by_key(Event events[], uint16_t events_count, const char* key) {
    for (uint16_t i = 0; i < events_count; i++) {
        if (strcmp(events[i].key, key) == 0) {
            return &events[i];
        }
    }
    return NULL;
}

// Function to find scene by key
Scene* find_scene_by_key(Scene scenes[], uint16_t scenes_count, const char* key) {
    for (uint16_t i = 0; i < scenes_count; i++) {
        if (strcmp(scenes[i].key, key) == 0) {
            return &scenes[i];
        }
    }
    return NULL;
}

// Function to find character by key
Character* find_character_by_key(Character characters[], uint16_t characters_count, const char* key) {
    for (uint16_t i = 0; i < characters_count; i++) {
        if (strcmp(characters[i].key, key) == 0) {
            return &characters[i];
        }
    }
    return NULL;
}

void print_all_data(Scene scenes[], uint16_t scenes_count, Character characters[], uint16_t characters_count, Event events[], uint16_t events_count, Dialogue dialogues[], uint16_t dialogues_count, Item items[], uint16_t items_count) {
    // Print scenes
    for (uint16_t i = 0; i < scenes_count; i++) {
        printf("Scene[%d]: Key: %s, Name: %s, Background: %s\n", i, scenes[i].key, scenes[i].name, scenes[i].background);
    }

    // Print characters
    for (uint16_t i = 0; i < characters_count; i++) {
        printf("Character[%d]: Key: %s, Name: %s, Tachie: %s\n", i, characters[i].key, characters[i].name, characters[i].tachie);
    }

    // Print events
    for (uint16_t i = 0; i < events_count; i++) {
        printf("Event[%d]: Key: %s, Scene: %s, Dialogue: %s\n", i, events[i].key, events[i].scene, events[i].dialogue);
    }

    // Print dialogues
    for (uint16_t i = 0; i < dialogues_count; i++) {
        printf("Dialogue[%d]: Key: %s, Character: %s, Item: %s, Text: %s\n", i, dialogues[i].key, dialogues[i].character, dialogues[i].item, dialogues[i].text);
        for (uint8_t j = 0; j < dialogues[i].options_count; j++) {
            printf("  Option[%d]: Text: %s, Next: %s, Event: %s\n", j, dialogues[i].options[j].text, dialogues[i].options[j].next, dialogues[i].options[j].event);
        }
    }

    // Print items
    for (uint16_t i = 0; i < items_count; i++) {
        printf("Item[%d]: Key: %s, Name: %s, Icon: %s\n", i, items[i].key, items[i].name, items[i].icon);
    }
}

Dialogue* process_dialogue(Dialogue* current_dialogue, Dialogue dialogues[], uint16_t dialogues_count, Event events[], uint16_t events_count, Event** next_event, Scene scenes[], uint16_t scenes_count, Character characters[], uint16_t characters_count) {
    Character* character = find_character_by_key(characters, characters_count, current_dialogue->character);
    if (character != NULL) {
        printf("Tachie: %s\n", character->tachie);
    }
    printf("Dialogue: %s\n", current_dialogue->text);

    if (current_dialogue->options_count > 1) {
        printf("Options:\n");
        for (uint8_t i = 0; i < current_dialogue->options_count; i++) {
            printf("%c. %s\n", 'A' + i, current_dialogue->options[i].text);
        }
        printf("Choose an option (A, B, C...): ");
        char choice;
        scanf(" %c", &choice);
        int option_index = choice - 'A';
        if (option_index >= 0 && option_index < current_dialogue->options_count) {
            if (current_dialogue->options[option_index].next && current_dialogue->options[option_index].next[0] != '\0') {
                Dialogue* next_dialogue = find_dialogue_by_key(dialogues, dialogues_count, current_dialogue->options[option_index].next);
                if (next_dialogue != NULL) {
                    return next_dialogue;
                } else {
                    *next_event = find_event_by_key(events, events_count, current_dialogue->options[option_index].next);
                    return NULL;
                }
            } else if (current_dialogue->options[option_index].event && current_dialogue->options[option_index].event[0] != '\0') {
                *next_event = find_event_by_key(events, events_count, current_dialogue->options[option_index].event);
                return NULL;
            }
        } else {
            printf("Invalid choice. Exiting.\n");
            return NULL;
        }
    } else if (current_dialogue->options_count == 1) {
        if (current_dialogue->options[0].next && current_dialogue->options[0].next[0] != '\0') {
            Dialogue* next_dialogue = find_dialogue_by_key(dialogues, dialogues_count, current_dialogue->options[0].next);
            if (next_dialogue != NULL) {
                return next_dialogue;
            } else {
                *next_event = find_event_by_key(events, events_count, current_dialogue->options[0].next);
                return NULL;
            }
        } else if (current_dialogue->options[0].event && current_dialogue->options[0].event[0] != '\0') {
            *next_event = find_event_by_key(events, events_count, current_dialogue->options[0].event);
            return NULL;
        }
    } else {
        return NULL;
    }

    return NULL;
}

Dialogue* process_event(Event* current_event, Dialogue dialogues[], uint16_t dialogues_count, Scene scenes[], uint16_t scenes_count) {
    printf("Event: %s\n", current_event->key);
    Scene* scene = find_scene_by_key(scenes, scenes_count, current_event->scene);
    if (scene != NULL) {
        printf("Background: %s\n", scene->background);
    }
    return find_dialogue_by_key(dialogues, dialogues_count, current_event->dialogue);
}

int main(int argc, char* argv[]) {
    const char* filePath;
    if (argc > 1) {
        filePath = argv[1];
    } else {
        filePath = getenv("TOML_FILE_PATH");
        if (filePath == NULL) {
            filePath = "script.toml"; // Default path
        }
    }

    // Initialize arrays and counters
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

    // Initialize structures
    for (int i = 0; i < MAX_SCENES; i++) {
        scenes[i] = initial_scenes(scenes[i]);
    }
    for (int i = 0; i < MAX_CHARACTERS; i++) {
        characters[i] = initial_character(characters[i]);
    }
    for (int i = 0; i < MAX_EVENTS; i++) {
        events[i] = initial_event(events[i]);
    }
    for (int i = 0; i < MAX_DIALOGUE; i++) {
        dialogues[i] = initial_dialogue(dialogues[i]);
    }
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i] = initial_item(items[i]);
    }

    toml_table_t* config = parseToml(filePath);
    if (config != NULL) {
        load_data(config, scenes, characters, events, dialogues, items, options, &scenes_count, &characters_count, &events_count, &dialogues_count, &items_count, &options_count);

        // Print all data with labels
        print_all_data(scenes, scenes_count, characters, characters_count, events, events_count, dialogues, dialogues_count, items, items_count);

        // Start from the event with key "start"
        Event* current_event = find_event_by_key(events, events_count, "start");
        Dialogue* current_dialogue = NULL;
        Event* next_event = NULL;

        if (current_event != NULL) {
            current_dialogue = process_event(current_event, dialogues, dialogues_count, scenes, scenes_count);
        }

        while (current_dialogue != NULL || next_event != NULL) {
            if (current_dialogue != NULL) {
                current_dialogue = process_dialogue(current_dialogue, dialogues, dialogues_count, events, events_count, &next_event, scenes, scenes_count, characters, characters_count);
                if (next_event != NULL) {
                    current_dialogue = NULL;
                }
            } else if (next_event != NULL) {
                current_dialogue = process_event(next_event, dialogues, dialogues_count, scenes, scenes_count);
                next_event = NULL;
            }
        }

        // Free allocated memory
        for (uint16_t i = 0; i < scenes_count; i++) {
            free_scene(scenes[i]);
        }
        for (uint16_t i = 0; i < characters_count; i++) {
            free_character(characters[i]);
        }
        for (uint16_t i = 0; i < events_count; i++) {
            free_event(events[i]);
        }
        for (uint16_t i = 0; i < dialogues_count; i++) {
            free_dialogue(dialogues[i]);
        }
        for (uint16_t i = 0; i < items_count; i++) {
            free_item(items[i]);
        }

        // Free the parsed TOML structure
        toml_free(config);
    } else {
        printf("Failed to parse the TOML file.\n");
    }
    return 0;
}
