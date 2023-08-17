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

<<<<<<< Updated upstream
static const struct pwm_dt_spec boost = PWM_DT_SPEC_GET(DT_ALIAS(pwm_boost));
static const struct pwm_dt_spec buck = PWM_DT_SPEC_GET(DT_ALIAS(pwm_buck));
=======
static const struct pwm_dt_spec pwm =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_0));

static const struct gpio_dt_spec pwm_en =
    GPIO_DT_SPEC_GET(DT_ALIAS(pwm_en), gpios);
>>>>>>> Stashed changes

static const struct gpio_dt_spec pwm_skip =
    GPIO_DT_SPEC_GET(DT_ALIAS(pwm_skip), gpios);

static const struct gpio_dt_spec vin_detect =
    GPIO_DT_SPEC_GET(DT_ALIAS(vin_detect), gpios);

void print_voltages(Adc adc) {
    printk("Vout: %d mV\t", adc.read_vout());
    printk("Vbat: %d mV\t", adc.read_vbat());
    printk("Iout: %d mA\t", adc.read_iout());
    printk("Ibat: %d mA\n", adc.read_ibat());
}

void buckboost(void)
{
    int ret;
    float drive;
    float actual;

	k_sleep(K_MSEC(500U));
	printk("~~~ Protectli UPS ~~~\n");

    HwErrors hw_errors;
    Pid pid(12.0, 0.03, 0.0001, 0.0);
    gpio_pin_configure_dt(&pwm_en, GPIO_OUTPUT);
    gpio_pin_configure_dt(&pwm_skip, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&vin_detect, GPIO_INPUT);

    Adc adc;
    print_voltages(adc);

    if (!device_is_ready(pwm.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       pwm.dev->name);
	}

<<<<<<< Updated upstream
    if (!device_is_ready(boost.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       boost.dev->name);
	}
=======
    // This works
    // if(gpio_pin_get_dt(&vin_detect))
	//     printk("Vin Detected\n");

    // while(1) {};
>>>>>>> Stashed changes

    // Main PID Loop
    bool first = true;
    int countdown = 1000;
    while(true) {
        if(!countdown--) {
            ret = hw_errors.errors();
            countdown = 1000;
            print_voltages(adc);
        }

        // Buck State
        if(!ret && !gpio_pin_get_dt(&vin_detect)) {
            if(first) {
                printk("Entering Buck State\n");
                first = false;
            }
            actual =  adc.read_vout();
            actual = actual / 1000;
<<<<<<< Updated upstream
            buck_pid.compute(actual);
            drive = buck_pid.get_duc();
=======
            pid.compute(actual);
            drive = pid.get_duc();
>>>>>>> Stashed changes
            // printk("%f\n", drive);
            if(drive < 0)
                drive = 0;

            gpio_pin_set_dt(&pwm_en, true);
            pwm_set_dt(&pwm, PERIOD, PERIOD*drive);

        }
<<<<<<< Updated upstream
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
            // printk("%f\n", drive);
            if(drive < 0)
                drive = 0;
=======
        // Boost State
        else if(!ret && gpio_pin_get_dt(&vin_detect)) {
            // if(first) {
            //     printk("Entering Boost State\n");
            //     first = false;
            // }
            // gpio_pin_set_dt(&pwm_en, true);
            // actual = adc.read_vbat();
            // actual = actual / 1000;
            // if(actual > 16) {
            //     gpio_pin_set_dt(&pwm_en, false);
            //     // pwm_set_dt(&boost, PERIOD, PERIOD);
            //     printk("Safety Trip\n");
            //     while(1) {};
            // }
            // pid.compute(actual);
            // drive = pid.get_duc();
            // // printk("%f\n", actual);
            // if(drive < 0)
            //     drive = 0;
>>>>>>> Stashed changes

            // pwm_set_dt(&pwm, PERIOD, PERIOD * drive);
        }
        else {
            if(first) {
                printk("UPS In Error State: %d\n", ret);
                first = false;
            }
            gpio_pin_set_dt(&pwm_en, false);
            pwm_set_dt(&pwm, PERIOD, 0);
		    k_sleep(K_SECONDS(1U));
        }

    }

}

K_THREAD_DEFINE(buckboost_id, STACKSIZE, buckboost, NULL, NULL, NULL,
		0, 0, 0);
