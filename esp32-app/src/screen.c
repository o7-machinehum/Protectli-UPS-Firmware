#include "screen.h"
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>

const struct device *display;

int screen_init()
{
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

	return 0;
}

int screen_set_font()
{
}

int screen_draw()
{
	uint16_t rows;
	uint8_t ppt;
	uint8_t font_width;
	uint8_t font_height;

	cfb_framebuffer_clear(display, true);
	display_blanking_off(display);
	rows = cfb_get_display_parameter(display, CFB_DISPLAY_ROWS);
	ppt = cfb_get_display_parameter(display, CFB_DISPLAY_PPT);

	for (int idx = 0; idx < 42; idx++) {
		if (cfb_get_font_size(display, idx, &font_width, &font_height)) {
			break;
		}
		if (font_height == 16) {
			cfb_framebuffer_set_font(display, idx);
		}

		printf("font width %d, font height %d\n", font_width, font_height);
	}

	printf("x_res %d, y_res %d, ppt %d, rows %d, cols %d\n",
	       cfb_get_display_parameter(display, CFB_DISPLAY_WIDTH),
	       cfb_get_display_parameter(display, CFB_DISPLAY_HEIGH), ppt, rows,
	       cfb_get_display_parameter(display, CFB_DISPLAY_COLS));

	cfb_framebuffer_clear(display, true);

	cfb_draw_text(display, "Hello_new", 10, 10);
	// cfb_draw_rect(display, &start, &end);

	cfb_framebuffer_finalize(display);
	// cfb_invert_area(display, 0, 0, 100, 64);
	// printk("%d", display->buf[0]);
}
