#include <zephyr/kernel.h>


class Battery
{
	int target_v;   // mV
	float target_i; // mA
	float bump_amt;
	float drive;

      public:
	Battery();
	float compute_drive(float v, float i);

	Battery &setVoltage(float v) {
		target_v = v * 1000;
		return *this;
	}

	Battery &setCurrent(float i) {
		target_i = i;
		return *this;
	}
};
