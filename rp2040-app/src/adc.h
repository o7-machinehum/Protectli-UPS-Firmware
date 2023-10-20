#include <zephyr/drivers/adc.h>

class Adc
{
	uint8_t num_chan;
	struct adc_sequence sequence;
	int vout, vbat, iout, ibat;

	bool check_chan(size_t ch);
	int read(size_t chan);

      public:
	Adc();

	int sample_vout();
	int sample_vbat();
	int sample_iout();
	int sample_ibat();

	void read_all();
	int get_vout()
	{
		return vout;
	};
	int get_vbat()
	{
		return vbat;
	};
	int get_iout()
	{
		return iout;
	};
	int get_ibat()
	{
		return ibat;
	};
};
