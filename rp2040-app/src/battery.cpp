#include "battery.h"

Battery::Battery(float target_voltage, float target_current,
		 float initial_value, float bump_amt)
	: target_i(target_current), bump_amt(bump_amt), drive(initial_value),
	  statechange_delay(TIMEOUT), state(CC)
{
	target_v = target_voltage * 1000;
}

float Battery::compute_drive(float v, float i)
{
	if (statechange_delay--) {
		statechange_delay = TIMEOUT;
		if (v > V_THRESH) {
			state = CV;
		} else {
			state = CC;
		}
	}

	if (state == CC) {
		if (target_i < i) {
			drive += bump_amt;
		} else {
			drive -= bump_amt;
		}
	} else if (state == CV) {
		if (target_v < v) {
			drive += bump_amt;
		} else {
			drive -= bump_amt;
		}
	}

	if (drive >= 0.85) {
		drive = 0.85;
	}
	if (drive <= 0.02) {
		drive = 0.02;
	}

	return drive;
}
