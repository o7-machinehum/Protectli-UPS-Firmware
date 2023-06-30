#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

// #include "hw_errors.h"
// #include "pid.h"
// #include "buck.h"

static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));


int main(void)
{
    int ret;

    // hw_errors *hw = new hw_errors();
    // pid *pid0 = new pid(12.0, 1.0, 0.0, 0.0);

    if (ret < 0) {
			return 0;
	}
    
    // Main PID Loop
    while(true) {
        
//		pwm_set_dt(&pwm_led0, period, period * pid.get_duc());


    }

	return 0;
}
