#include <periph/gpio.h>
#include <xtimer.h>
#include <board.h>

void stop(void* pin2){
    puts("func");
    // (void)pin2;
    gpio_t btn_pin = BTN0_PIN;
    gpio_t* cur_LED = (gpio_t*) pin2;

    gpio_clear(*cur_LED);

    while (gpio_read(btn_pin))
    {
        // xtimer_sleep(1); //! FAILED ASSERTION
        puts("stop");
    }
    puts("go");
}

int main (void){
    gpio_t cur_LED = LED3_PIN;
    gpio_t btn_pin = BTN0_PIN;
    gpio_init_int(btn_pin, GPIO_IN, GPIO_RISING, stop,  &cur_LED);
    while(true){
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