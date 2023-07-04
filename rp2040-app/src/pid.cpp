#include "pid.h"

Pid::Pid(float target, float kp, float kd, float ki)
:target(target), kp(kp), ki(ki), kd(kd)
{

}

float Pid::compute(float actual) {
    e = target - actual;
    float de = e - e_prev;
    float dt = t - t_prev;

    drive = (e * kp) + ((de/dt) * kd); // Plus ki * integral
    if(drive > MAX_DRIVE)
        drive = MAX_DRIVE;

    e_prev = e;
    return drive / MAX_DRIVE;
}
