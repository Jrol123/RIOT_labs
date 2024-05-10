#include <periph/gpio.h>
#include <xtimer.h>
#include <board.h>

void blink(gpio_t led_pin)
{
    char delay = 5;
    gpio_set(led_pin);
    xtimer_sleep(delay);
    gpio_clear(led_pin);
    xtimer_sleep(delay);
}

void switch_blink(void* void_pin)
{
    gpio_t* current_pin = (gpio_t*)void_pin;
    if (*current_pin == LED3_PIN)
        *current_pin = LED4_PIN;
    else if (*current_pin == LED4_PIN)
        *current_pin = LED6_PIN;
    else if (*current_pin == LED6_PIN)
        *current_pin = LED5_PIN;
    else if (*current_pin == LED5_PIN)
        *current_pin = LED3_PIN;
}

int main(void)
{
    gpio_t cur_led = LED3_PIN;
    gpio_init_int(BTN0_PIN, GPIO_IN, GPIO_FALLING, switch_blink, &cur_led);
    while (1) {
        blink(cur_led);
        puts("blink!");
        printf("You are running RIOT on a %s board.\n", RIOT_BOARD);
    }
    return 0;
}
