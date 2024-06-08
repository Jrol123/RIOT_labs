#include <periph/gpio.h>
#include <periph/rtc.h>
#include <xtimer.h>
#include <board.h>

#define LONG_PRESS 5

// Пользовательская структура
typedef enum led_mode
{
    FIRST_RUN = 0,
    NORMAL_RUN,
    OFF_RUN
} led_mode_t;

struct tm cur_time;
struct tm next_time;

gpio_t LED_array[4] = {LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};

void change_blinkers(void *arg)
{
    (void)arg;
    uint32_t len = sizeof(LED_array) / sizeof(LED_array[0]);
    gpio_t temp_LED = LED_array[0];
    for (uint32_t i = 0; i < len - 1; i++)
    {
        LED_array[i] = LED_array[i + 1];
    }
    LED_array[len - 1] = temp_LED;
}

void alarm_func(void *arg)
{
    (void)arg;
    puts("ALARM!");
}

void change_mode(void *arg)
{
    (void)arg;

    // Активация таймера на длительное нажатие
    // В случае недодержания активируется под-функция режима (else)
    if (gpio_read(BTN0_PIN) > 0)
    {
        // debounce_timestamp = xtimer_usec_from_ticks(xtimer_now());
        puts("a");
        rtc_get_time(&next_time);
        next_time.tm_sec += LONG_PRESS;

        rtc_set_alarm(&next_time, alarm_func, NULL);
    }
    else
    {
        rtc_get_time(&cur_time);
        if (cur_time.tm_min <= next_time.tm_min && cur_time.tm_sec < next_time.tm_sec)
        {
            rtc_clear_alarm();
            puts("b");
            change_blinkers(NULL);
        }
    }
}

int main(void)
{
    gpio_t cur_LED = LED3_PIN;
    gpio_t btn_pin = BTN0_PIN;
    gpio_init_int(btn_pin, GPIO_IN, GPIO_BOTH, change_mode, NULL);
    while (true)
    {
        // if (gpio_read(btn_pin)){
        //     gpio_clear(cur_LED);
        //     puts("button pressed");
        // }
        gpio_set(cur_LED);
        xtimer_sleep(2);
        gpio_clear(cur_LED);
        xtimer_sleep(2);
    }
}