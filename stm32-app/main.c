#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <stdio.h>

#define PORT_LED GPIOA
#define PIN_LED1 GPIO5
#define PIN_LED2 GPIO7

void uart_out(char* data);

static void clock_setup(void)
{
    /* Enable GPIOC clock for LED & USARTs. */
    rcc_set_sysclk_source(RCC_CLOCK_CONFIG_HSI_PLL_64MHZ);
    rcc_periph_clock_enable(RCC_GPIOA);
    // rcc_periph_clock_enable(RCC_GPIOC);

    /* Enable clocks for USART. */
    // rcc_periph_clock_enable(RCC_USART1);
}

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

    // gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);

    // gpio_set_af(GPIOA, GPIO_AF1, GPIO9);
}

void uart_out(char* data) {
    while(*data) {
        usart_send_blocking(USART1, *data++); /* USART2: Send byte. */
    }
}

int main(void)
{
    int i = 0;
    // char buf[256];

    clock_setup();
    gpio_setup();
    // usart_setup();
    // sprintf(buf, "Hello world\r\n");
    // uart_out(buf);

    while(1) {
        gpio_set(PORT_LED, PIN_LED1 | PIN_LED2);
    }

    return 0;
}
