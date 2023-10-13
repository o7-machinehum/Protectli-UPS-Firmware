#include <zephyr/kernel.h>

#define TIMEOUT  10000
#define V_THRESH 16800

class Battery
{
	int target_v;   // mV
	float target_i; // mA
	float bump_amt;
	float drive;
	uint32_t statechange_delay;

	enum chrg_state {
		CC,
		CV
	} state;

      public:
	Battery(float target_voltage, float target_current, float initial_value,
		float bump_amt);
	float compute_drive(float v, float i);
};
