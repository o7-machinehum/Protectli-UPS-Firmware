#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>

#include "../../common/msg.h"
#include "screen.h"

#define SLEEP_TIME_MS   5000
#define LED0_NODE DT_ALIAS(led0)

#define MSG_SIZE 32

K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static struct k_poll_signal spi_slave_done_sig
    = K_POLL_SIGNAL_INITIALIZER(spi_slave_done_sig);

static const struct gpio_dt_spec led
    = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

static const struct gpio_dt_spec relay1
    = GPIO_DT_SPEC_GET(DT_NODELABEL(relay1), gpios);

static const struct gpio_dt_spec relay2
    = GPIO_DT_SPEC_GET(DT_NODELABEL(relay2), gpios);

static const struct device *const uart_dev
    = DEVICE_DT_GET(DT_CHOSEN(zephyr_mcu_uart));

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos = 0;

struct Msg msg = {};

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (!uart_irq_rx_ready(uart_dev)) {
        return;
    }
    while (uart_fifo_read(uart_dev, &c, 1) == 1) {
        printk("int: %d\n", c);
        *((uint8_t*)(&msg) + rx_buf_pos++) = c;
        if(rx_buf_pos >= sizeof(struct Msg)) {
            // printk("thing: %d\n", msg.voltage);
            // printk("thing: %d\n", msg.current);
        }
    }

    /* read until FIFO empty */
    // while (uart_fifo_read(uart_dev, &c, 1) == 1) {
    //     printk("%c", c);
    //     if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
    //         /* terminate string */
    //         rx_buf[rx_buf_pos] = '\0';

    //         /* if queue is full, message is silently dropped */
    //         k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

    //         printk("rx_buf: %s\n", rx_buf);

    //         /* reset the buffer (it was copied to the msgq) */
    //         rx_buf_pos = 0;
    //     } else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
    //         printk("Add \n");
    //         rx_buf[rx_buf_pos++] = c;
    //     }
    //     /* else: characters beyond buffer size are dropped */
    // }
}

void print_uart(char *buf)
{
    int msg_len = strlen(buf);

    for (int i = 0; i < msg_len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}

int main(void) {
    int ret;
    char tx_buf[MSG_SIZE];

    printf("Starting Up!\n");

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

	screen_init();
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay1, GPIO_OUTPUT_ACTIVE);
    
    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!");
        return 0;
    }

    /* configure interrupt and callback to receive data */
    ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

    if (ret < 0) {
        if (ret == -ENOTSUP) {
            printk("Interrupt-driven UART API support not enabled\n");
        } else if (ret == -ENOSYS) {
            printk("UART device does not support interrupt-driven API\n");
        } else {
            printk("Error setting UART callback: %d\n", ret);
        }
        return 0;
    }

    uart_irq_rx_enable(uart_dev);
	screen_draw();
    // print_uart("Testing");
    
    while (1) {
        gpio_pin_toggle_dt(&led);

        if (!k_msgq_get(&uart_msgq, &tx_buf, K_NO_WAIT)) {
            printk("Data: %s", tx_buf);
        }

        k_sleep(K_MSEC(100U));

    }
    return 0;
}
