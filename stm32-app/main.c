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
void fault(void);

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

void fault(void) {
    gpio_set(PORT_LED, PIN_LED1);
    while(1) {};
}


extern int8_t adc_offset;
extern uint16_t adc_gain;

int main(void) {
    int i = 0;
    uint8_t ret = 0;
    char buf[128];

    clock_setup();
    gpio_setup();
    usart_setup();
    i2c_setup();

    sprintf(buf, "Starting BMS\r\n");
    uart_out(buf);

    // Set CC_CFG to 0x19 as per the datasheet
    bq76920_write_reg(CC_CFG, 0x19);
    ret = bq76920_read_reg(CC_CFG);
    if(ret != 0x19) {
        // Something is wrong with the chip
        fault();
    }

    bq76920_init();
    bq76920_clear_faults();
    bq76920_output_enable();

    #define FAULT_CNTDOWN 10

    int clear_fault = FAULT_CNTDOWN;
    while(1) {
        sprintf(buf, "C0: %d C1: %d C2: %d C3: %d C4: %d C5: %d\n\r",
            bq76920_read_cell_v(0),
            bq76920_read_cell_v(1),
            bq76920_read_cell_v(2),
            bq76920_read_cell_v(3),
            bq76920_read_cell_v(4),
            bq76920_read_cell_v(5)
        );

        uart_out(buf); // 48 121

        ret = bq76920_read_reg(SYS_STAT);
        if(ret) {
            gpio_set(PORT_LED, PIN_LED1);
            sprintf(buf, "Fault: %d\n\r", ret);
            uart_out(buf); // 48 121
            if(!clear_fault--) {
                bq76920_clear_faults();
                bq76920_output_enable();
                clear_fault = FAULT_CNTDOWN;
            }
        }

        for (i = 0; i < 5000000; i++) { /* Wait a bit. */
            __asm__("nop");
        }

    }

    return 0;
}
