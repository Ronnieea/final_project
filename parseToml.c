#include "toml.h"
#include "parseToml.h"

char *get_string_form_toml(toml_table_t *table, char *key)
{
    toml_datum_t datum = toml_string_in(table, key);
    if (datum.ok)
        return datum.u.s;
    else
        return NULL;
}

Character *load_character(toml_table_t *table)
{
    if (!table)
        return NULL;

    Character *character = malloc(sizeof(Character));
    character->name = get_string_form_toml(table, "name");
    character->avatar = get_string_form_toml(table, "avatar");
    character->tachie = get_string_form_toml(table, "tachie");
    character->location = get_string_form_toml(table, "location"); // 可能為NULL

    return character;
}

Scene *load_scene(toml_table_t *table)
{
    if (!table)
        return NULL;

    Scene *scene = malloc(sizeof(Scene));
    scene->name = get_string_form_toml(table, "name");
    scene->background = get_string_form_toml(table, "background");

    return scene;
}

Option *load_option(toml_table_t *table)
{
    if (!table)
        return NULL;

    Option *option = malloc(sizeof(Option));
    option->text = get_string_form_toml(table, "text");
    option->next = get_string_form_toml(table, "next");
    option->event = get_string_form_toml(table, "event");

    return option;
}

Dialogue *load_dialogue(toml_table_t *table)
{
    if (!table)
        return NULL;

    Dialogue *dialogue = malloc(sizeof(Dialogue));
    dialogue->character = get_string_form_toml(table, "character");
    dialogue->text = get_string_form_toml(table, "text");
}