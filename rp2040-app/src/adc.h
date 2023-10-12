#include <zephyr/drivers/adc.h>

class Adc
{
	uint8_t num_chan;
	struct adc_sequence sequence;

	bool check_chan(size_t ch);
	int read(size_t chan);

      public:
	Adc();

	int read_vout();
	int read_vbat();
	int read_iout();
	int read_ibat();
};
