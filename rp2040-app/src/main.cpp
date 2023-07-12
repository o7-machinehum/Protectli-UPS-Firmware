#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "hw_errors.h"
#include "pid.h"
#include "adc.h"
// #include "buck.h"

#define PERIOD PWM_NSEC(2000) // 500Khz
#define STACKSIZE 16384

static const struct pwm_dt_spec boost = PWM_DT_SPEC_GET(DT_ALIAS(pwm_boost));
static const struct pwm_dt_spec buck = PWM_DT_SPEC_GET(DT_ALIAS(pwm_buck));

// This needs to be implemented to see if we're charing or discharging
// ie: We have power, or we don't.
bool vin_detect(void) {
    return true;
}

void buckboost(void)
{
    int ret;
    float drive;
    float actual;

	printk("~~~ Protectli UPS ~~~\n");

    HwErrors hw_errors;
    Pid buck_pid(12.0, 0.03, 0.0001, 0.000002);
    Pid boost_pid(15.0, 0.001, 0.0, 0.0);

    Adc adc;

	k_sleep(K_MSEC(50U));
    printk("Vout: %d mV\t", adc.read_vout());
    printk("Vbat: %d mV\n", adc.read_vbat());
    printk("Iout: %d mV\t", adc.read_iout());
    printk("Ibat: %d mV\n", adc.read_ibat());

    if (!device_is_ready(buck.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       buck.dev->name);
	}

    if (!device_is_ready(boost.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       boost.dev->name);
	}

    pwm_set_dt(&boost, PERIOD, PERIOD * 0.75);
    pwm_set_dt(&buck, PERIOD, PERIOD * 0.5);
    while(true) {};

    // Main PID Loop
    bool first = true;
    while(true) {
        ret = hw_errors.errors();

        if(!ret & !vin_detect()) {
            if(first) {
                printk("Entering Buck State\n");
                first = false;
            }
            actual =  adc.read_vout();
            actual = actual / 1000;
            buck_pid.compute(actual);
            drive = buck_pid.get_duc();
            printk("%f\n", drive);
            if(drive < 0)
                drive = 0;

            pwm_set_dt(&boost, PERIOD, 0);
            pwm_set_dt(&buck, PERIOD, PERIOD * drive);

        }
        else if(!ret & vin_detect()) {
            if(first) {
                printk("Entering Boost State\n");
                first = false;
            }
            actual = adc.read_vbat();
            actual = actual / 1000;
            if(actual > 16) {
                pwm_set_dt(&boost, PERIOD, 0);
                printk("Safety Trip\n");
                while(1) {};
            }
            boost_pid.compute(actual);
            drive = boost_pid.get_duc();
            // printk("%f\n", actual);
            if(drive < 0)
                drive = 0;

            pwm_set_dt(&buck, PERIOD, 0);
            pwm_set_dt(&boost, PERIOD, PERIOD * drive);
        }
        else {
            if(first) {
                printk("UPS In Error State: %d\n", ret);
                first = false;
            }
            pwm_set_dt(&buck, PERIOD, 0);
		    k_sleep(K_SECONDS(1U));
        }

    }

}

K_THREAD_DEFINE(buckboost_id, STACKSIZE, buckboost, NULL, NULL, NULL,
		0, 0, 0);
