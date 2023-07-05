#define MAX_DRIVE 1.0

class Pid {
    float target;
    float kp, ki, kd;
    float e_prev, integral;
    float t, t_prev;
    float drive;

protected:
    void update_target();

public:
    Pid(float target, float kp, float ki, float kd);
    float compute(float actual);

    void update_target(float t) {
        target = t;
    }

    float get_duc() {
        return drive / MAX_DRIVE;
    }
};
