#define F_CPU 16000000UL  // 16 MHz - czêstotliwoœæ pracy mikrokontrolera
#include <avr/io.h>  // Biblioteka zawieraj¹ca m.in. definicje rejestrów mikrokontrolera
#include <util/delay.h>  // Biblioteka zawieraj¹ca funkcjê delay_ms() s³u¿¹c¹ do opóŸnieñ

#define SERVO_DDR DDRD  // Rejestr kierunku (DDR) dla pinu obs³uguj¹cego serwo (PD5)
#define SERVO_PORT PORTD  // Rejestr wyjœcia (PORT) dla pinu obs³uguj¹cego serwo (PD5)
#define SERVO_POSITION_MIN 500  // Odpowiada k¹towi oko³o 0 stopni
#define SERVO_POSITION_MAX 2500 // Odpowiada k¹towi oko³o 180 stopni
#define PD5 5 //Definicja pi¹tego pinu dla PORTD

// Ustawienie pozycji serwa

void set_servo_position(uint16_t position)
{
	// Sprawdzenie, czy pozycja serwa jest w dopuszczalnym zakresie
	if (position < SERVO_POSITION_MIN)
	{
		position = SERVO_POSITION_MIN;
	}
	else if (position > SERVO_POSITION_MAX)
	{
		position = SERVO_POSITION_MAX;
	}

	// Ustawienie pozycji serwa poprzez zapisanie odpowiedniej wartoœci do rejestru OCR1A
	OCR1A = position;
}

int main(void)
{
	// Inicjalizacja timera oraz pinu obs³uguj¹cego serwo
	// Ustawienie trybu pracy timera: Fast PWM, top = ICR1
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	// Ustawienie prescalera timera na 1
	TCCR1B |= (1 << CS10);

	// Ustawienie wartoœci top timera (ICR1) tak, aby uzyskaæ czêstotliwoœæ PWM 50 Hz (okres 20 ms)
	ICR1 = 19999;
	
	
	// Ustawienie pinu PD5 jako wyjœcie
	SERVO_DDR |= (1 << PD5);
	
	while (1)
	{
		// Ustawienie pozycji serwa na minimum (ok. 0 stopni)
		set_servo_position(SERVO_POSITION_MIN);
		_delay_ms(1000);  // OpóŸnienie 1 sekundy

		// Ustawienie pozycji serwa na maksimum (ok. 180 stopni)
		set_servo_position(SERVO_POSITION_MAX);
		_delay_ms(1000);  // OpóŸnienie 1 sekundy
	}

	return 0;
}