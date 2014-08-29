// ------------------------------------------------------------------
// --- lcd_io.c                                                   ---
// ---                                                            ---
// --- Library for controlling HD44780 via AVR's IO pins          ---
// ---                                                            ---
// ---                                 3.mar.2007, Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "hwdefs.h"

// ------------------------------------------------------------------
// --- defines ------------------------------------------------------
// ------------------------------------------------------------------

const uint8_t lcd_busw = 0x10;

#define LCD_RS_1 LCD_RS_PORT |= _BV(LCD_RS_PIN)
#define LCD_RS_0 LCD_RS_PORT &= ~_BV(LCD_RS_PIN)

#define LCD_RW_1 LCD_RW_PORT |= _BV(LCD_RW_PIN)
#define LCD_RW_0 LCD_RW_PORT &= ~_BV(LCD_RW_PIN)

#define LCD_E_1 LCD_E_PORT |= _BV(LCD_E_PIN)
#define LCD_E_0 LCD_E_PORT &= ~_BV(LCD_E_PIN)

// ------------------------------------------------------------------
// --- private procedures -------------------------------------------
// ------------------------------------------------------------------

void lcd_ddir(uint8_t out)
{
	if( out ) {
		DDR(LCD_D0_PORT) |= _BV(LCD_D0_BIT);
		DDR(LCD_D1_PORT) |= _BV(LCD_D1_BIT);
		DDR(LCD_D2_PORT) |= _BV(LCD_D2_BIT);
		DDR(LCD_D3_PORT) |= _BV(LCD_D3_BIT);
		DDR(LCD_D4_PORT) |= _BV(LCD_D4_BIT);
		DDR(LCD_D5_PORT) |= _BV(LCD_D5_BIT);
		DDR(LCD_D6_PORT) |= _BV(LCD_D6_BIT);
		DDR(LCD_D7_PORT) |= _BV(LCD_D7_BIT);
	} else {
		DDR(LCD_D0_PORT) &= ~_BV(LCD_D0_BIT);
		DDR(LCD_D1_PORT) &= ~_BV(LCD_D1_BIT);
		DDR(LCD_D2_PORT) &= ~_BV(LCD_D2_BIT);
		DDR(LCD_D3_PORT) &= ~_BV(LCD_D3_BIT);
		DDR(LCD_D4_PORT) &= ~_BV(LCD_D4_BIT);
		DDR(LCD_D5_PORT) &= ~_BV(LCD_D5_BIT);
		DDR(LCD_D6_PORT) &= ~_BV(LCD_D6_BIT);
		DDR(LCD_D7_PORT) &= ~_BV(LCD_D7_BIT);		
	}
}

void lcd_dout(uint8_t data)
{
	if( data & 0x01 ) LCD_D0_PORT |= _BV(LCD_D0_BIT); else LCD_D0_PORT &= ~_BV(LCD_D0_BIT);
	if( data & 0x02 ) LCD_D1_PORT |= _BV(LCD_D1_BIT); else LCD_D1_PORT &= ~_BV(LCD_D1_BIT);
	if( data & 0x04 ) LCD_D2_PORT |= _BV(LCD_D2_BIT); else LCD_D2_PORT &= ~_BV(LCD_D2_BIT);
	if( data & 0x08 ) LCD_D3_PORT |= _BV(LCD_D3_BIT); else LCD_D3_PORT &= ~_BV(LCD_D3_BIT);
	if( data & 0x10 ) LCD_D4_PORT |= _BV(LCD_D4_BIT); else LCD_D4_PORT &= ~_BV(LCD_D4_BIT);
	if( data & 0x20 ) LCD_D5_PORT |= _BV(LCD_D5_BIT); else LCD_D5_PORT &= ~_BV(LCD_D5_BIT);
	if( data & 0x40 ) LCD_D6_PORT |= _BV(LCD_D6_BIT); else LCD_D6_PORT &= ~_BV(LCD_D6_BIT);
	if( data & 0x80 ) LCD_D7_PORT |= _BV(LCD_D7_BIT); else LCD_D7_PORT &= ~_BV(LCD_D7_BIT);
}

uint8_t lcd_din(void)
{
	uint8_t r = 0;
	if( PIN(LCD_D0_PORT) & _BV(LCD_D0_BIT) ) r |= 0x01;
	if( PIN(LCD_D1_PORT) & _BV(LCD_D1_BIT) ) r |= 0x02;
	if( PIN(LCD_D2_PORT) & _BV(LCD_D2_BIT) ) r |= 0x04;
	if( PIN(LCD_D3_PORT) & _BV(LCD_D3_BIT) ) r |= 0x08;
	if( PIN(LCD_D4_PORT) & _BV(LCD_D4_BIT) ) r |= 0x10;
	if( PIN(LCD_D5_PORT) & _BV(LCD_D5_BIT) ) r |= 0x20;
	if( PIN(LCD_D6_PORT) & _BV(LCD_D6_BIT) ) r |= 0x40;
	if( PIN(LCD_D7_PORT) & _BV(LCD_D7_BIT) ) r |= 0x80;
	return r;
}

// writes instruction or data to LCD
void lcd_out(uint8_t data, uint8_t rs)
{
	if( rs ) {LCD_RS_1;} else {LCD_RS_0;}	// select instruction(0) or data(1)
	LCD_RW_0;				// RW low (write)
	lcd_ddir(1);
	lcd_dout(data);
	LCD_E_1;				// E high
	_delay_us(2);
	LCD_E_0;				// high to low on E to clock data
	_delay_us(2);
}

// reads instruction or data from LCD
uint8_t lcd_in(uint8_t rs)
{
	lcd_ddir(0);
	lcd_dout(0xff);	// pullups on
	if( rs ) {LCD_RS_1;} else {LCD_RS_0;}	// select instruction/data
	LCD_RW_1;		// RW high (read)
	LCD_E_1;
	_delay_us(2);
	uint8_t r = lcd_din();
	LCD_E_0;
	_delay_us(2);
	
	return r;
}

// reads busy flag
uint8_t lcd_busy(void)
{
	return( lcd_in(0) & _BV(7) );
}

// returns 0(false) on timeout and 1-20(true) on lcd available
uint8_t lcd_available(void)
{
	uint8_t i = 100;			// wait max 10ms
	while( lcd_busy() && i ) {
		_delay_us(100);
		i--;
	}
	return i;
}

// write command to lcd
uint8_t lcd_command(uint8_t data)
{
	if( lcd_available() ) {
		lcd_out(data, 0);
		return 1;
	}
	
	return 0;
}

// write data to lcd
uint8_t lcd_data(uint8_t data)
{
	if( lcd_available() ) {
		lcd_out(data, 1);
		return 1;
	}
	
	return 0;
}

void lcd_bl(uint8_t on)
{
	if( on ) {
		LCD_BL_PORT |= _BV(LCD_BL_BIT);
	} else {
		LCD_BL_PORT &= ~_BV(LCD_BL_BIT);
	}
}

void lcd_hwinit(void)
{
	LCD_E_0;	// idle E is low
	
	// configure the 3 LCD control pins as outputs
	DDR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
	DDR(LCD_RW_PORT) |= _BV(LCD_RW_PIN);
	DDR(LCD_E_PORT) |= _BV(LCD_E_PIN);
	
	// make LCD BL pin an output
	DDR(LCD_BL_PORT) |= _BV(LCD_BL_BIT);
	lcd_bl(0);	
}