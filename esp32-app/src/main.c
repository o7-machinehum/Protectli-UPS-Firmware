/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>
#include <stdio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;
    const struct device *display;
    uint16_t rows;
	uint8_t ppt;
	uint8_t font_width;
	uint8_t font_height;

    printf("Starting Up!\n");
    gpio_pin_set_dt(&led, true);

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

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

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    while (1) {
        for (int i = 0; i < rows; i++) {
            cfb_framebuffer_clear(display, false);
            if (cfb_print(display,
                      "0123456789mMgj!\"§$%&/()=",
                      0, i * ppt)) {
                printf("Failed to print a string\n");
                continue;
            }
        }

        cfb_framebuffer_finalize(display);
        gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);
    }
    return 0;
}
