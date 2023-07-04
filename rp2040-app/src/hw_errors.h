class HwErrors{
    struct {
        bool load_oc;
        bool batt_oc;
        bool batt_ov;
        bool batt_ot;
    } errorCodes;

public:
    HwErrors();
    uint8_t errors();
};
