#include <periph/gpio.h>
#include <periph/rtc.h>
#include <xtimer.h>
#include <board.h>

#define LONG_PRESS 3

#define RCV_QUEUE_SIZE (0)

// ENUM режимов
typedef enum led_mode
{
    CIRCLE = 0,
    LANE
} led_mode_t;

signed short mode = CIRCLE;

signed short sub_mode = 10;

// Получение времени задержки в микросекундах
static signed int get_time(signed short dsec)
{
    return dsec * 100000;
}

// Стэк памяти
static char rcv_stack_1[THREAD_STACKSIZE_DEFAULT / 4];
static char rcv_stack_2[THREAD_STACKSIZE_DEFAULT / 4];
// Очередь входящих сообщений
static msg_t rcv_queue_1[RCV_QUEUE_SIZE];
static msg_t rcv_queue_2[RCV_QUEUE_SIZE];

struct tm cur_time;  // Текущее время
struct tm next_time; // Время конца таймера смены режимы

// Список светодиодов
gpio_t LED_array[4] = {LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};
uint32_t lenLED = 4;

// Переход против часовой стрелки
void change_blinkers(void *arg)
{
    (void)arg;
    gpio_t temp_LED = LED_array[0];
    for (uint32_t i = 0; i < lenLED - 1; i++)
    {
        LED_array[i] = LED_array[i + 1];
    }
    LED_array[lenLED - 1] = temp_LED;
}

void change_mode(void *arg)
{
    (void)arg;
    puts("long_press");
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
    switch (sub_mode)
    {
    case 10:
        sub_mode = 5;
        break;
    case 5:
        sub_mode = 3;
        break;
    case 3:
        sub_mode = 10;
        break;
    }
}

// Смена режимов
void btn_press(void *arg)
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
            if (mode == CIRCLE)
            {
                gpio_clear(LED_array[0]);
                change_blinkers(NULL);
                puts("");
            }
            else
            {
                puts("");
                // change_submode(NULL);
            }
        }
    }
}

// TODO: Разобраться, как передавать сразу несколько аргументов в функцию по таймеру
uint32_t delay = 1;

void LED_off(void *pin)
{
    puts("clear coast");
    gpio_clear(*(gpio_t *)pin);
}

void LED_on(void *pin)
{
    puts("alarm off");
    gpio_set(*(gpio_t *)pin);

    rtc_get_time(&next_time);
    next_time.tm_sec += delay;

    rtc_set_alarm(&next_time, LED_off, &pin);
}

struct tm LED_cur_time;
struct tm LED_next_time;

// *CIRCLE
void *
mode_1(void *arg)
{
    (void)arg;
    msg_t msg;

    msg_init_queue(rcv_queue_1, RCV_QUEUE_SIZE);
    while (true)
    {
        msg_receive(&msg);

        puts("circle");

        for (uint32_t i = 0; i < lenLED; i++)
        {
            if (i == 0 || i == 1)
            {
                rtc_get_time(&LED_cur_time);
                if (!(LED_cur_time.tm_min <= LED_next_time.tm_min && LED_cur_time.tm_sec < LED_next_time.tm_sec))
                {
                    puts("alarm on");
                    LED_on(&LED_array[i]);
                    rtc_get_time(&LED_next_time);
                    LED_next_time.tm_sec += delay * 2;

                    rtc_set_alarm(&LED_next_time, LED_on, &LED_array[i]);
                }
                xtimer_sleep(delay * 10);
            }
        }
    }

    return NULL;
}

// *LANE
void *mode_2(void *arg)
{
    (void)arg;
    msg_t msg;

    msg_init_queue(rcv_queue_2, RCV_QUEUE_SIZE);
    while (true)
    {
        msg_receive(&msg);

        puts("lane");

        for (uint32_t i = 0; i < lenLED; i++)
        {
            gpio_set(LED_array[i]);
            xtimer_usleep(get_time(sub_mode));
            gpio_clear(LED_array[i]);
            xtimer_usleep(get_time(sub_mode));
        }
    }

    return NULL;
}

int main(void)
{
    gpio_init_int(BTN0_PIN, GPIO_IN, GPIO_BOTH, btn_press, NULL);
    msg_t msg;
    kernel_pid_t rcv_pid_1 = thread_create(rcv_stack_1, sizeof(rcv_stack_1), THREAD_PRIORITY_MAIN - 1, 0, mode_1, NULL, "mode_1");
    kernel_pid_t rcv_pid_2 = thread_create(rcv_stack_2, sizeof(rcv_stack_2), THREAD_PRIORITY_MAIN - 1, 0, mode_2, NULL, "mode_2");

    // TODO: Разобраться, как резко переходить с одного потока на другой. Тогда можно будет переделать систему
    while (true)
    {
        msg.content.value = mode;

        if (mode == CIRCLE)
        {
            msg_try_send(&msg, rcv_pid_1);
            // xtimer_usleep(get_time(5));
        }
        else
        {
            msg_try_send(&msg, rcv_pid_2);
            // xtimer_usleep(get_time(sub_mode));
        }
    }
}