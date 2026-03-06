#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Definição dos GPIOs dos LEDs
#define LED1 4
#define LED2 5
#define LED3 6
#define LED4 7

// Array para facilitar manipulação
int leds[4] = {LED1, LED2, LED3, LED4};

// Configuração inicial dos LEDs
void configurar_leds()
{
    for(int i = 0; i < 4; i++)
    {
        gpio_reset_pin(leds[i]);
        gpio_set_direction(leds[i], GPIO_MODE_OUTPUT);
        gpio_set_level(leds[i], 0);
    }
}


// ---------------- FASE 1 ----------------
// Contador binário de 0 até 15

void contador_binario()
{
    for(int valor = 0; valor < 16; valor++)
    {
        gpio_set_level(LED4, (valor >> 0) & 1); // LSB
        gpio_set_level(LED3, (valor >> 1) & 1);
        gpio_set_level(LED2, (valor >> 2) & 1);
        gpio_set_level(LED1, (valor >> 3) & 1); // MSB

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


// ---------------- FASE 2 ----------------
// Sequência LED1 → LED4 → LED1

void sequencia_varredura()
{

    // ida
    for(int i = 0; i < 4; i++)
    {
        gpio_set_level(leds[i], 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(leds[i], 0);
    }

    // volta
    for(int i = 3; i >= 0; i--)
    {
        gpio_set_level(leds[i], 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(leds[i], 0);
    }
}


// ---------------- MAIN ----------------

void app_main()
{
    configurar_leds();

    while(1)
    {
        contador_binario();
        sequencia_varredura();
    }
}