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
	float integral = (de / 2) * dt;
	float derivatave = (e - e_prev) / dt;
	e_sum += e;

	if (e_sum < INTEGRATOR_WINDUP_NEG) {
		e_sum = INTEGRATOR_WINDUP_NEG;
	}

	drive = (e * kp) + (e_sum * ki) + (derivatave * kd);

	if (drive > MAX_DRIVE) {
		drive = MAX_DRIVE;
	}

	e_prev = e;
	t_prev = t;
	return drive;
}

float Pid::compute_boost(float actual)
{
	uint32_t t = k_cycle_get_32();
	float e = target - actual;
	float dt = (t - t_prev) / 125e6;
	float de = e - e_prev;
	float integral = (de / 2) * dt;
	float derivatave = (e - e_prev) / dt;
	e_sum += e / 10;

	// printk("%f\n", e_sum);

	if (e_sum < INTEGRATOR_WINDUP_NEG) {
		e_sum = INTEGRATOR_WINDUP_NEG;
	} else if (e_sum > INTEGRATOR_WINDUP_POS) {
		e_sum = INTEGRATOR_WINDUP_POS;
	}

	drive = (e * kp) + (e_sum * ki) + (derivatave * kd);

	if (drive > MAX_DRIVE) {
		drive = MAX_DRIVE;
	}

	e_prev = e;
	t_prev = t;
	return drive;
}

Bump::Bump(float target, float bump_amt) : target(target), bump_sum(0), bump_amt(bump_amt), drive(0)
{
}

float Bump::compute_boost(float actual)
{
	bump_sum += (target - actual);
	drive = bump_sum * bump_amt;

	if (drive < 0) {
		drive = 0;
	}
	if (drive > 1) {
		drive = 1;
	}

	return drive;
}
