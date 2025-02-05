#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define BUTTON 5
#define DEBOUNCE_TIME 50 // 50ms debounce

volatile bool button_pressed = false;
volatile int led_state = 0;
volatile bool sequence_running = false; // Flag para indicar se a sequência está em execução

// Callback para desligar LEDs (sem argumentos)
void turn_off_callback(void) {
    if (led_state == 3) {
        gpio_put(LED_BLUE, 0);
    } else if (led_state == 2) {
        gpio_put(LED_GREEN, 0);
    } else if (led_state == 1) {
        gpio_put(LED_RED, 0);
        sequence_running = false; // A sequência terminou
        button_pressed = false; // Libera o botão para novo acionamento
        led_state = 0; // Reset do estado dos LEDs
        return; // Importante: Sai do callback para evitar chamadas desnecessárias
    }
    led_state--;
}

// Callback do botão (com debounce)
void button_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON && (events & GPIO_IRQ_EDGE_FALL)) {
        if (!button_pressed && !sequence_running) { // Verifica se o botão já foi pressionado e se a sequência não está em execução
            button_pressed = true;
            sequence_running = true; // A sequência começou
            led_state = 3;
            gpio_put(LED_RED, 1);
            gpio_put(LED_BLUE, 1);
            gpio_put(LED_GREEN, 1);

            // Configura o tempo para o primeiro desligamento (usando timer raw)
            uint32_t target_time = time_us_32() + (3000 * 1000); // 3 segundos

            // Loop de espera (bloqueante) para cada LED
            while (time_us_32() < target_time);
            turn_off_callback();

            target_time = time_us_32() + (3000 * 1000);
            while (time_us_32() < target_time);
            turn_off_callback();

            target_time = time_us_32() + (3000 * 1000);
            while (time_us_32() < target_time);
            turn_off_callback();
        }
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    irq_set_enabled(0, true); // Habilita interrupções globalmente
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    while (1) {
        // Loop principal vazio
    }
}