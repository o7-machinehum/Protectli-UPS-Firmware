#include <zephyr/kernel.h>
#include "pid.h"

Pid::Pid(float target, float kp, float ki, float kd)
	: target(target), kp(kp), ki(ki), kd(kd), e_sum(0)
{
	compute(target);
}

float Pid::get_i_term(void)
{
	return e_sum;
}

void Pid::zero_i_term(void)
{
	e_sum = 0;
}

float Pid::compute(float actual)
{
	uint32_t t = k_cycle_get_32();
	float e = target - actual;
	float dt = (t - t_prev) / 125e6;
	float de = e - e_prev;
	float derivatave = (e - e_prev) / dt;
	e_sum += e;

	if (e_sum < INTEGRATOR_WINDUP_NEG) {
		e_sum = INTEGRATOR_WINDUP_NEG;
	}

	drive = (e * kp) + (e_sum * ki) + (derivatave * kd);

	if (drive >= 0.90) {
		drive = 0.90;
	}

	if (drive <= 0.02) {
		drive = 0.02;
	}

	e_prev = e;
	t_prev = t;
	return drive;
}
