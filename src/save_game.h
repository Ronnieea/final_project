#ifndef SAVE_GAME_H_INCLUDED
#define SAVE_GAME_H_INCLUDED

void save_game(char **filename, Event *current_event, Dialogue *current_dialogue, int show_overlay, char current_background_path[512], int current_sound);
void load_game(const char *filename, Event **current_event, Dialogue **current_dialogue, int *show_overlay, int *current_sound, char current_background_path[512], Event events[], uint16_t events_count, Dialogue dialogues[], uint16_t dialogues_count);

#endif