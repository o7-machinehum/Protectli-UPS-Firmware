#define MAX_DRIVE 10000.0

class pid {
    float target;
    float kp, ki, kd;
    float e, e_prev;
    float t, t_prev;
    float drive;

protected:
    void update_target();
    
public:
    pid(float target, float kp, float kd, float ki);
    float compute(float actual);

    void update_target(float t) {
        target = t;
    }

    float get_duc() {
        return drive / MAX_DRIVE;
    }
};
