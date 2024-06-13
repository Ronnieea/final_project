#ifndef PARSETOML_H_INCLUDED
#define PARSETOML_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include "toml.h"

#define MAX_SCENES 50
#define MAX_CHARACTERS 10
#define MAX_EVENTS 50
#define MAX_DIALOGUE 50
#define MAX_ITEMS 10
#define MAX_OPTIONS 10

typedef struct _Scene
{
    char *key;
    char *name;
    char *background;
} Scene;

typedef struct _Option
{
    char *text;
    char *next;  // 表示跳到下一個dialogue
    char *event; // 表示結束目前的場景，跳到下一個場景
    int32_t effect;
} Option;

typedef struct _Dialogue
{
    char *key;
    char *character;
    char *item;
    char *text;
    Option *options;
    uint8_t options_count;
} Dialogue;

typedef struct _Event
{
    char *key;
    char *scene;
    char *dialogue;
} Event;

typedef struct _Character
{
    char *key;
    char *name;
    char *avatar;
    char *tachie;
} Character;

typedef struct _Item
{
    char *key;
    char *name;
    char *icon;
} Item;

typedef struct _player
{
    char *role;
    char **inventory;
} Player;

void load_data(toml_table_t *config, Player *player, Scene scenes[MAX_SCENES], Character characters[MAX_CHARACTERS], Event events[MAX_EVENTS], Dialogue dialogues[MAX_DIALOGUE], Item items[MAX_ITEMS], Option options[MAX_OPTIONS], uint16_t *inventory_count, uint16_t *scenes_count, uint16_t *characters_count, uint16_t *events_count, uint16_t *dialogues_count, uint16_t *items_count, uint16_t *options_count);
Player initial_player(Player player);
Scene initial_scenes(Scene scene);
Character initial_character(Character character);
Event initial_event(Event event);
Dialogue initial_dialogue(Dialogue dialogue);
Item initial_item(Item item);
void free_player(Player player);
void free_scene(Scene scene);
void free_character(Character character);
void free_event(Event event);
void free_dialogue(Dialogue dialogue);
void free_item(Item item);

#endif