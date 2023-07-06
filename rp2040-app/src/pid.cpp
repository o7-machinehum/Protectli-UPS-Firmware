#include <zephyr/kernel.h>
#include "pid.h"

Pid::Pid(float target, float kp, float ki, float kd)
:target(target), kp(kp), ki(ki), kd(kd)
{
    compute(0);

}

float Pid::compute(float actual) {
    uint32_t t = k_cycle_get_32();
    float e = target - actual;
    float dt = (t - t_prev) / 125e6;
    float de = e - e_prev;
    float integral = (de / 2) * dt;
    float derivatave = (e - e_prev) / dt;
    e_sum += e;

    if(e_sum < INTEGRATOR_WINDUP)
        e_sum = INTEGRATOR_WINDUP;

    drive = (e * kp) + (e_sum * ki) + (derivatave  * kd);

    if(drive > MAX_DRIVE)
        drive = MAX_DRIVE;

    e_prev = e;
    t_prev = t;
    return drive;
}
