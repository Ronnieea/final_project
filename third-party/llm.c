#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include </home/Ronnie/final_project/src/toml.h>
#include <sndfile.h>
#include "llm.h"

#define API_KEY ""
#define API_URL "https://api.openai.com/v1/chat/completions"
#define IMAGE_API_URL "https://api.openai.com/v1/images/generations"
#define AUDIO_API_URL "https://api.openai.com/v1/audio/speech"

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
        snprintf(data, sizeof(data), "{\"model\":\"dall-e-3\",\"prompt\": \"%s\", \"n\": 1, \"size\": \"1024x1024\", \"quality\":\"hd\"}", prompt);

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
            char *description = malloc(512 * sizeof(char));
            toml_table_t *subtable = toml_table_in(table, key);
            if (toml_rtos(toml_raw_in(subtable, "description"), &description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_background.bmp", key);
                generate_image(description, fileName);
            }
            free(description);
        }
    }
    // 獲取頭像和立繪的圖片說明並產生圖片
    table = toml_table_in(config, "character");
    if (table)
    {
        for (int i = 0; table == NULL || (key = toml_key_in(table, i)) != NULL; i++)
        {
            char *description = malloc(512 * sizeof(char));
            toml_table_t *subtable = toml_table_in(table, key);
            if (toml_rtos(toml_raw_in(subtable, "avatar_description"), &description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_avatar.bmp", key);
                generate_image(description, fileName);
            }
            if (toml_rtos(toml_raw_in(subtable, "tachie_description"), &description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_tachie.bmp", key);
                generate_image(description, fileName);
            }
            free(description);
        }
    }
    // 獲取物品圖片說明並產生圖片
    table = toml_table_in(config, "item");
    if (table)
    {
        for (int i = 0; table == NULL || (key = toml_key_in(table, i)) != NULL; i++)
        {
            char *description = malloc(512 * sizeof(char));
            toml_table_t *subtable = toml_table_in(table, key);
            if (toml_rtos(toml_raw_in(subtable, "description"), &description) == 0)
            {
                char fileName[256] = {0};
                snprintf(fileName, sizeof(fileName), "%s_icon.bmp", key);
                generate_image(description, fileName);
            }
            free(description);
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

        const char *data = "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"Below are some examples and explanations of interactive novels:\\n\\n"
                           "1. scene\\n"
                           "Description: Background image for the scene\\n"
                           "content: Background name(name)、background image address(background)、describe the image(description):Very detailed description\\n"
                           "example: [scene.street]\\n"
                           "background = \\\"/third-party/assets/street.bmp\\\"\\n"
                           "name = \\\"dark alley\\\"\\n"
                           "description = \\\"\\\"\\n\\n"
                           "2. character\\n"
                           "Description: Images of people talking in the scene\\n"
                           "content: character name(name)、Character head image address(avatar)、character tachie image address(tachie)、describe the avatar image(avatar_description):Provides more details, such as the character's clothing color, facial expression, pose, etc.、describe the tachie image(tachie_description):Provides more details, such as the character's clothing color, facial expression, pose, etc.\\n"
                           "example: [character.man]\\n"
                           "avatar = \\\"/third-party/assets/unknownman_avatar.bmp\\\"\\n"
                           "name = \\\"weido\\\"\\n"
                           "tachie = \\\"/third-party/assets/unknownman_tachie.bmp\\\"\\n"
                           "avatar_description = \\\"\\\"\\n\\n"
                           "tachie_description = \\\"\\\"\\n\\n"
                           "3. item\\n"
                           "Description: Images of items required in the scene\\n"
                           "content: item's name(name)、item icon address(icon)、describe the item icon(description):Very detailed description\\n"
                           "example: [item.gun]\\n"
                           "name = \\\"gun\\\"\\n"
                           "icon = \\\"/third-party/assets/gun_icon.bmp\\\"\\n"
                           "description = \\\"\\\"\\n\\n"
                           "4. event\\n"
                           "Explanation: Each screen shown is a scene; the plot of the entire script must begin with [event.start]; If there is no dialogue, it will end up being displayed.\\n"
                           "content: Scene background image(scene)、A conversation taking place in the scene(dialogue)\\n"
                           "example: [event.start]\\n"
                           "scene = \\\"street\\\" #The key for the corresponding scene\\n"
                           "dialogue = \\\"ask_1\\\" #The corresponding dialogue key\\n\\n"
                           "5. dialogue\\n"
                           "Description: Each character is speaking, and only one person on the screen can speak, and there may be an optional array in the dialogue.\\n"
                           "content: Characters in the story(character)、Items that appear during conversation(item)、Character story content(text)、Offering players' choices(option)At least there is one option, which can represent the player's conversation, or just click next or event to jump to the next dialogue or event. If there is no option it will be displayed after all.\\n"
                           "example: (1)There are 2 or more options\\n"
                           "[dialogue.ask_1]\\n"
                           "character = \\\"man\\\"\\n"
                           "text = \\\"Two old tigers...Two old tigers...\\\"\\n"
                           "[[dialogue.ask_1.options]]\\n"
                           "next = \\\"ask_3\\\"\\n"
                           "text = \\\"Run fast...Run fast...\\\"\\n"
                           "[[dialogue.ask_1.options]]\\n"
                           "next = \\\"ask_2\\\"\\n"
                           "text = \\\"Uncomfortable running...Uncomfortable running...\\\"\\n"
                           "(2)Only 1 option: The character on the screen talks to the player and jumps to the next event or dialogue\\n"
                           "[dialogue.ask_3]\\n"
                           "character = \\\"man\\\"\\n"
                           "text = \\\"you will call me\\\"\\n"
                           "[[dialogue.ask_3.options]]\\n"
                           "next = \\\"inhospital_marina\\\"\\n"
                           "(3)Only 1 option: Use to show the character's story on the screen\\n"
                           "[dialogue.inhospital_player]\\n"
                           "character = \\\"marina\\\"\\n"
                           "[[dialogue.inhospital_player.options]]\\n"
                           "event = \\\"inhospital_marina\\\"\\n"
                           "text = \\\"How could it suddenly happen like this?\\\"\\n\\n"
                           "6. option\\n"
                           "description: There exists an array under dialogue.\\n"
                           "content: Content of the option(text)、next dailogue(next)、next event(event)Only next and event have one, if there is neither next nor event in the end.\\n"
                           "範例: [dialogue.inhospital_marina]\\n"
                           "character = \\\"marina\\\"\\n"
                           "text = \\\"The doctor said that after someone visited his mother last night, he was like this...\\\"\\n"
                           "[[dialogue.inhospital_marina.options]]\\n"
                           "event = \\\"bar\\\" #Players can select this story, wrap up the current scene, and jump to the next event\\n"
                           "text = \\\"(Call the black-clad man and receive an assassination mission)\\\"\\n"
                           "[[dialogue.inhospital_marina.options]]\\n"
                           "next = \\\"home\\\" #Players can choose this topic, but the current situation will still exist, so the next dialogue will start.\\n"
                           "text = \\\"(Black-clad man who doesn't answer the phone)\\\"\\n\\n"
                           "7. player\\n"
                           "Description: Displays the player's character.(character的其中一個)\\n"
                           "content: player's name(role)、player's inventory(inventory):All items in it\\n"
                           "example: [player]\\n"
                           "inventory = [\\\"sword\\\", \\\"potion\\\", \\\"scroll\\\"]\\n"
                           "role = \\\"knight\\\"\\n\\n"
                           "Please create a brand new interactive novel, with completely different plots and scenes. A script contains at least three characters, three scenes, two objects, and three different endings. [scene], [character], [item], [event], [dialogue], [player], [option] all should be exist. Please make sure the script is smooth and that all characters, scenes and objects are included in the script.\"}]}";

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
            // 解析JSON
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json == NULL)
            {
                fprintf(stderr, "Error parsing JSON\n");
            }
            else
            {
                // 獲取content字段
                cJSON *choices = cJSON_GetObjectItem(json, "choices");
                if (choices != NULL)
                {
                    cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
                    if (first_choice != NULL)
                    {
                        cJSON *message = cJSON_GetObjectItem(first_choice, "message");
                        if (message != NULL)
                        {
                            cJSON *content = cJSON_GetObjectItem(message, "content");
                            if (content != NULL)
                            {
                                const char *content_str = cJSON_GetStringValue(content);
                                if (content_str != NULL)
                                {
                                    printf("Content: %s\n", content_str);

                                    // 将内容写入文件，并将\\n替换为换行符
                                    if ((pFile = fopen("game_script.toml", "w")) != NULL)
                                    {
                                        for (const char *p = content_str; *p; p++)
                                        {
                                            if (*p == '\\' && *(p + 1) == 'n')
                                            {
                                                fputc('\n', pFile);
                                                p++;
                                            }
                                            else
                                            {
                                                fputc(*p, pFile);
                                            }
                                        }
                                        fclose(pFile);
                                    }
                                    else
                                    {
                                        fprintf(stderr, "Failed to open file for writing.\n");
                                    }
                                }
                            }
                        }
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
