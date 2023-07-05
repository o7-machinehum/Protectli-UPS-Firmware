#include "adc.h"

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),


/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                 DT_SPEC_AND_COMMA)
};

Adc::Adc(float *scales)
: num_chan(ARRAY_SIZE(adc_channels))
{
    int err;
    v_scales = new float[ARRAY_SIZE(adc_channels)]();

    sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
    };

    /* Configure channels individually prior to sampling. */
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
        v_scales[i] = scales[i];
        if (!device_is_ready(adc_channels[i].dev)) {
            printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
        }

        err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0) {
            printk("Could not setup channel #%d (%d)\n", i, err);
        }
    }
}

bool Adc::check_chan(uint8_t ch) {
    if(ch > num_chan) {
        printk("Channel out of range!");
        return false;
    }
    return true;
}

int Adc::read(uint8_t i) {
    int err;
    int32_t val_mv;

    if(!check_chan(i))
        return -1;

    (void)adc_sequence_init_dt(&adc_channels[i], &sequence);
    err = adc_read(adc_channels[i].dev, &sequence);

    if (err < 0) {
        printk("Could not read (%d)\n", err);
        return -1;
    }

    if (adc_channels[0].channel_cfg.differential) {
        val_mv = (int32_t)((int16_t)buf);
    } else {
        val_mv = (int32_t)buf;
    }

    err = adc_raw_to_millivolts_dt(&adc_channels[i],
        &val_mv);

    return val_mv * v_scales[i];

}

int Adc::read_vout() {
    return read(0);
}

int Adc::read_vbat() {
    return read(1);
}

int Adc::read_iout() {
    return read(2);
}

int Adc::read_ibat() {
    return read(3);
}

