#include <stdio.h>
#include <stdlib.h>
#include "toml-c.h"

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
