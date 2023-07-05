#include <zephyr/kernel.h>
#include "pid.h"

Pid::Pid(float target, float kp, float ki, float kd)
:target(target), kp(kp), ki(ki), kd(kd), integral(0)
{
    compute(target);
}

float Pid::compute(float actual) {
    t = k_uptime_get();
    float e = target - actual;
    float de = e - e_prev;
    float dt = t - t_prev;
    integral = (e*dt) + integral;

    drive = (e * kp) + ((de/dt) * kd) + (integral * ki);

    // printk("Drive: %f\t de: %f\t e: %f\t target: %f\t actual: %f\n ", drive, de, e, target, actual);
    if(drive > MAX_DRIVE)
        drive = MAX_DRIVE;

    e_prev = e;
    t_prev = t;
    return drive;
}
