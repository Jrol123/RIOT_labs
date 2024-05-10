#include <stdio.h>
#include <periph/gpio.h>
#include <periph/pwm.h>
#include <xtimer.h>
#include <board.h>

int main(void)
{
    uint8_t i = 0;
    int8_t cmp = 1;
    
    pwm_init(PWM_DEV(1),PWM_LEFT, 1000, 256);
    puts("Hello, world!"); // Куда это выводится?
    
    for (i = 0; ;i += cmp) {
        pwm_set(PWM_DEV(1),0,i);
        xtimer_usleep(10000);
        if (i == 255)
            cmp = -1;
        if (i == 0)
            cmp = 1;
    }
    return 0;
}
