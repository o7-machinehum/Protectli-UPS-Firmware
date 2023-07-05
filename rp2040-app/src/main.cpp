#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "hw_errors.h"
#include "pid.h"
#include "adc.h"
// #include "buck.h"

#define PERIOD PWM_USEC(2) // 500Khz

static const struct pwm_dt_spec buck = PWM_DT_SPEC_GET(DT_ALIAS(pwm_buck));

int main(void)
{
    int ret;

	printk("~~~ Protectli UPS ~~~\n");

    HwErrors hw_errors;
    Pid pid0(12.0, 1.0, 0.0, 0.0);

    float scales[4] = {5.1666, 14.6612, 1, 1};
    Adc adc(scales);

    printk("Vout: %d mV\t", adc.read_vout());
    printk("Vbat: %d mV\n", adc.read_vbat());
    printk("Iout: %d mV\t", adc.read_iout());
    printk("Ibat: %d mV\n", adc.read_ibat());

    if (!device_is_ready(buck.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       buck.dev->name);
		return 0;
	}


    // Main PID Loop
    while(true) {
        ret = hw_errors.errors();

        if(!ret) {
            pwm_set_dt(&buck, PERIOD, PERIOD*0.5);
            // pwm_set_dt(&buck, PERIOD, PERIOD * pid0.get_duc());

        }
        else {
            pwm_set_dt(&buck, PERIOD, 0);
            printk("UPS In Error State: %d\n", ret);
		    k_sleep(K_SECONDS(1U));
        }

    }

	return 0;
}
