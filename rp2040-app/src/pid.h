#define MAX_DRIVE 1.0
#define INTEGRATOR_WINDUP_NEG (-100)
#define INTEGRATOR_WINDUP_POS (1000000)

class Pid {
    float target;
    float kp, ki, kd;
    float e_prev, e_sum;
    float t, t_prev;
    float drive;

protected:
    void update_target();

public:
    Pid(float target, float kp, float ki, float kd);
    float compute(float actual);
    float compute_boost(float actual);
    float get_i_term(void);
    void zero_i_term(void);

    void update_target(float t) {
        target = t;
    }

    float get_duc() {
        return drive;
    }
};

class Bump {
    float target;
    float bump_sum;
    float bump_amt;
    float drive;

public:
    Bump(float target, float bump_amt);
    float compute_boost(float actual);

    float get_duc() {
        return drive;
    }
};
