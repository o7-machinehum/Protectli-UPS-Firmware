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

static const struct pwm_dt_spec pwm =
    PWM_DT_SPEC_GET(DT_ALIAS(pwm_0));

static const struct gpio_dt_spec pwm_en =
    GPIO_DT_SPEC_GET(DT_ALIAS(pwm_en), gpios);

static const struct gpio_dt_spec pwm_skip =
    GPIO_DT_SPEC_GET(DT_ALIAS(pwm_skip), gpios);

static const struct gpio_dt_spec vin_detect =
    GPIO_DT_SPEC_GET(DT_ALIAS(vin_detect), gpios);

void print_voltages(Adc adc, float drive) {
    printk("Vout: %d mV\t", adc.read_vout());
    printk("Vbat: %d mV\t", adc.read_vbat());
    printk("Iout: %d mA\t", adc.read_iout());
    printk("Ibat: %d mA\t", adc.read_ibat());
    printk("Drve: %f\n", drive);
}

void buckboost(void)
{
    int ret = 0;
    float drive;
    float vout = 0, vbat = 0;

	k_sleep(K_MSEC(1000));
	printk("~~~ Protectli UPS ~~~\n");

    HwErrors hw_errors;
    Pid buck_pid(12.0, 0.03, 0.0001, 0.0);
    Pid boost_pid(500.0, 0.003, 0.0, 0.0); // 500mA current target
    gpio_pin_configure_dt(&pwm_en, GPIO_OUTPUT);
    gpio_pin_configure_dt(&pwm_skip, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&vin_detect, GPIO_INPUT);

    Adc adc;
    print_voltages(adc, 0.00);

    if (!device_is_ready(pwm.dev)) {
		printk("Error: PWM device %s is not ready\n",
		       pwm.dev->name);
	}

    // This works
    // if(gpio_pin_get_dt(&vin_detect))
	//     printk("Vin Detected\n");

    // while(1) {};

    // Main PID Loop
    bool first = true;
    int countdown = 1000;
    while(true) {
        if(!countdown--) {
            ret = hw_errors.errors();
            countdown = 1000;
            print_voltages(adc, drive);
        }

        // Buck State
        if(!ret && !gpio_pin_get_dt(&vin_detect)) {
            if(first) {
                printk("Entering Buck State\n");
                first = false;
            }
            vout =  adc.read_vout();
            vout = vout / 1000;
            buck_pid.compute(vout);
            drive = buck_pid.get_duc();
            // printk("%f\n", drive);
            // if(drive < 0)
            //     drive = 0;

            // if(drive >= 0.99)
            //     drive = 0.98;

            gpio_pin_set_dt(&pwm_en, true);
            pwm_set_dt(&pwm, PERIOD, PERIOD*drive);

        }
        // Boost State (Charging)
        else if(!ret && gpio_pin_get_dt(&vin_detect)) {
            int target = 16000;
            if(first) {
                printk("Entering Boost State\n");
                first = false;
            }

            // Read vbat in mA
            vbat = adc.read_vbat();
            if(vbat > 20000.0) {
                gpio_pin_set_dt(&pwm_en, false);
                printk("Safety Trip\n");
                while(1) {};
            }

            boost_pid.compute(adc.read_ibat());
            // drive = adc.read_vout() / vbat;
            // drive = boost_pid.get_duc();
            drive = 0.75;
            // vbat / target;

            // printk("%f\n", drive);
            if(drive < 0)
                drive = 0;
            pwm_set_dt(&pwm, PERIOD, PERIOD * drive);
            gpio_pin_set_dt(&pwm_en, true);
        }
        // Error State
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
