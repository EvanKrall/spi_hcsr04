spi_hcsr04: spi_hcsr04.c
	gcc -std=gnu11 spi_hcsr04.c -o spi_hcsr04

install: spi_hcsr04
	cp spi_hcsr04 /usr/local/bin/spi_hcsr04
