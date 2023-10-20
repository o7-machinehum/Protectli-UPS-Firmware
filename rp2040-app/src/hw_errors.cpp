#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "hw_errors.h"

/* LEDs */
static const struct gpio_dt_spec load_oc_led =
	GPIO_DT_SPEC_GET(DT_ALIAS(load_overcurrent_led), gpios);

static const struct gpio_dt_spec batt_oc_led =
	GPIO_DT_SPEC_GET(DT_ALIAS(batt_overcurrent_led), gpios);

static const struct gpio_dt_spec batt_ov_led =
	GPIO_DT_SPEC_GET(DT_ALIAS(batt_overvoltage_led), gpios);

/* Input Signals from HW */
static const struct gpio_dt_spec load_oc =
	GPIO_DT_SPEC_GET(DT_NODELABEL(load_overcurrent), gpios);

static const struct gpio_dt_spec batt_oc =
	GPIO_DT_SPEC_GET(DT_NODELABEL(batt_overcurrent), gpios);

static const struct gpio_dt_spec batt_ov =
	GPIO_DT_SPEC_GET(DT_NODELABEL(batt_overvoltage), gpios);

HwErrors::HwErrors() : last_error_code(0), error_code(0)
{
	gpio_pin_configure_dt(&load_oc_led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&batt_oc_led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&batt_ov_led, GPIO_OUTPUT_INACTIVE);

	gpio_pin_configure_dt(&load_oc, GPIO_INPUT);
	gpio_pin_configure_dt(&batt_oc, GPIO_INPUT);
	gpio_pin_configure_dt(&batt_ov, GPIO_INPUT);
}

uint8_t HwErrors::check(void)
{
	memset(&errorCodes, 0x00, sizeof(errorCodes));
	error_code = 0x00;

	/* Load Overcurrent */
	if (gpio_pin_get_dt(&load_oc)) {
		printk("Load Overcurrent! \n");
		errorCodes.load_oc = true;
		gpio_pin_set_dt(&load_oc_led, errorCodes.load_oc);
		error_code |= 1;
	}

	/* Battery Overcurrent */
	if (gpio_pin_get_dt(&batt_oc)) {
		printk("Battery Overcurrent! \n");
		errorCodes.batt_oc = true;
		gpio_pin_set_dt(&batt_oc_led, errorCodes.batt_oc);
		error_code |= 2;
	}

	/* Battery Overvoltage */
	if (gpio_pin_get_dt(&batt_ov)) {
		printk("Battery Overvoltage! \n");
		errorCodes.batt_ov = true;
		gpio_pin_set_dt(&batt_ov_led, errorCodes.batt_ov);
		error_code |= 4;
	}

	if (last_error_code != error_code) {
		printk("New Errorlevel: %d\n", error_code);
	}

	last_error_code = error_code;

	return error_code;
}
