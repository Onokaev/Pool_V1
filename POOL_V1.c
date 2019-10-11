#include <avr/io.h>
#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



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
void GSM_Begin(void);
void GSM_Response(void);
void GSM_Response_Display(void);
void GSM_Msg_Read(int x);
bool GSM_Wait_For_Msg(void);
void GSM_Msg_Display(void);
void GSM_Msg_Delete(unsigned int y);
void GSM_Delete_All_Msgs(void);

char buffer[60];                  //array to store messages and responses
char status_flag = 0;             //flag for checking new messages
volatile int buffer_pointer;
char Mobile_No[10];                //buffer to hold mobile number of received mmessage
char message_received[60];        //store messages
int position = 0;                 //save location of current message









int main(void)
{
	buffer_pointer = 0;
	int is_msg_arrived;
	memset(message_received, 0, 60);
	//char c;
	UART_Init();
	LCD_Init();
	sei();
	
	LCD_String_xy(0,0,"GSM Initializing");
	_delay_ms(3000);
	LCD_Clear();
	
	LCD_String_xy(1,0,"AT");
	GSM_Begin();               //this starts the GSM with AT
	LCD_Clear();
	
		while (1){
			
			/*check if any new message received */
			if(status_flag==1){
				is_msg_arrived = GSM_Wait_For_Msg();        /*check for message arrival*/
				if(is_msg_arrived== true)
				{
					LCD_Clear();
					LCD_String_xy(1,0,"new message");         /* new message arrived */
					_delay_ms(1000);
					LCD_Clear();
					GSM_Msg_Read(position);                  /* read arrived message */
					_delay_ms(3000);
					
					
					/*check if received message is "call me" */
// 					if(strstr( message_received,"call me")){
// 						
// 						GSM_Calling(Mobile_no);              /* call sender of message */
// 						LCD_Clear();
// 						LCD_String_xy(1,0,"Calling...");
// 						_delay_ms(15000);
// 						GSM_HangCall(); /* hang call */
// 						LCD_String_xy(1,0,"Hang Call");
// 						_delay_ms(1000);
// 					}
					
					LCD_Clear();
					GSM_Msg_Delete(position); /* to save SIM memory delete current message */
					LCD_String_xy(1,0,"Clear msg");
					GSM_Response();
					_delay_ms(1000);
					
				}
				
				
				is_msg_arrived=0;
				status_flag=0;
				LCD_Clear();
			}
			LCD_String_xy(1,0,"waiting for msg");
			memset(Mobile_No, 0, 14);
			memset(message_received, 0, 60);
			
			
		}
	
}






//gsm functions
void GSM_Begin(void)
{
	while(1)
	{
		LCD_Command(0xC0);    //set cursor to the second line
		UART_SendString("AT\r");           //send AT to check if module is ready or not
		_delay_ms(500);
		if(strstr(buffer, "OK"))
		{
			GSM_Response();
			memset(buffer, 0, 160);
			break;
		}   
		else
		{
			LCD_String("Error occurred!");
		}  
	}
	_delay_ms(1000);
	
	LCD_Clear();
	LCD_String_xy(0,0,"Text Mode");
	LCD_Command(0xC0);
	UART_SendString("AT+CMGF=1\r");              //set GSM in text mode
	GSM_Response();
	_delay_ms(1000);
	
}

void GSM_Response(void)
{
	unsigned int timeout = 0;
	int CRLF_Found = 0;
	char CRLF_buff[2];
	int Response_Length = 0;
	
	while(1)
	{
		if(timeout >= 60000)
		return;
		
		Response_Length = strlen(buffer);
		
		if(Response_Length)
		{
			_delay_ms(2);
			timeout++;
			
			if(Response_Length == strlen(buffer))
			{
				for (int i = 0; i < Response_Length ; i++)
				{
					memmove(CRLF_buff, CRLF_buff + 1, 1);
					CRLF_buff[1] = buffer[i];
					
					if(strncmp(CRLF_buff, "\r\n", 2))
					{
						if (CRLF_Found ++ == 2)
						{
							GSM_Response_Display();
							return;
						}
					}
				}
				CRLF_Found = 0;
			}
		}
		_delay_ms(1);
		timeout++;
	}
	status_flag = 0;
}

