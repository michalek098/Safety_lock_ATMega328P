#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#define BAUD 9600
#define BAUDRATE F_CPU/16/BAUD-1

void serwo_init(void)
{
	DDRB |= (1 << PB1); // wyjscie
	TCCR1A = (1 << WGM11) | (1 << COM1A1); // tryb fast pwm zbocze narastaj¹ce
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); //preskaler
	ICR1 = 20000;
}
void servo_left(void)
{	OCR1A = 1000;
	_delay_ms(1500);
}
void servo_right(void)
{
	OCR1A = 2000;
	_delay_ms(1500);
}

void init_led(void)
{
	//czerwony led
	DDRD |= (1<<PD6); //wyjœcie PWM
	TCCR0A |= (1<<COM0A1); //zbocze narastaj¹ce
	TCCR0A |= (1<<WGM00) | (1<<WGM01); //0xFF = TOP FAST PWM
	TCCR0B |= (1<<CS00); //16000000/(256*preskaler)=F_PWM
	//zielony led
	DDRB |= (1<<PB3); //wyjœcie PWM
}
void zielony_led(void)
{
	PORTB |= (1<<PB3);
}
void alarm(void)
{
	for(uint8_t i = 0; i < 255; i++)
	{
		OCR0A = i;
		_delay_ms(5);
	}
	
	for(uint8_t i = 255; i > 0; i--)
	{
		OCR0A = i;
		_delay_ms(5);
	}
}
void alarm_stop(void)
{
	OCR0A = 0;
}
void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0) | (1<<ADLAR);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	
}

uint16_t pomiar_adc(uint8_t kanal)
{
	// wybór kanalu
	// of ‘kanal’ between 0 and 7
	kanal &= (1<<PC0);
	ADMUX = (ADMUX & 0xF8) | kanal; // clears the bottom 3 bits before ORing
	
	// start single convertion
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADCH);
}

//uart
void UART_init(unsigned int UBRR)
{
	// Set baud rate
	UBRR0H = (unsigned char)(UBRR>>8);
	UBRR0L = (unsigned char)UBRR;
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	// Set frame format: 8data, 2stop bit
	UCSR0C = (1<<USBS0) | (3<<UCSZ00);
}

void UART_transmit(unsigned char data)
{
	// Wait for empty transmit buffer
	while ( !( UCSR0A & (1<<UDRE0)) );
	// Put data into buffer, sends the data
	UDR0 = data;
}
void send_adc_via_uart(uint8_t adc_val) {
	char adc_str[4]; // create a char array to store the converted ADC value as a string
	itoa(adc_val, adc_str, 10); // convert the ADC value to a string and store it in adc_str
	for (int i = 0; i < sizeof(adc_str); i++) {
		UART_transmit(adc_str[i]); // send each character of the adc_str via UART
	}
	UART_transmit('\r'); // add a line return after sending the ADC value
	UART_transmit('\n');
}
int main(void)
{
	init_led();
	uint16_t  adc_wynik;
	adc_init();
	
	int haslo[4] = {0,0,0,0};
	int licz = 0;
	int spraw[1] = {0};
	
	
	UART_init(BAUDRATE);
	char print[] = "Prosze wpisac haslo";
	
	for (int i = 0; i < sizeof(print); i++)
	{
		UART_transmit(print[i]);
	}
	
	
	while (1)
	{
		serwo_init();
		init_led();
		adc_wynik = pomiar_adc(0);
		_delay_ms(1000);
		send_adc_via_uart(adc_wynik); // send the ADC value via UART
		
		send_adc_via_uart(licz);
		
		if(pomiar_adc(0) < 249) // jak jest inna wartoœæ ni¿nominalna
		{
			haslo[licz]=pomiar_adc(0); //do kolejnyhc komorek hasla daje ta wartosc
			licz++; // indeks hasla
		}
		send_adc_via_uart(haslo[0]);
		send_adc_via_uart(haslo[1]);
		send_adc_via_uart(haslo[2]);
		send_adc_via_uart(haslo[3]);
		
		
		if(licz>3 && pomiar_adc(0)>225 && pomiar_adc(0)<248) //klawisz 9 zamyka 
		{
			PORTB &= ~(1<<PB3);
			servo_left();
			licz=0;
			spraw[0]=0;
			for (int i = 0; i < 4; i++)
			{
				haslo[i]=0;
			}
		}
		if(licz>3 && pomiar_adc(0)<5) // 0 resetuje
		{
			PORTB &= ~(1<<PB3);
			alarm_stop();
			licz=0;
			spraw[0]=0;
			for (int i = 0; i < 4; i++)
			{
				haslo[i]=0;
			}
		}
		
		
		if(licz>3) {kb(haslo,spraw);} //jak zostalo przypisany 4 numer daje haslo do sprawdzenia
		
		
	}
}
void kb(int has[4], int spraw[1])
{
	int klawiatura[10] = {0,1,2,3,4,5,6,7,8,9};
	

	{
		if(spraw[0] == 0)
		{
			for(int i = 0; i < 4; i++)
			{
				if (has[i] < 5) has[i] = klawiatura[0]; //0
				else if(has[i] > 24 && has[i] < 35) has[i] = klawiatura[1]; //29 +-2
				else if(has[i] > 49 && has[i] < 59) has[i] = klawiatura[2]; //54 +-2
				else if(has[i] > 68 && has[i] < 78) has[i] = klawiatura[3]; //73 +-2
				else if(has[i] > 97 && has[i] < 107) has[i] = klawiatura[4]; //101 +-2
				else if(has[i] > 127 && has[i] < 137) has[i] = klawiatura[5]; //132 +-2
				else if(has[i] > 146 && has[i] < 156) has[i] = klawiatura[6]; //151 +-2
				else if(has[i] > 167 && has[i] < 177) has[i] = klawiatura[7]; //176 +-2
				else if(has[i] > 204 && has[i] < 214) has[i] = klawiatura[8]; //208 +-2
				else if(has[i] > 226 && has[i] < 238) has[i] = klawiatura[9]; //232 +-2
			}
			
		}
		spraw[0]=1;
		
		if( has[0]== 1 && has[1] == 2 && has[2] == 3 && has[3] == 4)
		{
			zielony_led();
			servo_right();
		}
		else
		{
			alarm();
		}

	}

}
