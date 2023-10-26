build: main.hex

main.hex: main.elf
	avr-objcopy -O ihex -j .text -j .data main.elf main.hex
main.elf: main.c
	avr-gcc main.c -O2 -o main.elf -mmcu=atmega329p -DF_CPU=8000000UL -std=gnu99
clear:
	rm main.elf main.hex
clean:
	rm main.elf main.hex
