# Makefile for AXI4-Lite test application
CC            = arm-linux-gnueabihf-gcc
CFLAGS        = -Wall -I../include

all: axi4lite_app

axi4lite_app: axi4lite_app.c
	$(CC) $(CFLAGS) -o $@ $

clean:
	rm -f axi4lite_app