#include <periph/gpio.h>
#include <periph/rtc.h>
#include <xtimer.h>
#include <board.h>

#define LONG_PRESS 5

#define RCV_QUEUE_SIZE (2)

// ENUM режимов
typedef enum led_mode
{
    CIRCLE = 0,
    LANE
} led_mode_t;

signed short mode = CIRCLE;

signed short sub_mode_2 = 10;

// Получение времени задержки для LANE
static signed int get_time(signed short dsec)
{
    return dsec * 100000;
}

// Стэк памяти
static char rcv_stack[THREAD_STACKSIZE_DEFAULT / 4];
// Очередь входящих сообщений
static msg_t rcv_queue[RCV_QUEUE_SIZE];

struct tm cur_time;  // Текущее время
struct tm next_time; // Время конца таймера смены режимы

// Список светодиодов
gpio_t LED_array[4] = {LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};

// Переход против часовой стрелки
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

void change_mode(void *arg)
{
    (void)arg;
    puts("long_press");
    puts("ALARM!");
    if (mode == CIRCLE)
    {
        mode = LANE;
    }
    else
    {
        mode = CIRCLE;
    }
}

void change_submode(void *arg)
{
    (void)arg;
    switch (sub_mode_2)
    {
        case 10:
            sub_mode_2 = 5;
            break;
        case 5:
            sub_mode_2 = 3;
            break;
        case 3:
            sub_mode_2 = 10;
            break;
    }
}

// Смена режимов
void change_mode(void *arg)
{
    (void)arg;

    // Активация таймера на длительное нажатие
    // В случае недодержания активируется под-функция режима (else)
    if (gpio_read(BTN0_PIN) > 0)
    {
        // debounce_timestamp = xtimer_usec_from_ticks(xtimer_now());
        rtc_get_time(&next_time);
        next_time.tm_sec += LONG_PRESS;

        rtc_set_alarm(&next_time, change_mode, NULL);
    }
    else
    {
        rtc_get_time(&cur_time);
        // Короткое нажатие
        if (cur_time.tm_min <= next_time.tm_min && cur_time.tm_sec < next_time.tm_sec)
        {
            rtc_clear_alarm();
            puts("short_press");
            if(mode == CIRCLE)
            {
                change_blinkers(NULL);
            }
            else
            {
                change_submode(NULL);
            }
        }
    }
}

void *rcv(void *arg)
{
    (void)arg;
    msg_t msg;

    msg_init_queue(rcv_queue, RCV_QUEUE_SIZE);
    while (true)
    {
        msg_receive(&msg);

        if (msg.content.value == CIRCLE)
        {
            puts("circle");
        }
        else if (msg.content.value == LANE)
        {
            puts("lane");
        }
    }

    return NULL;
}

int main(void)
{
    gpio_init_int(BTN0_PIN, GPIO_IN, GPIO_BOTH, change_mode, NULL);
    msg_t msg;
    kernel_pid_t rcv_pid = thread_create(rcv_stack, sizeof(rcv_stack), THREAD_PRIORITY_MAIN - 1, 0, rcv, NULL, "rcv");

    while (true)
    {
        msg.content.value = mode;

        msg_try_send(&msg, rcv_pid);

        if(mode == CIRCLE)
        {
            xtimer_usleep(get_time(5));
        }
        else
        {
            xtimer_usleep(get_time(sub_mode_2));
        }
    }
}