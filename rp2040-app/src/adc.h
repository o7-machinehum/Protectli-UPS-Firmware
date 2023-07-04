#include <zephyr/drivers/adc.h>

class Adc {
    struct adc_sequence sequence;
    uint16_t buf;

public:
    Adc();
    int read(uint8_t chan);

};
