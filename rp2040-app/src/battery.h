#include <zephyr/kernel.h>


class Battery
{
	int target_v;   // mV
	float target_i; // mA
	float bump_amt;
	float drive;

      public:
	Battery(float target_voltage, float target_current, float initial_value,
		float bump_amt, bool dynamic_current);
	float compute_drive(float v, float i);
};
