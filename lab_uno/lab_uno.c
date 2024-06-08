#include <periph/gpio.h>
#include <periph/rtc.h>
#include <xtimer.h>
#include <board.h>

#define long_press 5

// uint32_t debounce_timestamp = 0;
struct tm cur_time;
struct tm next_time;

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
        next_time.tm_sec += long_press;

        rtc_set_alarm(&next_time, alarm_func, NULL);
    }
    else
    {
        rtc_get_time(&cur_time);
        if (cur_time.tm_min <= next_time.tm_min && cur_time.tm_sec < next_time.tm_sec)
        {
            puts("b");
            rtc_clear_alarm();
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