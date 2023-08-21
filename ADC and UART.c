#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#define BAUD 9600
#define BAUDRATE F_CPU/16/BAUD-1

void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0) | (1<<ADLAR);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	
	//LED
	DDRB |= (1<<PB0);
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
	// ADSC becomes ’0′ again
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
	uint16_t  adc_wynik;
	
	adc_init();
	
	
	UART_init(BAUDRATE);
	char print[] = "Prosze wpisac haslo\r\n";
	
	for (int i = 0; i < sizeof(print); i++)
	{
		UART_transmit(print[i]);
	}

	while (1)
	{
		adc_wynik = pomiar_adc(0);
		if(adc_wynik < 0x1F) PORTB |= (1<<PB0);
		else if(adc_wynik >=0x1F) PORTB &= ~(1<<PB0);
		send_adc_via_uart(adc_wynik); // send the ADC value via UART

	}
}
