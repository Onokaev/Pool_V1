//stepper motor
void Stepper_Motor(void);


//main here


void Stepper_Motor(void)
{
	while(1)
	{
		//rotate stepper motor with full-step sequence
		for (int i = 0; i < 12; i++)
		{
			PORTD |= 0x0C;
			_delay_ms(1100);
			PORTD |= 0x06;
			_delay_ms(1100);
			PORTD |= 0x03;
			_delay_ms(1100);
			PORTD |= 0x09;
			_delay_ms(1100);
		}
		_delay_ms(1100);             //this depends on the amount of time taken to eject all balls. subject to change
		
		//rotate in the opposite direction
		for(int i = 0; i < 12 ; i++)
		{
			PORTD |= 0x09;
			_delay_ms(1100);
			PORTD |= 0x03;
			_delay_ms(1100);
			PORTD |= 0x06;
			_delay_ms(1100);
			PORTD |= 0x0C;
			_delay_ms(1100);
			
		}
		PORTD |= 0x09;
		_delay_ms(1100);
		
		break;
	}
}
