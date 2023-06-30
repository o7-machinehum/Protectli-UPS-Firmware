#include "pid.h"

class buck {
    float v_set;
    pid *pd;

public:
    buck(float v_set);
    void update_voltage(float v);

};
