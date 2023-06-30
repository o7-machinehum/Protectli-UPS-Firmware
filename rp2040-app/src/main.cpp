#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

// #include "hw_errors.h"
#include "pid.h"
// #include "buck.h"

#define PERIOD PWM_USEC(2) // 500Khz

static const struct pwm_dt_spec buck = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

int main(void)
{
    int ret;

    // hw_errors *hw = new hw_errors();
    pid pid0(12.0, 1.0, 0.0, 0.0);

    if (ret < 0) {
			return 0;
	}

    // Main PID Loop
    while(true) {
        pwm_set_dt(&buck, PERIOD, PERIOD * pid0.get_duc());

    }

	return 0;
}
