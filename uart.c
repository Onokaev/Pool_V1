#define UART_BAUD_RATE 9600
#define BAUD_PRESCALE  ((F_CPU/(16UL*UART_BAUD_RATE))-1)
void UART_Init(void);
void UART_Transmit(char data);
char UART_Receive(void);
void UART_SendString(char *str);


//main should be here



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
