#include <string.h>
#include "battery.h"

Battery::Battery(float target_voltage, float target_current,
		 float initial_value, float bump_amt)
	: target_i(target_current), bump_amt(bump_amt), drive(initial_value)
{
	target_v = target_voltage * 1000;
}

float Battery::compute_drive(float v, float i)
{

	if(i < target_i && v < target_v)
		drive -= bump_amt; // This adds current
	else
		drive += bump_amt;

	if (drive >= 0.85) {
		drive = 0.85;
	}
	if (drive <= 0.02) {
		drive = 0.02;
	}

	return drive;
}
