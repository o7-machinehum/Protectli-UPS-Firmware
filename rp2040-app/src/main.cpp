#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <stdio.h>

#include "hw_errors.h"
#include "pid.h"
#include "adc.h"
#include "battery.h"

extern "C" {
#include <msg.h>
}

#define PERIOD    PWM_NSEC(2500) // 500Khz
#define STACKSIZE 32768

#define MSG_SIZE 32
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct pwm_dt_spec pwm = PWM_DT_SPEC_GET(DT_ALIAS(pwm_0));

static const struct gpio_dt_spec pwm_en =
	GPIO_DT_SPEC_GET(DT_ALIAS(pwm_en), gpios);

static const struct gpio_dt_spec pwm_skip =
	GPIO_DT_SPEC_GET(DT_ALIAS(pwm_skip), gpios);

static const struct gpio_dt_spec vin_detect =
	GPIO_DT_SPEC_GET(DT_ALIAS(vin_detect), gpios);

static const struct gpio_dt_spec pack_boot =
	GPIO_DT_SPEC_GET(DT_ALIAS(pack_boot), gpios);

static const struct gpio_dt_spec gpio2 =
	GPIO_DT_SPEC_GET(DT_ALIAS(gpio_2), gpios);

static const struct gpio_dt_spec gpio3 =
	GPIO_DT_SPEC_GET(DT_ALIAS(gpio_3), gpios);

static const struct device *const uart_dev =
	DEVICE_DT_GET(DT_CHOSEN(zephyr_mcu_uart));

void print_voltages(Adc adc, float drive)
{
	printk("Vout : %d mV\t", adc.get_vout());
	printk("Vbat : %d mV\t", adc.get_vbat());
	printk("Iout : %d mA\t", adc.get_iout());
	printk("Ibat : %d mA\t", adc.get_ibat());
	printk("Drve : %f\n", drive);
}

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

void print_uart(char *buf, uint8_t msg_len)
{
	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

enum State {
	NONE,
	BUCK,
	BOOST,
	ERROR
} state;

void buckboost(void)
{
	int ret = 0;
	float drive;
	float vout = 0, vbat = 0;
	char uartbuf[64] = {};
	Msg msg = {.vout = 0, .vbat = 0};

	printk("~~~ Protectli UPS ~~~\n");

	HwErrors hw_errors;
	Pid buck_pid(12.0, 0.03, 0.0001, 0.0);

	Battery battery = Battery().setVoltage(16.7).setCurrent(800.0);

	gpio_pin_configure_dt(&pwm_en, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&pack_boot, GPIO_OUTPUT);
	gpio_pin_configure_dt(&pwm_skip, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&vin_detect, GPIO_INPUT);

	gpio_pin_configure_dt(&gpio2, GPIO_DISCONNECTED);
	gpio_pin_configure_dt(&gpio3, GPIO_DISCONNECTED);

	k_sleep(K_MSEC(1000));

	Adc adc;
	print_voltages(adc, 0.00);

	if (!device_is_ready(pwm.dev)) {
		printk("Error: PWM device %s is not ready\n", pwm.dev->name);
	}

	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
	}

	/* configure interrupt and callback to receive data */
	uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	uart_irq_rx_enable(uart_dev);

	state = NONE;
	int countdown = 1000;

	while (true) {
		adc.read_all();
		if (!countdown--) {
			hw_errors.check();
			countdown = 1000;
			print_voltages(adc, drive);

			msg.vout = adc.get_vout();
			msg.vbat = adc.get_vbat();
			msg.iout = adc.get_iout();
			msg.ibat = adc.get_ibat();
			msg.gas = (adc.get_vbat() - 12000) / 48;
			ret = msg_cobs_encode(msg, uartbuf);
			print_uart(uartbuf, ret);
		}

		// Buck State
		if (!hw_errors.get() && !gpio_pin_get_dt(&vin_detect)) {
			if (state != BUCK) {
				state = BUCK;
				printk("Entering Buck State\n");
				msg.state = MSG_STATE_DISCHARGING;
			}
			vout = adc.get_vout();
			vout = vout / 1000;
			buck_pid.compute(vout);
			drive = buck_pid.get_duc();

			gpio_pin_set_dt(&pwm_en, true);
			pwm_set_dt(&pwm, PERIOD, PERIOD * drive);
		}
		// Boost State (Charging)
		else if (!hw_errors.get() && gpio_pin_get_dt(&vin_detect)) {
			if (state != BOOST) {
				state = BOOST;
				printk("Entering Boost State\n");
				msg.state = MSG_STATE_CHARGING;
#if defined(CONFIG_FORCE_PACK)
				gpio_pin_set_dt(&pack_boot, true);
				k_sleep(K_MSEC(100U));
				gpio_pin_set_dt(&pack_boot, false);
#endif
			}

			drive = battery.compute_drive(adc.get_vbat(),
						      adc.get_ibat());
			pwm_set_dt(&pwm, PERIOD, PERIOD * drive);
			gpio_pin_set_dt(&pwm_en, true);
		}
		// Error State
		else {
			if (state != ERROR) {
				state = ERROR;
				msg.state = MSG_STATE_ERROR;
				printk("UPS In Error State: %d\n",
				       hw_errors.get());
			}
			gpio_pin_set_dt(&pwm_en, false);
			pwm_set_dt(&pwm, PERIOD, 0);
			k_sleep(K_SECONDS(5U));
			state = NONE;
		}
	}
}

K_THREAD_DEFINE(buckboost_id, STACKSIZE, buckboost, NULL, NULL, NULL, 0, 0, 0);
