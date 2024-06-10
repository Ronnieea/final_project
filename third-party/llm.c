#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include </home/Ronnie/final_project/src/toml.h>

#define API_KEY "your_openai_api_key_here"
#define API_URL "https://api.openai.com/v1/completions"
#define IMAGE_API_URL "https://api.openai.com/v1/images/generations"

struct MemoryStruct
{
    char *memory;
    size_t size;
};

// libcurl寫入回調函數
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
    {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Initialize LLM
bool init_llm()
{
    // Initialize libcurl
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
    {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return false;
    }
    return true;
}

// 使用openAI API生成圖片
void generate_image(const char *prompt, const char *filename)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer " API_KEY);

        char data[512];
        snprintf(data, sizeof(data), "{\"model\": \"image-alpha-001\", \"prompt\": \"%s\", \"n\": 1, \"size\": \"1024x1024\"}", prompt);

        curl_easy_setopt(curl, CURLOPT_URL, IMAGE_API_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            // 解析並保存圖片URL
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json == NULL)
            {
                printf("Failed to parse JSON response.\n");
            }
            else
            {
                cJSON *data = cJSON_GetObjectItem(json, "data");
                if (cJSON_IsArray(data))
                {
                    cJSON *first_item = cJSON_GetArrayItem(data, 0);
                    cJSON *url = cJSON_GetObjectItem(first_item, "url");
                    if (cJSON_IsString(url))
                    {
                        // 下載並保存圖片
                        CURL *image_curl;
                        FILE *fp = fopen(filename, "wb");
                        image_curl = curl_easy_init();
                        curl_easy_setopt(image_curl, CURLOPT_URL, url->valuestring);
                        curl_easy_setopt(image_curl, CURLOPT_WRITEFUNCTION, NULL);
                        curl_easy_setopt(image_curl, CURLOPT_WRITEDATA, fp);
                        curl_easy_perform(image_curl);
                        curl_easy_cleanup(image_curl);
                        fclose(fp);
                    }
                }
                cJSON_Delete(json);
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(chunk.memory);
    }
    curl_global_cleanup();
}

