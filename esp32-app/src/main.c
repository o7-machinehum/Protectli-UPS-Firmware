#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>

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
static int rx_buf_pos;

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

	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
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
    const struct device *display;
    uint16_t rows;
	uint8_t ppt;
	uint8_t font_width;
	uint8_t font_height;
	char tx_buf[MSG_SIZE];

    printf("Starting Up!\n");

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&relay1, GPIO_OUTPUT_ACTIVE);

    display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display)) {
        printf("Device %s not ready\n", display->name);
        return 0;
    }

    if (display_set_pixel_format(display, PIXEL_FORMAT_MONO10) != 0) {
        printf("Failed to set required pixel format\n");
        return 0;
    }

    if (cfb_framebuffer_init(display)) {
        printf("Framebuffer initialization failed!\n");
        return 0;
    }

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

    cfb_framebuffer_clear(display, true);
    display_blanking_off(display);
    rows = cfb_get_display_parameter(display, CFB_DISPLAY_ROWS);
    ppt = cfb_get_display_parameter(display, CFB_DISPLAY_PPT);

    for (int idx = 0; idx < 42; idx++) {
        if (cfb_get_font_size(display, idx, &font_width, &font_height)) {
            break;
        }
        cfb_framebuffer_set_font(display, idx);
        printf("font width %d, font height %d\n",
               font_width, font_height);
    }

    printf("x_res %d, y_res %d, ppt %d, rows %d, cols %d\n",
           cfb_get_display_parameter(display, CFB_DISPLAY_WIDTH),
           cfb_get_display_parameter(display, CFB_DISPLAY_HEIGH),
           ppt,
           rows,
           cfb_get_display_parameter(display, CFB_DISPLAY_COLS));

    cfb_framebuffer_clear(display, true);

    cfb_draw_text(display, "Hello", 10, 10);
    // cfb_draw_rect(display, &start, &end);

    cfb_framebuffer_finalize(display);
    // cfb_invert_area(display, 0, 0, 100, 64);
    // printk("%d", display->buf[0]);

    while (1) {
        gpio_pin_toggle_dt(&led);

	    if (!k_msgq_get(&uart_msgq, &tx_buf, K_NO_WAIT)) {
			printk("Got some Data: %s\n", tx_buf);
        }

        k_msleep(SLEEP_TIME_MS);
    }
    return 0;
}