void GSM_Response_Display(void)
{
	buffer_pointer = 0;
	int lcd_pointer = 0;
	
	while(1)
	{
		//search for /r/n in string
		
		if(buffer[buffer_pointer] == '\r' || buffer[buffer_pointer] == '\n')
		{
			buffer_pointer++;
		}
		else
		{
			break;
		}
	}
	
	LCD_Command(0xC0);
	
	while(buffer[buffer_pointer] != '\r')
	{
		LCD_Char(buffer[buffer_pointer]);
		buffer_pointer++;
		lcd_pointer++;
		
		if(lcd_pointer == 15)        //lcd has reached the end 
		{
			LCD_Command(0x80);       //force cursor to first line first position
		}
	}
	buffer_pointer = 0;
	memset(buffer, 0, strlen(buffer));    //put zeros where the buffer is. clear the buffer memory
	
}


void GSM_Msg_Read(int position)
{
	char read_cmd[10];
	sprintf(read_cmd, "AT+CMGR=%d\r", position);
	UART_SendString(read_cmd);
    GSM_Msg_Display();
}

void GSM_Msg_Display(void)
{
	_delay_ms(500);
	if(!(strstr(buffer,"+CMGR")))	/*check for +CMGR response */
	{
		LCD_String_xy(1,0,"No message");
	}
	else
	{
		buffer_pointer = 0;
		
		while(1)
		{
			/*wait till \r\n not over*/

			if(buffer[buffer_pointer] =='\r' || buffer[buffer_pointer] == 'n')
			{
				buffer_pointer++;
			}
			else
			break;
		}
		
		/* search for first comma ',' to get mobile no.*/
		while(buffer[buffer_pointer]!=',')
		{
			buffer_pointer++;
		}
		buffer_pointer = buffer_pointer + 2;

		/* extract mobile no. of message sender */
		for(int i = 0 ; i <= 12 ; i++)                         //confirm length of phone number for safaricom
		{
			Mobile_No[i] = buffer[buffer_pointer];
			buffer_pointer++;
		}
		
		do
		{
			buffer_pointer++;
		}while(buffer[buffer_pointer-1]!= '\n');
		
		LCD_Command(0xC0);
		int i=0;

		/* display and save message */
		while(buffer[buffer_pointer]!= '\r' && i < 31)
		{
			LCD_Char(buffer[buffer_pointer]);
			message_received[i] = buffer[buffer_pointer];
			
			buffer_pointer++;
			i++;
			
			if(i==16)
			LCD_Command(0x80); /* display on 1st line */
		}
		
		buffer_pointer = 0;
		memset(buffer , 0 , strlen(buffer));
	}
	status_flag = 0;
}

bool GSM_Wait_For_Msg(void)
{
	char msg_location[4];
	int i;
	
	_delay_ms(500);
	buffer_pointer = 0;
	
	while(1)
	{
		//eliminate \r\n which is star of string
		
		if(buffer[buffer_pointer] == '\r' || buffer[buffer_pointer] == '\n')
		{
			buffer_pointer++;
		}
		else
		{
			break;
		}
		
	}
	
	
	if(strstr(buffer, "CMTI:"))
	{
		while(buffer[buffer_pointer] != ',')
		{
			buffer_pointer++;
		}
		
		buffer_pointer++;
		
		i = 0;
		
		while(buffer[buffer_pointer] != '\r')
		{
			msg_location[i] = buffer[buffer_pointer];
			buffer_pointer++;
			i++;
		}
		
		//convert string of position to integer
		
		position = atoi(msg_location);
		
		memset(buffer, 0, strlen(buffer));
		buffer_pointer = 0;
		
		return true;
	}
	
	else
	{
		return false;
	}
	
}


void GSM_Msg_Delete(unsigned int position)
{
	buffer_pointer = 0;
	char delete_cmd[20];
	
	//delete message at specified position
	
	sprintf(delete_cmd, "AT+CMGD=%d\r", position);
	UART_SendString(delete_cmd);
}



void GSM_Delete_All_Msgs(void)
{
	//count messages upto 40, then delete all. do this later
	UART_SendString("AT+CMGDA=\"DEL ALL\"\r");       //delete all messages
}

//end of GSM functions






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
	
	for(i=0; str[i]!= '\0' ;i++)				/* Send each char of string till the NULL */
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
