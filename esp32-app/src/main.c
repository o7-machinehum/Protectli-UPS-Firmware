#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/spi.h>
#include <stdio.h>

#define SLEEP_TIME_MS   5000
#define LED0_NODE DT_ALIAS(led0)

static struct k_poll_signal spi_slave_done_sig
    = K_POLL_SIGNAL_INITIALIZER(spi_slave_done_sig);

static const struct gpio_dt_spec led
    = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

static const struct gpio_dt_spec relay1
    = GPIO_DT_SPEC_GET(DT_NODELABEL(relay1), gpios);

static const struct gpio_dt_spec relay2
    = GPIO_DT_SPEC_GET(DT_NODELABEL(relay2), gpios);

#define SPI_OP  SPI_OP_MODE_SLAVE | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE
static const struct spi_dt_spec spi2
    = SPI_DT_SPEC_GET(DT_NODELABEL(mcp3201), SPI_OP, 0);

// const struct device *dev_spi;
//
// static const struct spi_config spi_slave_cfg = {
// 	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
// 				 SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_OP_MODE_SLAVE,
// 	.frequency = 4000000,
// 	.slave = 0,
// };

static int spi_check_for_messages(void) {
    int signaled, result;
    k_poll_signal_check(&spi_slave_done_sig, &signaled, &result);
    if(signaled) {
        return 1;
    }
    return 0;
}

static uint8_t slave_tx_buffer[2];
static uint8_t slave_rx_buffer[2];
static int spi_slave_write(void) {

    const struct spi_buf s_tx_buf = {
		.buf = slave_tx_buffer,
		.len = sizeof(slave_tx_buffer)
	};

    const struct spi_buf_set s_tx = {
		.buffers = &s_tx_buf,
		.count = 1
	};

    struct spi_buf s_rx_buf = {
		.buf = slave_rx_buffer,
		.len = sizeof(slave_rx_buffer),
	};

    const struct spi_buf_set s_rx = {
		.buffers = &s_rx_buf,
		.count = 1
	};

    k_poll_signal_reset(&spi_slave_done_sig);
    // int error = spi_transceive_signal(dev_spi, &spi_slave_cfg, &s_tx, &s_rx, &spi_slave_done_sig);
    int error = spi_transceive_dt(&spi2, &s_tx, &s_rx);

	if(error != 0){
		printk("SPI slave transceive error: %i\n", error);
		return error;
	}
	printk("Did SPI Thing");
    return 0;
}

int main(void) {
    int ret;
    const struct device *display;
    uint16_t rows;
	uint8_t ppt;
	uint8_t font_width;
	uint8_t font_height;

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

    printf("Initialized %s\n", display->name);

    if (cfb_framebuffer_init(display)) {
        printf("Framebuffer initialization failed!\n");
        return 0;
    }

    // dev_spi = DEVICE_DT_GET(DT_CHOSEN(zephyr_spi));
    // if (dev_spi == NULL) {
    //     printk("Could not get SPI device\n");
    //     return 0;
    // }

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

    spi_slave_write();

    while (1) {
        gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);
        if(spi_check_for_messages()) {
            printk("SPI SLAVE RX: 0x%.2x, 0x%.2x\n",
                slave_rx_buffer[0], slave_rx_buffer[1]);
        }
    }
    return 0;
}
