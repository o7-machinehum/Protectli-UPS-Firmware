class HwErrors
{
	struct {
		bool load_oc;
		bool batt_oc;
		bool batt_ov;
	} errorCodes;

    uint8_t error_code;
	uint8_t last_error_code;

      public:
	HwErrors();
	uint8_t check();
	uint8_t get() {
        return error_code;
    }
};
