#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/syscfg.h>

#include <stdio.h>

#define PORT_LED GPIOA
#define PIN_LED1 GPIO5
#define PIN_LED2 GPIO7

void uart_out(char* data);

static void clock_setup(void)
{
    rcc_clock_setup(&rcc_clock_config[RCC_CLOCK_CONFIG_HSI_16MHZ]);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_SYSCFG);
    rcc_periph_clock_enable(RCC_USART1);
}

// rcc_clock_setup

static void usart_setup(void)
{
    /* Setup USART parameters. */
    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_stopbits(USART1, USART_CR2_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART1);
}

static void gpio_setup(void)
{
    gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED1 | PIN_LED2);

    SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_RMP;
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF1, GPIO9);
}

void uart_out(char* data) {
    while(*data) {
        usart_send_blocking(USART1, *data++);
    }
}

int main(void)
{
    int i = 0;
    char buf[256];

    clock_setup();
    gpio_setup();
    usart_setup();
    sprintf(buf, "Hello world\r\n");
    uart_out(buf);

    gpio_toggle(PORT_LED, PIN_LED1);
    while(1) {
        gpio_toggle(PORT_LED, PIN_LED1 | PIN_LED2);

		for (i = 0; i < 1000000; i++) {	/* Wait a bit. */
			__asm__("nop");
		}

    }

    return 0;
}
