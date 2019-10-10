#include <avr/io.h>
#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>



//uart
#define UART_BAUD_RATE 9600
#define BAUD_PRESCALE  ((F_CPU/(16UL*UART_BAUD_RATE))-1)
void UART_Init(void);
void UART_Transmit(char data);
char UART_Receive(void);
void UART_SendString(char *str);

//lcd
//LCD
#define LCD_Dir DDRB					/* Define LCD data port direction */
#define LCD_Port PORTB					/* Define LCD data port */
#define RS PC1							/* Define Register Select (data reg./command reg.) signal pin */
#define EN PC0 							/* Define Enable signal pin */
void LCD_Command( unsigned volatile char cmnd );
void LCD_Char( unsigned volatile char data );
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear(void);

//gsm




int main(void)
{
	//char c;
	UART_Init();
	LCD_Init();
	UART_SendString("\n\t Okay okay test");
	LCD_String_xy(0,0,"I am okay");	
	while(1)
	{

	}
}






//uart functions

void UART_Init(void)
{
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0);   //enable receiver and transmitter
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);   //8bit data, 1 stop bit, no parity
	UBRR0H = (unsigned char) (BAUD_PRESCALE >> 8);  //shift the baud rate to the two registers
	UBRR0L = BAUD_PRESCALE;
}

void UART_Transmit(char data)
{
	while(!(UCSR0A & (1 << UDRE0)));   //wait for current transmission to end
	UDR0 = data;             //copy the data to the UDR0 register
}

char UART_Receive(void)
{
	while((UCSR0A & (1 << RXC0)) == 0);   //wait
	return (UDR0);
	
}

void UART_SendString(char *str)
{
	int i = 0;
	for (i = 0; str[i] != '\0' ; i++)
	{
		UART_Transmit(str[i]);
	}
}

//end of uart functions


//lcd functions
void LCD_Command( unsigned volatile char cmnd )
{

	LCD_Port = (LCD_Port & 0xF0) | ((cmnd & 0xF0) >> 4); /* sending upper nibble */
	PORTC &= ~ (1<<RS);				/* RS=0, command reg. */
	PORTC |= (1<<EN);				/* Enable pulse */
	_delay_us(1);
	PORTC &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0xF0) | (cmnd & 0x0F);  /* sending lower nibble */
	PORTC |= (1<<EN);
	_delay_us(1);
	PORTC &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned volatile char data )
{
	LCD_Port = (LCD_Port & 0xF0) | ((data & 0xF0) >> 4); /* sending upper nibble */
	PORTC |= (1<<RS);				/* RS=1, data reg. */
	PORTC |= (1<<EN);
	_delay_us(1);
	PORTC &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0xF0) | (data & 0x0F); /* sending lower nibble */
	PORTC |= (1<<EN);
	_delay_us(1);
	PORTC &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Init (void)					/* LCD Initialize function */
{
	LCD_Dir |= 0x0F;						/* Make LCD command port direction as o/p */
	DDRC |= (1 << DDC0) | (1 << DDC1);
	_delay_ms(20);						/* LCD Power ON delay always >15ms */
	
	LCD_Command(0x33);
	LCD_Command(0x32);		    		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              	/* Use 2 line and initialize 5*7 matrix in (4-bit mode)*/
	LCD_Command(0x0c);              	/* Display on cursor off*/
	LCD_Command(0x06);              	/* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              	/* Clear display screen*/
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}


void LCD_String (char *str)				/* Send string to LCD function */
{
	int i;
	
	for(i=0; str[i]!=0 ;i++)				/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
	
}


void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);		/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);		/* Command of first row and required position<16 */
	LCD_String(str);					/* Call LCD string function */
}

void LCD_Clear(void)
{
	LCD_Command (0x01);					/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);					/* Cursor 1st row 0th position */
}

//END OF LCD FUNCTIONS
