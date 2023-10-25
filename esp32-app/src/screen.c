#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/display/cfb.h>

#include "screen.h"
#include "protectli_logo.h"
#include "fw_version.h"
#include "msg.h"

#include <string.h>
#include <stdio.h>

const struct device *display;
char buf[256] = {};
extern struct k_msgq msgq;

int screen_init(void)
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

void screen_draw_battery()
{
	struct cfb_position p1, p2;
	p1.x = 98;
	p1.y = 50;
	p2.x = 120;
	p2.y = 60;
	cfb_draw_rect(display, &p1, &p2);
	p1.x = 121;
	p1.y = 52;
	p2.x = 124;
	p2.y = 58;
	cfb_draw_rect(display, &p1, &p2);
}

// Capacity 0 -> 100
void screen_fill_battery(int capacity)
{
	struct cfb_position p1, p2;
	capacity = capacity * 0.22;
	p1.y = 50;
	p2.y = 60;
	for (int i = 0; i <= 22; i++) {
		if (capacity > i) {
			p1.x = 98 + i;
			p2.x = 98 + i;
			cfb_draw_line(display, &p1, &p2);
		}
	}
}

void screen_draw_intro(struct Msg msg)
{
	memset(buf, 0x00, sizeof(buf));
	cfb_framebuffer_clear(display, true);

	draw_logo(display);
	sprintf(buf, "R1 FW: %d.%d", FW_VERSION, FW_SUBVERSION);
	cfb_draw_text(display, buf, 0, 50);

	cfb_framebuffer_finalize(display);
}

void screen_draw_error()
{
	memset(buf, 0x00, sizeof(buf));
	cfb_framebuffer_clear(display, true);

	sprintf(buf, "Error! No msg");
	cfb_draw_text(display, buf, 0, 0);
	sprintf(buf, "from RP2040!");
	cfb_draw_text(display, buf, 0, 20);

	cfb_framebuffer_finalize(display);
}
void screen_draw_banner(struct Msg msg)
{
	memset(buf, 0x00, sizeof(buf));

	screen_draw_battery();
	screen_fill_battery(msg.gas);
	if (msg.state == MSG_STATE_CHARGING) {
		sprintf(buf, "Char");
	} else {
		sprintf(buf, "Dischar");
	}
	cfb_draw_text(display, buf, 0, 46);
}

void screen_draw_vout(struct Msg msg)
{
	memset(buf, 0x00, sizeof(buf));
	cfb_framebuffer_clear(display, true);

	sprintf(buf, "Vout: %.2fV", (float)msg.vout / 1000);
	cfb_draw_text(display, buf, 0, 0);
	sprintf(buf, "Iout: %.1fA", (float)msg.iout / 1000);
	cfb_draw_text(display, buf, 0, 20);
	screen_draw_banner(msg);

	cfb_framebuffer_finalize(display);
}

void screen_draw_vbat(struct Msg msg)
{
	memset(buf, 0x00, sizeof(buf));
	cfb_framebuffer_clear(display, true);

	sprintf(buf, "Vbat: %.2fV", (float)msg.vbat / 1000);
	cfb_draw_text(display, buf, 0, 0);
	sprintf(buf, "Ibat: %.1fA", (float)msg.ibat / 1000);
	cfb_draw_text(display, buf, 0, 20);
	screen_draw_banner(msg);

	cfb_framebuffer_finalize(display);
}

void screen_thread(void *, void *, void *)
{
	int ret = 0;
	enum screen_state state = INTRO;

	while (true) {
		struct Msg msg = {};
		ret = k_msgq_get(&msgq, &msg, K_MSEC(1000));

		if (ret) {
			screen_draw_error();
			k_sleep(K_MSEC(5000U));
		} else if (state == INTRO) {
			screen_draw_intro(msg);
			k_sleep(K_MSEC(5000U));
			state = VBAT;
		} else if (state == VBAT) {
			screen_draw_vbat(msg);
			k_sleep(K_MSEC(3000U));
			state = VOUT;
		} else if (state == VOUT) {
			screen_draw_vout(msg);
			k_sleep(K_MSEC(3000U));
			state = VBAT;
		}
	}
}
