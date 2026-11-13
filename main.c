#include <stdint.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

static volatile uint32_t current_period = 50000;

void tim2_isr(void) {
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {
        timer_clear_flag(TIM2, TIM_SR_UIF);
        gpio_toggle(GPIOC, GPIO13);
    }
}

int main(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO0 | GPIO1);
    gpio_set(GPIOA, GPIO0 | GPIO1);

    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_reset_pulse(RST_TIM2);
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(TIM2, 719);
    timer_disable_preload(TIM2);
    timer_set_period(TIM2, current_period);
    timer_enable_irq(TIM2, TIM_DIER_UIE);

    nvic_enable_irq(NVIC_TIM2_IRQ);
    timer_enable_counter(TIM2);

    while (1) {
        if (!(gpio_get(GPIOA, GPIO0))) {
            if (current_period > (50000 / 64)) {
                current_period /= 2;
                timer_set_period(TIM2, current_period);
            }
            while (!(gpio_get(GPIOA, GPIO0)));
        }

        if (!(gpio_get(GPIOA, GPIO1))) {
            if (current_period < (50000 * 64)) {
                current_period *= 2;
                timer_set_period(TIM2, current_period);
            }
            while (!(gpio_get(GPIOA, GPIO1)));
        }
    }
}
