#include "screen.h"
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>

#include "msg.h"
#include <string.h>
#include <stdio.h>

const struct device *display;
char buf[256] = {};

int screen_init()
{
	uint16_t rows;
	uint8_t ppt;
	uint8_t font_width;
	uint8_t font_height;

	display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	if (!device_is_ready(display)) {
		printk("Device %s not ready\n", display->name);
		return 0;
	}

	if (display_set_pixel_format(display, PIXEL_FORMAT_MONO10) != 0) {
		printk("Failed to set required pixel format\n");
		return 0;
	}

	if (cfb_framebuffer_init(display)) {
		printk("Framebuffer initialization failed!\n");
		return 0;
	}

	display_blanking_off(display);
	rows = cfb_get_display_parameter(display, CFB_DISPLAY_ROWS);
	ppt = cfb_get_display_parameter(display, CFB_DISPLAY_PPT);

	for (int idx = 0; idx < 42; idx++) {
		if (cfb_get_font_size(display, idx, &font_width,
				      &font_height)) {
			break;
		}
		if (font_height == 16) {
			cfb_framebuffer_set_font(display, idx);
		}

		printk("font width %d, font height %d\n", font_width,
		       font_height);
	}

	printk("x_res %d, y_res %d, ppt %d, rows %d, cols %d\n",
	       cfb_get_display_parameter(display, CFB_DISPLAY_WIDTH),
	       cfb_get_display_parameter(display, CFB_DISPLAY_HEIGH), ppt, rows,
	       cfb_get_display_parameter(display, CFB_DISPLAY_COLS));


	cfb_framebuffer_invert(display);
	return 0;
}

int screen_draw(struct Msg msg)
{
	memset(buf, 0x00, sizeof(buf));
	sprintf(buf, "Vo: %.2fV", (float)msg.voltage/1000);
	cfb_framebuffer_clear(display, true);
	cfb_draw_text(display, buf, 0, 0);
	cfb_framebuffer_finalize(display);
	// cfb_invert_area(display, 0, 0, 128, 64);
}
