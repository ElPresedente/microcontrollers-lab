#define __AVR_ATmega329P__
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include <stdbool.h>
#include <stdint.h>

#define POWER_LED PC7

enum mode{
    flashing,
    half_blink,
    led_chain
} current_mode = flashing;

bool powered = false;
uint8_t speed = 1;

uint8_t buffer = 0;

struct command{
    unsigned char is_powered : 1;
    unsigned char mode : 2;
    unsigned char speed : 5;
};


void timer_init(){
    TCCR1A = 0;
    TCCR1B = 0 | (1 << WGM12); // включение cts
    TIMSK1 = 0 | (1 << OCIE1A); // включили таймер
    TCCR1B |= (1 << CS10) ; // prescale ((делитель))
}

#define FOSC 1843200
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


void USART_Init(unsigned int ubrr) {
    /* Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0A = 0;
    // RXEN0 - включить приемник пакетов
    // RXCIE0 - включить прерывания на получении пакетов
    UCSR0B = 0 | (1<<RXEN0);//| (1 << RXCIE0 );
    // USBS0 - использовать 2 стоп бита
    // UCSZ00 (2 бита) - использовать пакет 8 бит
    // четность-нечетность отключена (upm00, 2 бита)
    UCSR0C = 0 | (1<<USBS0)|(3<<UCSZ00) | (0 << UPM00);
}

void usart_init(){
    USART_Init(MYUBRR);
}

struct command USART_Receive( void )
{
    /* Wait for data to be received */
    while ( !(UCSR0A & (1<<RXC0)) )
    ;
    /* Get and return received data from buffer */
    return *((struct command*)(&UDR0));
}

void listen_for_commends(){
    struct command current_command = USART_Receive();
    unsigned char test = *((unsigned char*)(&current_command));
    if(test == 'u'){
        powered = true;
    }
    return;
    powered = current_command.is_powered;
    current_mode = current_command.mode;
    speed = current_command.speed;
}

ISR(USART0_RX_vect){
    powered = true;
}

ISR(TIMER1_COMPA_vect){
    PORTC = (powered << POWER_LED);
    if(powered){
        switch(current_mode){
            case flashing:
                if(buffer){
                    PORTA = 0xFF;
                    buffer = false;
                }
                else{
                    PORTA = 0;
                    buffer = true;
                }
                break;
            case half_blink:
                if(buffer){
                    PORTA = 0b10101010;
                    buffer = false;
                }
                else{
                    PORTA = 0b01010101;
                    buffer = true;
                }
                break;
            case led_chain:
                PORTA = (1 << buffer++);
                if(buffer == 8)
                    buffer = 0;
                break;
        }
    }
    else
        PORTA = 0;
}

int main(){
    uint8_t i; // для цикла
    DDRA = 0xFF; // лампочки
    DDRC = (1 << PC7); // питание
    DDRE = 0; // кнопочьки
    MCUCR = (1 << PUD); // подание питания на пины (не нада резистор)
    PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2); // включаем порты 0-2 порта pcint 
    EIMSK = (1 << PCIE0); // включаем порт pcie0 (pcint0:7)
    EICRA = (1 << ISC01) | (1 << ISC00); // не работает, потому что pcint, а не int

    timer_init();
    usart_init();

    //sei();
    SREG = 0 | (1 << 7);
    while (true){
        listen_for_commends();
    }
}