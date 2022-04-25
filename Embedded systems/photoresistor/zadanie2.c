#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>

#define R1 10000
#define VCC 5.0
#define V_REF 5.0

#define BTN PD2
#define BTN_PORT PORTD

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

volatile double resistance;

// inicjalizacja UART
void uart_init()
{
    // ustaw baudrate
    UBRR0 = UBRR_VALUE;
    // wyczyść rejestr UCSR0A
    UCSR0A = 0;
    // włącz odbiornik i nadajnik
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    // ustaw format 8n1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
    // czekaj aż transmiter gotowy
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = data;
    return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
    // czekaj aż znak dostępny
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

// oczekiwanie na zakończenie transmisji
void uart_wait()
{
    while (!(UCSR0A & _BV(TXC0)))
        ;
}

FILE uart_file;

// inicjalizacja ADC
void adc_init()
{
    //MUX = 0101  -- ADC5
    ADMUX = _BV(REFS0) | _BV(MUX0) | _BV(MUX2); // referencja AVcc, wejście ADC5
    DIDR0 = _BV(ADC5D);                         // wyłącz wejście cyfrowe na ADC5
    // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
    ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
    ADCSRA |= _BV(ADIE);                            // ADC Interrupt Enable
    ADCSRA |= _BV(ADATE);                           // ADC Auto Trigger Enable
    ADCSRB |= _BV(ADTS1);                           // External Interrupt Request 0
    ADCSRA |= _BV(ADEN);                            // enable ADC
}

void io_init()
{
    // ustaw pull-up na PD2(INT0)
    BTN_PORT |= _BV(BTN);
    // ustaw wyzwalanie przerwania na INT0 zboczem narastającym
    EICRA |= _BV(ISC00) | _BV(ISC01);
    // odmaskuj przerwania dla INT0
    EIMSK |= _BV(INT0);
}

ISR(INT0_vect) {} //ta procedura musi być i musi być pusta, aby poprawnie działał ADC Auto Trigger, który jest ustawiony na External Interrupt Request 0

ISR(ADC_vect)
{
    resistance = ADC * V_REF * R1 / (1024 * VCC - ADC * V_REF);
}

int main()
{
    // zainicjalizuj UART
    uart_init();
    // skonfiguruj strumienie wejścia/wyjścia
    fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    // zainicjalizuj ADC
    adc_init();
    // zainicjalizuj wejścia/wyjścia
    io_init();
    // odmaskuj przerwania
    sei();
    while (1)
    {
        printf("Odczytano: %fΩ\r\n", resistance);
        _delay_ms(1000);
    }
}