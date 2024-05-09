#include <SDL2/SDL.h>
#include "parseToml.h"

int main()
{
    FILE *pFile = NULL;
    if ((pFile = fopen("script.toml", "r")) == NULL)
    {
        printf("File could not be opened!\n");
        return 1;
    }

    char errbuf[200] = {0};
    toml_table_t *table = toml_parse_file(pFile, errbuf, sizeof(errbuf));
    fclose(pFile);
    if (!table)
    {
        printf("Parsing error: %s\n", errbuf);
        return 1;
    }
}