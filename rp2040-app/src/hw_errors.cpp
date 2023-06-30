#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "hw_errors.h"

static const struct gpio_dt_spec ld_oc_led = 
    GPIO_DT_SPEC_GET(DT_ALIAS(load_overcurrent_led), gpios);

hw_errors::hw_errors(void) {
	int ret;

	// if (!gpio_is_ready_dt(&ld_oc_led)) {
	// 	return 0;
	// }

	ret = gpio_pin_configure_dt(&ld_oc_led, GPIO_OUTPUT_ACTIVE);

}
