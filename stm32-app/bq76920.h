#define I2C_ADDR 0x18

#define SYS_STAT 0x00
#define CELLBAL1 0x01

#define SYS_CTRL1 0x04
#define SYS_CTRL2 0x05
#define PROTECT1 0x06

#define OV_TRIP 0x09
#define UV_TRIP 0x0A
#define CC_CFG 0x0B
#define ADCOFFSET 0x51
#define ADCGAIN1 0x50
#define ADCGAIN2 0x59

#define VC1_HI 0x0C
#define VC1_LO 0x0D

#define BAT_HI 0x2A
#define BAT_LO 0x2B

static int8_t adc_offset;
static uint16_t adc_gain;

void bq76920_write_reg(uint8_t reg, uint8_t val);
uint8_t bq76920_read_reg(uint8_t reg);
void bq76920_init(void);
void bq76920_output_enable(void);
void bq76920_set_uv(int voltage_mv);
void bq76920_set_ov(int voltage_mv);
void bq76920_clear_faults(void);
uint16_t bq76920_read_cell_v(uint8_t call);
void bq76920_shutdown(void);

void bq76920_write_reg(uint8_t reg, uint8_t val) {
    uint8_t dat[2];
    dat[0] = reg;
    dat[1] = val;

    i2c_transfer7(I2C1, I2C_ADDR, dat, 2, 0, 0);
}

uint8_t bq76920_read_reg(uint8_t reg) {
    uint8_t dat;

    i2c_transfer7(I2C1, I2C_ADDR, &reg, 1, &dat, 1);
    return dat;
}

void bq76920_set_uv(int voltage_mv) {
    uint16_t uv_trip = (((voltage_mv - adc_offset) * 1000 / adc_gain) >> 4)
        & 0x00FF;
    uv_trip += 1;

    bq76920_write_reg(UV_TRIP, uv_trip);
}

void bq76920_set_ov(int voltage_mv) {
    uint16_t ov_trip = (((voltage_mv - adc_offset) * 1000 / adc_gain) >> 4)
        & 0x00FF;

    bq76920_write_reg(OV_TRIP, ov_trip);
}

void bq76920_output_enable(void) {
    bq76920_write_reg(SYS_CTRL2, 0b00000011);
}

void bq76920_init() {
    adc_offset = (int8_t) bq76920_read_reg(ADCOFFSET);

    adc_gain = 365 + (((bq76920_read_reg(ADCGAIN1) & 0b00001100) << 1) |
      ((bq76920_read_reg(ADCGAIN2) & 0b11100000) >> 5));

    bq76920_set_uv(3000); // 3V / Cell
    bq76920_set_ov(4200); // 4.2V / Cell
    bq76920_write_reg(SYS_CTRL1, 0b00010000); // ADC_EN = 1

    // bq76920_write_reg(CELLBAL1, 0b00010111);
    bq76920_write_reg(CELLBAL1, 0x00);

    bq76920_write_reg(PROTECT1, 0b00000001);
    // PROTECT2 Left at default settings (8A Over Current)
}

void bq76920_clear_faults(void) {
    bq76920_write_reg(SYS_STAT, 0b00111111);
}

void bq76920_shutdown(void) {
    bq76920_write_reg(SYS_CTRL1, 0b00000001);
    bq76920_write_reg(SYS_CTRL1, 0b00000010);
}

uint16_t bq76920_read_cell_v(uint8_t cell) {
    uint16_t v = 0;
    cell = cell * 2;

    v = bq76920_read_reg(VC1_LO+cell);
    v |= (bq76920_read_reg(VC1_HI+cell) << 8);

    return (v*adc_gain / 1000) + adc_offset;
}
