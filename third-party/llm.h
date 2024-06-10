#ifndef LLM_H_INCLUDED
#define LLM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
bool init_llm();
void generate_image(const char *prompt, const char *filename);
void parse_script_and_generate_images(const char *script_path);
void generate_game_script();
void get_llm_response(const char *character, const char *input_text, char *response, size_t response_size);

#endif