// 解析劇本並生成圖片
void parse_script_and_generate_images(const char *script_path)
{
    FILE *pFile = NULL;
    if ((pFile = fopen(script_path, "r")) == NULL)
    {
        fprintf(stderr, "Failed to open script file: %s\n", script_path);
        return;
    }

    char errbuf[200];
    toml_table_t *config = toml_parse_file(pFile, errbuf, sizeof(errbuf));
    fclose(pFile);

    if (!config)
    {
        fprintf(stderr, "Failed to parse TOML file: %s\n", errbuf);
        return;
    }

    toml_table_t *table = NULL;
    const char *key = NULL;
    // 獲取背景圖片說明並產生圖片
    table = toml_table_in(config, "scene");
    if (table)
    {
        for (int i = 0; table == NULL || (key = toml_key_in(table, i)) != NULL; i++)
        {
            char description[512] = {0};
            toml_table_t *subtable = toml_table_in(table, key);
            if (toml_rtos(toml_raw_in(subtable, "background"), description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_background.bmp", key);
                generate_image(description, fileName);
            }
        }
    }
    // 獲取頭像和立繪的圖片說明並產生圖片
    table = toml_table_in(config, "character");
    if (table)
    {
        for (int i = 0; table == NULL || (key = toml_key_in(table, i)) != NULL; i++)
        {
            char description[512] = {0};
            toml_table_t *subtable = toml_table_in(table, key);
            if (toml_rtos(toml_raw_in(subtable, "avatar"), description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_avatar.bmp", key);
                generate_image(description, fileName);
            }
            if (toml_rtos(toml_raw_in(subtable, "tachie"), description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_tachie.bmp", key);
                generate_image(description, fileName);
            }
        }
    }
    // 獲取物品圖片說明並產生圖片
    table = toml_table_in(config, "item");
    if (table)
    {
        for (int i = 0; table == NULL || (key = toml_key_in(table, i)) != NULL; i++)
        {
            char description[512] = {0};
            toml_table_t *subtable = toml_table_in(table, key);
            if (toml_rtos(toml_raw_in(subtable, "icon"), description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_icon.bmp", key);
                generate_image(description, fileName);
            }
        }
    }

    toml_free(config);
}

// llm產生劇本
void generate_game_script()
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer " API_KEY);

        const char *data = "{\"model\": \"text-davinci-003\", \"prompt\": \"Generate a game script in TOML format with the following conditions:\\n1. At least three characters, each with a unique name and images (avatar and tachie).\\n2. At least three scenes, each with a unique name and background image.\\n3. At least two items, each with a unique name and icon image.\\n4. At least three endings.\\n5. A complete and coherent storyline.\\n6. All characters, scenes, and items are generated by the LLM.\\n7. The script should be in the following format:\\n\\n[scene.forest]\\nbackground = \\\"path/to/forest.jpg\\\"\\nname = \\\"Mysterious Forest\\\"\\n\\n[scene.village]\\nbackground = \\\"path/to/village.jpg\\\"\\nname = \\\"Quiet Village\\\"\\n\\n[scene.cave]\\nbackground = \\\"path/to/cave.jpg\\\"\\nname = \\\"Dark Cave\\\"\\n\\n[character.hero]\\navatar = \\\"path/to/hero_avatar.jpg\\\"\\nname = \\\"Hero\\\"\\ntachie = \\\"path/to/hero_tachie.jpg\\\"\\n\\n[character.villager]\\navatar = \\\"path/to/villager_avatar.jpg\\\"\\nname = \\\"Villager\\\"\\ntachie = \\\"path/to/villager_tachie.jpg\\\"\\n\\n[character.monster]\\navatar = \\\"path/to/monster_avatar.jpg\\\"\\nname = \\\"Monster\\\"\\ntachie = \\\"path/to/monster_tachie.jpg\\\"\\n\\n[item.sword]\\nname = \\\"Sword\\\"\\nicon = \\\"path/to/sword_icon.jpg\\\"\\n\\n[item.shield]\\nname = \\\"Shield\\\"\\nicon = \\\"path/to/shield_icon.jpg\\\"\\n\\n[event.start]\\nscene = \\\"forest\\\"\\ndialogue = \\\"intro\\\"\\n\\n[dialogue.intro]\\ncharacter = \\\"hero\\\"\\ntext = \\\"Welcome to the forest. What do you want to do next?\\\"\\n[[dialogue.intro.options]]\\nnext = \\\"explore\\\"\\ntext = \\\"Explore the forest\\\"\\n[[dialogue.intro.options]]\\nnext = \\\"village\\\"\\ntext = \\\"Go to the village\\\"\\n\\n[dialogue.explore]\\ncharacter = \\\"hero\\\"\\ntext = \\\"You explore the forest and find a cave.\\\"\\nevent = \\\"cave\\\"\\n\\n[dialogue.village]\\ncharacter = \\\"villager\\\"\\ntext = \\\"Welcome to our village.\\\"\\nevent = \\\"village\\\"\\n\\n[event.cave]\\nscene = \\\"cave\\\"\\ndialogue = \\\"cave_intro\\\"\\n\\n[dialogue.cave_intro]\\ncharacter = \\\"monster\\\"\\ntext = \\\"You have entered my lair!\\\"\\n[[dialogue.cave_intro.options]]\\nnext = \\\"fight\\\"\\ntext = \\\"Fight the monster\\\"\\n[[dialogue.cave_intro.options]]\\nnext = \\\"run\\\"\\ntext = \\\"Run away\\\"\\n\\n[dialogue.fight]\\ncharacter = \\\"hero\\\"\\ntext = \\\"You fought bravely and defeated the monster.\\\"\\nevent = \\\"end_good\\\"\\n\\n[dialogue.run]\\ncharacter = \\\"hero\\\"\\ntext = \\\"You ran away safely.\\\"\\nevent = \\\"end_neutral\\\"\\n\\n[event.end_good]\\nscene = \\\"forest\\\"\\ndialogue = \\\"good_ending\\\"\\n\\n[dialogue.good_ending]\\ncharacter = \\\"hero\\\"\\ntext = \\\"You have saved the day!\\\"\\n\\n[event.end_neutral]\\nscene = \\\"forest\\\"\\ndialogue = \\\"neutral_ending\\\"\\n\\n[dialogue.neutral_ending]\\ncharacter = \\\"hero\\\"\\ntext = \\\"You have survived, but the monster is still out there.\\\"\", \"max_tokens\": 1500}";

        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        FILE *pFile = NULL;
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            // 將得到的劇本寫入劇本檔裡
            printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
            if ((pFile = fopen("game_script.toml", "w")) == NULL)
            {
                fputs(chunk.memory, pFile);
                fclose(pFile);
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(chunk.memory);
        if (pFile)
        {
            fclose(pFile);
        }
    }
    curl_global_cleanup();
}

// 獲取玩家與角色對話的response
void get_llm_response(const char *character, const char *input_text, char *response, size_t response_size)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer " API_KEY);

        char data[512];
        snprintf(data, sizeof(data), "{\"model\": \"text-davinci-003\", \"prompt\": \"%s responds to: %s\", \"max_tokens\": 150}", character, input_text);

        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            // 處理與角色互動得到的response
            strncpy(response, chunk.memory, response_size - 1);
            response[response_size - 1] = '\0';
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(chunk.memory);
    }
    curl_global_cleanup();
}
