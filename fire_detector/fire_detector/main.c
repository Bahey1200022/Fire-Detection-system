/*
* fire_detector.c
*
* Created: 5/14/2024 2:38:30 PM
* Author : BEBO
*/

#define F_CPU 1000000UL
#include <stdio.h>

#define BIT_IS_SET(byte, bit) (byte & (1 << bit))
#define BIT_IS_CLEAR(byte, bit) (!(byte & (1 << bit)))
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#define LCD_DATA PORTB // port B is selected as LCD data port

#define en PD7 // enable signal is connected to port D pin 7
#define rw PD6 // read/write signal is connected to port D pin 6
#define rs PD5 // register select signal is connected to port D pin 5
void LCD_cmd(unsigned char cmd);
void init_LCD(void);
void LCD_write(unsigned char data);
void Cursor_pos(unsigned char x_pos, unsigned char y_pos);

ISR(PCINT1_vect){
	while( PINC & (1<<PINC5) )
	{
		PIND &= ~(1<<PIND3);
		LCD_cmd(0x01); // clear
		LCD_write('R');
		LCD_write('E');
		LCD_write('S');
		LCD_write('E');
		LCD_write('T');
		_delay_ms(500);
		LCD_cmd(0x01); // clear
	}
	
}



int main(void)
{
	DDRB = 0xFF; // set LCD data port as output
	
	DDRD = 0b11101000;// set LCD signals (RS, RW, E) as output - buzzer pin pd3
	
	PORTD |=(1<<PORTD0); //d0 input for smoke detector
	///////////////////////////////////////////////////////
	DDRC |=(1<<PINC5);  //interrupt setup pcint13
	
	PORTC |=(1<<PINC5);// interrupt pc5
	
	PCMSK1 |=(1<<PCINT13);
	PCICR |=(1<<PCIE1);
	
	sei();
	
	
	/////////////////////////////////////////////////
	init_LCD(); // initialize LCD
	_delay_ms(10); // delay of 100 Milli seconds

	LCD_cmd(0x0C); // display on, cursor off
	_delay_ms(10);
	LCD_cmd(0x01); // clear

	unsigned char ch[4]={' '};
	unsigned int tempVolt=0;
	while (1)
	{
		LCD_cmd(0x01);
		
		ADMUX = 0b01100011 ;
		ADCSRA = 0b10000011; ///pin3
		for (int j =0;j<4;j++){
			ch[j]=' ';///////////CLEARING CHAR ARRAY
		}
		////////////WRITING
		Cursor_pos(0,7);
		ADCSRA |= (1 << ADSC);					// start ADC conversion
		while(BIT_IS_SET(ADCSRA, ADSC)) {}
		tempVolt = ADCH;
		int temp = (int)((float)tempVolt / 255.0 * 500);
		itoa(temp,ch,10);  ////convert int to string

		for (int j=0;j<4;j++){
			if (ch[j]<'0'||ch[j]>'9')
			LCD_write(' ');
			else
			LCD_write(ch[j]);
		}
		_delay_ms(500);
		
		if(temp>50 ){
			LCD_write('H');
			LCD_write('E');
			LCD_write('A');
			LCD_write('T');

			PIND |=(1<<PIND3);
			_delay_ms(300);
			PIND &= ~(1<<PIND3);
		}
		if (PIND & (1<<PIND0)) {
			Cursor_pos(1,7);

			LCD_write('S');
			LCD_write('M');
			LCD_write('O');
			LCD_write('K');
			LCD_write('E');
			_delay_ms(300);

		}
		
		PIND &= ~(1<<PIND3);
		LCD_cmd(0x01); // clear


	}
}

void init_LCD(void)
{
	LCD_cmd(0x38); // initialization in 8bit mode of 16X2 LCD
	_delay_ms(1);

	LCD_cmd(0x01); // make clear LCD
	_delay_ms(1);

	LCD_cmd(0x02); // return home
	_delay_ms(1);

	LCD_cmd(0x06); // make increment in cursor
	_delay_ms(1);

	LCD_cmd(0x80); // "8" go to first line and "0" is for 0th position
	_delay_ms(1);

	return;
}

//*****sending command on LCD******//

void LCD_cmd(unsigned char cmd)
{
	LCD_DATA = cmd; // data lines are set to send command
	
	PORTD &= ~(1 << rs); // RS sets 0, for command data
	PORTD &= ~(1 << rw); // RW sets 0, to write data
	PORTD |= (1 << en); // make enable from high to low
	
	_delay_ms(100);
	PORTD &= ~(1 << en); // make enable low

	return;
}

//******write data on LCD******//

void LCD_write(unsigned char data)
{
	LCD_DATA = data; // data lines are set to send command
	PORTD |= (1 << rs); // RS sets 1, for command data
	PORTD &= ~(1 << rw); // RW sets 0, to write data
	PORTD |= (1 << en); // make enable from high to low

	_delay_ms(2);
	PORTD &= ~(1 << en); // make enable low

	return;
}



void Cursor_pos(unsigned char x_pos, unsigned char y_pos) //x awel aw tani row (0->1) el y column (0->15)
{
	uint8_t the_address=0;
	if (x_pos==0)
	the_address=0x80;
	else if(x_pos==1)
	the_address=0xC0;
	if(y_pos<16)
	the_address+=y_pos;
	LCD_cmd(the_address);
	
}