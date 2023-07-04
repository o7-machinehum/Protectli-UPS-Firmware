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

static const struct gpio_dt_spec batt_ot_led =
    GPIO_DT_SPEC_GET(DT_ALIAS(batt_overtemp_led), gpios);

/* Input Signals from HW */
static const struct gpio_dt_spec load_oc =
    GPIO_DT_SPEC_GET(DT_NODELABEL(load_overcurrent), gpios);

static const struct gpio_dt_spec batt_oc =
    GPIO_DT_SPEC_GET(DT_NODELABEL(batt_overcurrent), gpios);

static const struct gpio_dt_spec batt_ov =
    GPIO_DT_SPEC_GET(DT_NODELABEL(load_overcurrent), gpios);

static const struct gpio_dt_spec batt_ot =
    GPIO_DT_SPEC_GET(DT_NODELABEL(load_overcurrent), gpios);

HwErrors::HwErrors() {

    gpio_pin_configure_dt(&load_oc_led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&batt_oc_led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&batt_ov_led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&batt_ot_led, GPIO_OUTPUT_ACTIVE);

    gpio_pin_configure_dt(&load_oc, GPIO_INPUT);
    gpio_pin_configure_dt(&batt_oc, GPIO_INPUT);
    gpio_pin_configure_dt(&batt_ov, GPIO_INPUT);
    gpio_pin_configure_dt(&batt_ot, GPIO_INPUT);

}


uint8_t HwErrors::errors(void) {
    memset(&errorCodes, 0x00, sizeof(errorCodes));
    uint8_t ret = 0x00;
    static uint8_t last_ret;

    /* Load Overcurrent */
    errorCodes.load_oc = gpio_pin_get_dt(&load_oc);
    gpio_pin_set_dt(&load_oc_led, errorCodes.load_oc);
    ret |= errorCodes.load_oc << 0;

    /* Battery Overcurrent */
    errorCodes.batt_oc = gpio_pin_get_dt(&batt_oc);
    gpio_pin_set_dt(&batt_oc_led, errorCodes.batt_oc);
    ret |= errorCodes.batt_oc << 1;

    /* Battery Overvoltage */
    errorCodes.batt_ov = gpio_pin_get_dt(&batt_ov);
    gpio_pin_set_dt(&batt_ov_led, errorCodes.batt_ov);
    ret = errorCodes.batt_ov << 2;

    /* Battery Overtemp */
    errorCodes.batt_ot = gpio_pin_get_dt(&batt_ot);
    gpio_pin_set_dt(&batt_ot_led, errorCodes.batt_ot);
    ret = errorCodes.batt_ot << 3;

    if(last_ret != ret)
        printk("New Errorlevel: %d", ret);

    last_ret = ret;

    return ret;
}
