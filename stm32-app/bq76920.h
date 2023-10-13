#include "stdint.h"

#define I2C_ADDR 0x18

#define SYS_STAT               0x00
#define SYS_STAT_OCD           (1 << 0)
#define SYS_STAT_SCD           (1 << 1)
#define SYS_STAT_OV            (1 << 2)
#define SYS_STAT_UV            (1 << 3)
#define SYS_STAT_OVRD_ALERT    (1 << 4)
#define SYS_STAT_DEVICE_XREADY (1 << 5)

#define CELLBAL1 0x01

#define SYS_CTRL1        0x04
#define SYS_CTRL1_SHUT_B (1 << 0)
#define SYS_CTRL1_SHUT_A (1 << 1)
#define SYS_CTRL1_ADC_EN (1 << 4)

#define SYS_CTRL2        0x05
#define SYS_CTRL2_CHG_ON (1 << 0)
#define SYS_CTRL2_DSG_ON (1 << 1)

#define PROTECT1        0x06
#define PROTECT1_SCD_T0 (1 << 0)

#define OV_TRIP   0x09
#define UV_TRIP   0x0A
#define CC_CFG    0x0B
#define ADCOFFSET 0x51
#define ADCGAIN1  0x50
#define ADCGAIN2  0x59

#define VC1_HI 0x0C
#define VC1_LO 0x0D

#define BAT_HI 0x2A
#define BAT_LO 0x2B

#define CELL0 0
#define CELL1 1
#define CELL2 2
#define CELL3 4

struct cells {
    uint16_t c0;
    uint16_t c1;
    uint16_t c2;
    uint16_t c3;
};

void bq76920_write_reg(uint8_t reg, uint8_t val);
uint8_t bq76920_read_reg(uint8_t reg);
void bq76920_init(void);
void bq76920_output_enable(void);
void bq76920_set_uv(int voltage_mv);
void bq76920_set_ov(int voltage_mv);
void bq76920_clear_faults(void);
uint16_t bq76920_read_cell_v(uint8_t call);
void bq76920_read_cells_v(struct cells *c);
void bq76920_shutdown(void);
uint8_t bq76920_balance_cells(struct cells *c);
