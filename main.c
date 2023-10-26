#define __AVR_ATmega329P__
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

#include <stdbool.h>
#include <stdint.h>

#define MODE_SW (PINE & (1 << PE1))
#define POWER_SW (PINE & (1 << PE0))
#define SPEED_SW (PINE & (1 << PE2))

#define POWER_LED PC7

#define USE_OVERFLOW

enum mode{
    flashing,
    half_blink,
    led_chain
} current_mode = flashing;

bool powered = false;
uint8_t speed = 1;

uint8_t buffer = 0;

ISR(PCINT0_vect){
    cli();
    if(!MODE_SW){
        if(current_mode == led_chain)
            current_mode = flashing;
        else
            current_mode++;
        buffer = 0;
    }
    else if(!POWER_SW){
        powered = !powered;
        PORTC = (powered << POWER_LED);
    }
    else if(!SPEED_SW){
        speed += 1;
        if(speed > 10)
            speed = 1;
        OCR1A = speed * 1000;
    }    
    sei();
}

void timer_init(){
    TCCR1A = 0;
#ifndef USE_OVERFLOW
    TCCR1B = 0 | (1 << WGM12); // включение cts
    TIMSK1 = 0 | (1 << OCIE1A); // включили таймер
    TCCR1B |= (1 << CS10) ; // prescale ((делитель))
#else
    TIMSK1 = 0 | (1 << TOIE1);
    TCCR1B |= (1 << CS10) | (1 << CS11) ; // prescale ((делитель))
    OCR1A = 1000; // время таймера
#endif
}

#ifdef USE_OVERFLOW
ISR(TIMER1_OVF_vect){
#else
ISR(TIMER1_COMPA_vect){
#endif
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

    sei();
    while (true){}
}