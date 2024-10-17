#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define API_KEY "aaf128d4246b075ade58ba2054d61733"
#define API "http://api.openweathermap.org/data/2.5/forecast?q=%s&appid=%s&units=metric"
#define BUFFER_SIZE 1024

void SaveWeatherInFile(char *CityName, char *FileName) 
{
    CURL *curl;
    CURLcode Result;
    FILE *File;
    File = fopen(FileName, "wb");
    if (File == NULL) {
        perror("Can't open the file.");
        return;
    }

    char url[BUFFER_SIZE];
    snprintf(url, sizeof(url), API, CityName, API_KEY);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl == NULL) {
        perror("Curl object creation failed");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, File);
    Result = curl_easy_perform(curl);

    if (Result != CURLE_OK) {
        fprintf(stderr, "Curl failed: %s\n", curl_easy_strerror(Result));
        curl_easy_cleanup(curl);
        fclose(File);
        return;
    }

    curl_easy_cleanup(curl);
    fclose(File);
    curl_global_cleanup();
}

void ParsWeatherInFile(char *FileName)
{
    FILE *File = fopen(FileName, "r");
    if (File == NULL) {
        perror("Weather file does not exist");
        return;
    }

    fseek(File, 0, SEEK_END);
    long FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    char *Data = malloc(FileSize + 1);
    if (Data == NULL) {
        perror("Memory could not be allocated");
        return;
    }

    fread(Data, 1, FileSize, File);
    Data[FileSize] = '\0';
    fclose(File);

    cJSON *Json = cJSON_Parse(Data);
    if (Json == NULL) {
        perror("Can't parse JSON file");
        return;
    }

    cJSON *ListArray = cJSON_GetObjectItemCaseSensitive(Json, "list");
    if (!ListArray) {
        printf("Weather data not found\n");
        cJSON_Delete(Json);
        return;
    }

    // حلقه برای چاپ 7 پیش‌بینی
    for (int i = 0; i < 7; ++i) {
        cJSON *Forecast = cJSON_GetArrayItem(ListArray, i);
        if (Forecast != NULL) {
            cJSON *Main = cJSON_GetObjectItemCaseSensitive(Forecast, "main");
            cJSON *WeatherArray = cJSON_GetObjectItemCaseSensitive(Forecast, "weather");
            cJSON *Weather = cJSON_GetArrayItem(WeatherArray, 0);
            cJSON *DtTxt = cJSON_GetObjectItem(Forecast, "dt_txt");

            if (Main && Weather && DtTxt) {
                double Temp = cJSON_GetObjectItem(Main, "temp")->valuedouble;
                char *Description = cJSON_GetObjectItem(Weather, "description")->valuestring;
                char *Date = DtTxt->valuestring;

                printf("Date: %s\n", Date);
                printf("Temperature: %.2f°C\n", Temp);
                printf("Weather: %s\n\n", Description);
            } else {
                printf("some errores has occured %d.\n", i+1);
            }
        }
    }

    free(Data);
    cJSON_Delete(Json);
}



int main()
{
    char City[64];
    char *FileName = "Weather.json";
    printf("Enter city name: ");
    scanf("%63s", City);
    SaveWeatherInFile(City, FileName);
    ParsWeatherInFile(FileName);
    return 0;
}
