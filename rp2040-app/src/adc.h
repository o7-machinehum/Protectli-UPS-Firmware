#include <zephyr/drivers/adc.h>

class Adc {
    struct adc_sequence sequence;
    uint16_t buf;
    uint8_t num_chan;
    float *v_scales;
    bool check_chan(uint8_t ch);

public:
    Adc(float *scales);
    int read(uint8_t chan);

    int read_vout();
    int read_vbat();
    int read_iout();
    int read_ibat();
};
