#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "hw_errors.h"
#include "pid.h"
#include "adc.h"
// #include "buck.h"

#define PERIOD PWM_NSEC(2000) // 500Khz

static const struct pwm_dt_spec buck = PWM_DT_SPEC_GET(DT_ALIAS(pwm_buck));

int main(void)
{
    int ret;
    float drive;
    float actual;

	printk("~~~ Protectli UPS ~~~\n");

    HwErrors hw_errors;
    Pid pid0(12.0, 0.03, 0.0001, 0.00001);

    Adc adc;

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
    // ret = pwm_set_dt(&buck, PERIOD, PERIOD*0.08);
    // printk("%d", buck.channel);
    // if(ret) {
    //     printk("%d", ret);
    // }

    // while(true) {
	//     k_sleep(K_SECONDS(1U));
    // };
    while(true) {
        ret = hw_errors.errors();

        if(!ret) {
            actual =  adc.read_vout();
            actual = actual / 1000;
            // printk("%f\n", actual);
            pid0.compute(actual);
            drive = pid0.get_duc();
            // printk("%f\n", drive);
            if(drive < 0)
                drive = 0;

            pwm_set_dt(&buck, PERIOD, PERIOD * drive);

        }
        else {
            pwm_set_dt(&buck, PERIOD, 0);
            printk("UPS In Error State: %d\n", ret);
		    k_sleep(K_SECONDS(1U));
        }

    }

	return 0;
}
