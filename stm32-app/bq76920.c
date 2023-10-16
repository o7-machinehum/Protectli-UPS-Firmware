#include "bq76920.h"
#include "bq76920_config.h"
#include "stdint.h"
#include <libopencm3/stm32/i2c.h>

static int8_t adc_offset;
static uint16_t adc_gain;

void bq76920_write_reg(uint8_t reg, uint8_t val)
{
	uint8_t dat[2];
	dat[0] = reg;
	dat[1] = val;

	i2c_transfer7(I2C1, I2C_ADDR, dat, 2, 0, 0);
}

uint8_t bq76920_read_reg(uint8_t reg)
{
	uint8_t dat;

	i2c_transfer7(I2C1, I2C_ADDR, &reg, 1, &dat, 1);
	return dat;
}

void bq76920_set_uv(int voltage_mv)
{
	uint16_t uv_trip =
		(((voltage_mv - adc_offset) * 1000 / adc_gain) >> 4) & 0x00FF;
	uv_trip += 1;

	bq76920_write_reg(UV_TRIP, uv_trip);
}

void bq76920_set_ov(int voltage_mv)
{
	uint16_t ov_trip =
		(((voltage_mv - adc_offset) * 1000 / adc_gain) >> 4) & 0x00FF;

	bq76920_write_reg(OV_TRIP, ov_trip);
}

void bq76920_output_enable(void)
{

	bq76920_write_reg(SYS_CTRL2, (SYS_CTRL2_CHG_ON | SYS_CTRL2_DSG_ON));
}

void bq76920_init()
{
	adc_offset = (int8_t)bq76920_read_reg(ADCOFFSET);

	adc_gain = 365 + (((bq76920_read_reg(ADCGAIN1) & 0b00001100) << 1) |
			  ((bq76920_read_reg(ADCGAIN2) & 0b11100000) >> 5));

	bq76920_set_uv(UV_MV);                           // V / Cell
	bq76920_set_ov(OV_MV);                           // V / Cell
	bq76920_write_reg(SYS_CTRL1, SYS_CTRL1_ADC_EN); // ADC_EN = 1

	bq76920_write_reg(CELLBAL1, 0x00);
	bq76920_write_reg(PROTECT1, PROTECT1_SCD_T1);
	// PROTECT2 Left at default settings (8A Over Current)
}



void bq76920_clear_faults(void)
{
	bq76920_write_reg(SYS_STAT, (SYS_STAT_OCD | SYS_STAT_SCD | SYS_STAT_OV |
				     SYS_STAT_UV | SYS_STAT_OVRD_ALERT |
				     SYS_STAT_DEVICE_XREADY));
}

void bq76920_shutdown(void)
{
	bq76920_write_reg(SYS_CTRL1, SYS_CTRL1_SHUT_B);
	bq76920_write_reg(SYS_CTRL1, SYS_CTRL1_SHUT_A);
}

uint16_t bq76920_read_cell_v(uint8_t cell)
{
	uint16_t v = 0;
	cell = cell * 2;

	v = bq76920_read_reg(VC1_LO + cell);
	v |= (bq76920_read_reg(VC1_HI + cell) << 8);

	return (v * adc_gain / 1000) + adc_offset;
}

void bq76920_read_cells_v(struct cells *c)
{
	c->c0 = bq76920_read_cell_v(CELL0);
	c->c1 = bq76920_read_cell_v(CELL1);
	c->c2 = bq76920_read_cell_v(CELL2);
	c->c3 = bq76920_read_cell_v(CELL3);
}

uint8_t bq76920_balance_cells(struct cells *c) {
	uint8_t bal = 0;
    uint16_t lowest_cell_v;

	if(c->c0 > BAL_V_MV)
		bal = 1 << CELL0;
	if(c->c1 > BAL_V_MV)
		bal |= 1 << CELL1;
	if(c->c2 > BAL_V_MV)
		bal |= 1 << CELL2;
	if(c->c3 > BAL_V_MV)
		bal |= 1 << CELL3;

	bq76920_write_reg(CELLBAL1, bal);
	return bal;
}
