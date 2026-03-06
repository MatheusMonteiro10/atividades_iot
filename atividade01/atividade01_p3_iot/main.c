#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "esp_timer.h"

// GPIOs
#define LED1 4
#define LED2 5
#define LED3 6
#define LED4 7

#define BTN_A 15
#define BTN_B 16

#define BUZZER 17

// UART
#define UART_PORT UART_NUM_0
#define BUF_SIZE 1024

volatile bool botaoB_habilitado = true;

esp_timer_handle_t periodic_timer;


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

    // buzzer
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

    for(int duty = 0; duty < 1023; duty += 10)
    {
        for(int ch = 0; ch < 4; ch++)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }

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

    for(int freq = 500; freq <= 2000; freq += 100)
    {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, 4, 512);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, 4);

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    for(int freq = 2000; freq >= 500; freq -= 100)
    {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);

        vTaskDelay(pdMS_TO_TICKS(50));
    }

}


// ---------------- INTERRUPÇÕES GPIO ----------------

static void IRAM_ATTR botaoA_isr(void* arg)
{
    gpio_set_level(LED1, !gpio_get_level(LED1));
}

static void IRAM_ATTR botaoB_isr(void* arg)
{

    if(botaoB_habilitado)
    {
        gpio_set_level(BUZZER, 1);
        vTaskDelay(pdMS_TO_TICKS(1500));
        gpio_set_level(BUZZER, 0);
    }

}


// ---------------- TIMER ----------------

void timer_callback(void* arg)
{
    gpio_set_level(LED2, !gpio_get_level(LED2));
}


// ---------------- UART ----------------

void uart_task(void *arg)
{

    uint8_t data;

    while(1)
    {
        int len = uart_read_bytes(UART_PORT, &data, 1, 100 / portTICK_PERIOD_MS);

        if(len > 0)
        {
            if(data == 'a')
                botaoB_habilitado = false;

            if(data == 'b')
                botaoB_habilitado = true;
        }
    }
}


// ---------------- GPIO INIT ----------------

void gpio_init()
{

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL<<BTN_A) | (1ULL<<BTN_B),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BTN_A, botaoA_isr, NULL);
    gpio_isr_handler_add(BTN_B, botaoB_isr, NULL);

}


// ---------------- TIMER INIT ----------------

void timer_init()
{

    const esp_timer_create_args_t timer_args = {
        .callback = &timer_callback
    };

    esp_timer_create(&timer_args, &periodic_timer);

    esp_timer_start_periodic(periodic_timer, 2000000);
}


// ---------------- UART INIT ----------------

void uart_init()
{

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_PORT, BUF_SIZE, 0, 0, NULL, 0);

    uart_param_config(UART_PORT, &uart_config);
}


// ---------------- MAIN ----------------

void app_main()
{

    pwm_init();

    gpio_init();

    timer_init();

    uart_init();

    xTaskCreate(uart_task, "uart_task", 2048, NULL, 10, NULL);

    while(1)
    {
        fading_leds();
        buzzer_variavel();
    }

}