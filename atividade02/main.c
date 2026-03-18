#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Configuração Wi-Fi (Wokwi)
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

static const char *TAG = "SNTP_EXAMPLE";

// Evento de conexão Wi-Fi
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Reconectando ao Wi-Fi...");
        esp_wifi_connect();

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi conectado!");
    }
}

// Inicialização do Wi-Fi em modo Station
void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);

    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

// Inicialização do SNTP
void initialize_sntp(void) {
    ESP_LOGI(TAG, "Inicializando SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    // Servidores do ntp.br
    esp_sntp_setservername(0, "a.ntp.br");
    esp_sntp_setservername(1, "b.ntp.br");
    esp_sntp_setservername(2, "c.ntp.br");

    esp_sntp_init();
}

// Aguarda sincronização do horário
void obtain_time(void) {
    initialize_sntp();

    time_t now = 0;
    struct tm timeinfo = { 0 };

    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Aguardando sincronização... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

// Função principal
void app_main(void) {

    // Inicializa NVS
    nvs_flash_init();

    // Inicializa Wi-Fi
    wifi_init_sta();

    // Aguarda conexão antes de sincronizar
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // Obtém horário via SNTP
    obtain_time();

    // Define fuso horário (Brasil)
    setenv("TZ", "BRT3", 1);
    tzset();

    while (1) {
        time_t now;
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);

        printf("Data/Hora: %02d/%02d/%04d %02d:%02d:%02d\n",
               timeinfo.tm_mday,
               timeinfo.tm_mon + 1,
               timeinfo.tm_year + 1900,
               timeinfo.tm_hour,
               timeinfo.tm_min,
               timeinfo.tm_sec);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}