#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/syscfg.h>

#include "printf.h"

#include "bq76920.h"

#define PORT_LED GPIOA
#define PIN_LED1 GPIO5
#define PIN_LED2 GPIO7

void uart_out(char* data);
void hard_fault(void);
void delay(int t);
uint8_t check_faults(uint8_t *fault_counter);

static void clock_setup(void) {
    rcc_clock_setup(&rcc_clock_config[RCC_CLOCK_CONFIG_HSI_16MHZ]);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_SYSCFG);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_I2C1);
}

static void usart_setup(void) {
    SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_RMP;
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF1, GPIO9);

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

static void i2c_setup(void) {
    rcc_periph_reset_pulse(RST_I2C1);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set_af(GPIOB, GPIO_AF6, GPIO6 | GPIO7);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_HIGH, GPIO6 | GPIO7);

    i2c_peripheral_disable(I2C1);
    i2c_disable_stretching(I2C1);
    i2c_enable_analog_filter(I2C1);
    i2c_set_digital_filter(I2C1, 5);
    i2c_set_speed(I2C1, i2c_speed_sm_100k, 16);
    i2c_set_7bit_addr_mode(I2C1);
    i2c_peripheral_enable(I2C1);
}

static void gpio_setup(void) {
    gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED1 | PIN_LED2);

    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO8);
}

void uart_out(char* data) {
    while(*data) {
        usart_send_blocking(USART1, *data++);
    }
}

void delay(int t) {
    for (int i = 0; i < t; i++) {
        __asm__("nop");
    }
}

// Fast blink hard fault
void hard_fault(void) {
    while(1) {
        gpio_toggle(PORT_LED, PIN_LED1);
        delay(1e6);
    };
}

uint8_t check_faults(uint8_t *fault_counter) {
    uint8_t ret = 0;

    ret = bq76920_read_reg(SYS_STAT);
    if(ret) {
        gpio_set(PORT_LED, PIN_LED1);
        delay(1e6);
        if(*fault_counter <= 10) {
            bq76920_clear_faults();
            bq76920_output_enable();
            (*fault_counter)++;
        }
        else {
            // Shutdown
            bq76920_shutdown();
        }
    }
    return ret;
}

int main(void) {
    uint8_t ret = 0;
    char buf[128];
    uint8_t fault_counter = 0;

    clock_setup();
    gpio_setup();

    gpio_set(PORT_LED, PIN_LED1);
    delay(1e6);
    gpio_clear(PORT_LED, PIN_LED1);

    usart_setup();
    i2c_setup();

    sprintf(buf, "Starting BMS\r\n");
    uart_out(buf);

    // Set CC_CFG to 0x19 as per the datasheet
    bq76920_write_reg(CC_CFG, 0x19);
    ret = bq76920_read_reg(CC_CFG);
    if(ret != 0x19) {
        // Something is wrong with the chip
        hard_fault();
    }

    bq76920_init();
    bq76920_clear_faults();
    bq76920_output_enable();

    while(1) {
        sprintf(buf, "C0: %d C1: %d C2: %d C3: %d C4: %d C5: %d\n\r",
            bq76920_read_cell_v(0),
            bq76920_read_cell_v(1),
            bq76920_read_cell_v(2),
            bq76920_read_cell_v(3),
            bq76920_read_cell_v(4),
            bq76920_read_cell_v(5)
        );
        uart_out(buf);

        ret = check_faults(&fault_counter);
        if(ret) {
            sprintf(buf, "Fault: %d, Fault Counter: %d\n\r",
                ret, fault_counter);
            uart_out(buf);
        }

        gpio_toggle(PORT_LED, PIN_LED2);
        delay(5e6);
    }

    return 0;
}
