#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define API_KEY "aaf128d4246b075ade58ba2054d61733"
#define URL_FORMAT "http://api.openweathermap.org/data/2.5/forecast?q=%s&appid=%s&units=metric"

// Structure to handle the HTTP response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function to handle data received by libcurl
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Not enough memory\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Function to fetch the weather data from OpenWeatherMap
char *get_weather_data(const char *city) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    char url[256];
    snprintf(url, sizeof(url), URL_FORMAT, city, API_KEY);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return NULL;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return chunk.memory;
}

// Function to parse the JSON response and print the weather details
void parse_and_print_weather(char *json_data) {
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) {
        printf("Error parsing JSON\n");
        return;
    }

    cJSON *list = cJSON_GetObjectItem(json, "list");
    if (!list) {
        printf("Weather data not found\n");
        cJSON_Delete(json);
        return;
    }

    for (int i = 0; i < 5; ++i) { // Printing forecast for the first 5 entries (next few days)
        cJSON *entry = cJSON_GetArrayItem(list, i);
        cJSON *main = cJSON_GetObjectItem(entry, "main");
        cJSON *weather = cJSON_GetArrayItem(cJSON_GetObjectItem(entry, "weather"), 0);
        cJSON *temp = cJSON_GetObjectItem(main, "temp");
        cJSON *description = cJSON_GetObjectItem(weather, "description");
        cJSON *dt_txt = cJSON_GetObjectItem(entry, "dt_txt");

        printf("Date: %s\n", dt_txt->valuestring);
        printf("Temperature: %.2f°C\n", temp->valuedouble);
        printf("Weather: %s\n\n", description->valuestring);
    }

    cJSON_Delete(json);
}

int main() {
    char city[50];

    printf("Enter city name: ");
    scanf("%49s", city);

    char *weather_data = get_weather_data(city);

    if (weather_data) {
        parse_and_print_weather(weather_data);
        free(weather_data);
    } else {
        printf("Failed to fetch weather data.\n");
    }

    return 0;
}