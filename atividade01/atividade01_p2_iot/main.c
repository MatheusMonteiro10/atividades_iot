#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// GPIOs
#define LED1 4
#define LED2 5
#define LED3 6
#define LED4 7

#define BUZZER 17


// ---------------- PWM INIT ----------------

void pwm_init()
{

    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer);

    int leds[4] = {LED1, LED2, LED3, LED4};

    // LEDs
    for(int i = 0; i < 4; i++)
    {
        ledc_channel_config_t channel = {
            .channel = i,
            .duty = 0,
            .gpio_num = leds[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint = 0,
            .timer_sel = LEDC_TIMER_0
        };

        ledc_channel_config(&channel);
    }

    // Buzzer
    ledc_channel_config_t buzzer_channel = {
        .channel = 4,
        .duty = 0,
        .gpio_num = BUZZER,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0
    };

    ledc_channel_config(&buzzer_channel);
}


// ---------------- FADING ----------------

void fading_leds()
{

    // Aumenta brilho
    for(int duty = 0; duty < 1023; duty += 10)
    {
        for(int ch = 0; ch < 4; ch++)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Diminui brilho
    for(int duty = 1023; duty > 0; duty -= 10)
    {
        for(int ch = 0; ch < 4; ch++)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }

}


// ---------------- BUZZER ----------------

void buzzer_variavel()
{

    // frequência crescente
    for(int freq = 500; freq <= 2000; freq += 100)
    {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, 4, 512);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, 4);

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // frequência decrescente
    for(int freq = 2000; freq >= 500; freq -= 100)
    {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);

        vTaskDelay(pdMS_TO_TICKS(50));
    }

}


// ---------------- MAIN ----------------

void app_main()
{

    pwm_init();

    while(1)
    {
        fading_leds();
        buzzer_variavel();
    }

}