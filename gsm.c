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
int position = 0;   



//MAIN IS HERE




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
