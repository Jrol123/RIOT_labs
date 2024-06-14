#include <periph/gpio.h>
#include <periph/rtc.h>
#include <periph/pwm.h>
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
gpio_t LED_array[4] = {LED6_PIN, LED4_PIN, LED3_PIN, LED5_PIN};
gpio_t LED_PWM_array[4] = {LED3_PIN, LED5_PIN};
uint32_t pwm_freq[2] = {0, 0};
uint32_t lenLED = 4;
bool LED_is_led[4] = {false, false, false, false};

// TODO: Сделать зеркалтное отражение
//  Переход против часовой стрелки
void change_blinkers(void *arg)
{
    (void)arg;
    gpio_t temp_LED = LED_array[0];
    for (uint32_t i = 0; i < lenLED - 1; i++)
    {
        LED_is_led[i] = false;
        LED_array[i] = LED_array[i + 1];
    }
    LED_array[lenLED - 1] = temp_LED;
    LED_is_led[lenLED - 1] = false;
}

void init_PWM_LED(void *arg)
{
    (void)arg;
    for (uint32_t i = 0; i < 2; i++)
    {
        pwm_init(PWM_DEV(i + 1), LED_PWM_array[i], 1000, 249);
    }
}

void init_GPIO_LED(void *arg)
{
    (void)arg;
    for (uint32_t i = 0; i < 2; i++)
    {
        gpio_init(LED_PWM_array[i], GPIO_OUT);
    }
}

void change_mode(void *arg)
{
    (void)arg;
    puts("long_press");
    if (mode == CIRCLE)
    {
        mode = LANE;
        init_PWM_LED(NULL);
    }
    else
    {
        init_GPIO_LED(NULL);
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
            }
            else
            {
                change_submode(NULL);
            }
        }
    }
}

void LED_switch_blink(gpio_t pin, uint32_t index_pin)
{
    if (LED_is_led[index_pin])
    {
        gpio_clear(pin);
        // printf("Took off pin: %lu", index_pin);
    }
    else
    {
        gpio_set(pin);
        // printf("Took on pin: %lu", index_pin);
    }
    LED_is_led[index_pin] = !LED_is_led[index_pin];
}

uint32_t delay = 1;

bool is_half = false;

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
            switch (i)
            {
            case (0):
                if (!is_half)
                {
                    LED_switch_blink(LED_array[i], i);
                }
                break;
            case (1):
                LED_switch_blink(LED_array[i], i);
                break;
            // 0 - 249
            case (2):
                pwm_freq[i - 2] += 1;
                pwm_set(PWM_DEV(i - 1), pwm_freq[i - 2], 0);
                break;
            case(3):
                pwm_freq[i - 2] += 2;
                pwm_set(PWM_DEV(i - 1), pwm_freq[i - 2], 0);
                break;
            }
        }
        xtimer_usleep(get_time(5));
        is_half = !is_half;
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

    init_PWM_LED(NULL);

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