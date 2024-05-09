#include <stdio.h>
#include <stdlib.h>
#include "toml.h"

typedef struct _Scene
{
    char *name;
    char *background;
} Scene;

typedef struct _Option
{
    char *text;
    char *next;  // 表示跳到下一個dialogue
    char *event; // 表示結束目前的場景，跳到下一個場景
} Option;

typedef struct _Dialogue
{
    char *character;
    char *text;
    Option *options;
    uint8_t option_count;
} Dialogue;

typedef struct _Event
{
    Scene *scene;
    Dialogue *dialogue;
} Event;

typedef struct _Character
{
    char *name;
    char *avatar;
    char *tachie;
    char *location; // 地點，不一定
} Character;

typedef struct _Item
{
    char *name;
    char *description; // 不一定
    char *icon;
} Item;

// 從toml表中讀取字符串
char *get_string_form_toml(toml_table_t *table, char *key);
// 讀取角色資料
Character *load_character(toml_table_t *table);
// 讀取場景資料
Scene *load_scene(toml_table_t *table);
// 讀取選擇
Option *load_option(toml_table_t *table);
// 讀取對話
Dialogue *load_dialogue(toml_table_t *table);