#include <stdio.h>
#include <stdint.h>
#include "llm.h"

#define SCRIPT_PATH "/home/Ronnie/final_project/third-party/game_script.toml"

int main()
{
    generate_game_script();
    parse_script_and_generate_images(SCRIPT_PATH);
    return 0;
}