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

        const char *data = "{\"model\": \"text-davinci-003\", \"prompt\": \"请生成一个互動小說的劇本，格式如下示例。劇本應包含至少三個角色、三個場景、兩個物品和三個不同的结局。請確保劇本的內容流暢，並且所有角色、場景和物品都包含在劇本中。\\n\\n"
                           "[scene.market]\\n"
                           "background = \\\"/example_game/assets/market.bmp\\\"\\n"
                           "name = \\\"熱鬧市場\\\"\\n\\n"
                           "[scene.castle]\\n"
                           "background = \\\"/example_game/assets/castle.bmp\\\"\\n"
                           "name = \\\"古老城堡\\\"\\n\\n"
                           "[scene.forest]\\n"
                           "background = \\\"/example_game/assets/forest.bmp\\\"\\n"
                           "name = \\\"幽深森林\\\"\\n\\n"
                           "[character.knight]\\n"
                           "affinity = 50\\n"
                           "avatar = \\\"/example_game/assets/knight_avatar.bmp\\\"\\n"
                           "name = \\\"騎士阿勇\\\"\\n"
                           "tachie = \\\"/example_game/assets/knight_tachie.bmp\\\"\\n\\n"
                           "[character.merchant]\\n"
                           "affinity = 50\\n"
                           "avatar = \\\"/example_game/assets/merchant_avatar.bmp\\\"\\n"
                           "name = \\\"商人\\\"\\n"
                           "tachie = \\\"/example_game/assets/merchant_tachie.bmp\\\"\\n\\n"
                           "[item.sword]\\n"
                           "icon = \\\"/example_game/assets/sword_icon.bmp\\\"\\n"
                           "name = \\\"魔法劍\\\"\\n\\n"
                           "[item.potion]\\n"
                           "icon = \\\"/example_game/assets/potion_icon.bmp\\\"\\n"
                           "name = \\\"治療藥水\\\"\\n\\n"
                           "[player]\\n"
                           "inventory = [\\\"sword\\\", \\\"potion\\\"]\\n"
                           "role = \\\"knight\\\"\\n\\n"
                           "[event.start]\\n"
                           "dialogue = \\\"intro\\\"\\n"
                           "scene = \\\"market\\\"\\n\\n"
                           "[dialogue.intro]\\n"
                           "character = \\\"knight\\\"\\n"
                           "text = \\\"今天是個充滿希望的日子，我需要準備好迎接挑戰。\\\"\\n"
                           "[[dialogue.intro.options]]\\n"
                           "effect = \\\"+10\\\"\\n"
                           "next = \\\"meet_merchant\\\"\\n"
                           "text = \\\"前往市場準備裝備。\\\"\\n\\n"
                           "[dialogue.meet_merchant]\\n"
                           "character = \\\"merchant\\\"\\n"
                           "item = \\\"sword\\\"\\n"
                           "text = \\\"勇敢的騎士，這是你需要的魔法劍。\\\"\\n"
                           "[[dialogue.meet_merchant.options]]\\n"
                           "effect = \\\"+5\\\"\\n"
                           "event = \\\"castle_entry\\\"\\n"
                           "text = \\\"謝謝你，商人。\\\"\\n\\n"
                           "請根據上述格式生成一个新的劇本。\", \"max_tokens\": 1500}";

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
