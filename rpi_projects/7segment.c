#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <wiringPi.h>

#ifndef ARRAY_SIZE
#	define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

static const int STDIN = 0;

bool has_input() 
{
    int nbbytes;
    ioctl(STDIN, FIONREAD, &nbbytes);
    return nbbytes;
}

void consume_line()
{
	char *line = NULL;
	size_t lineSize = 0;
	getline(&line, &lineSize, stdin);
	free(line);
}

const int SerialPin = 0;
const int ClockPin = 3;
const int LatchPin = 2;

const int NumBits = 8;

void SendData(unsigned val, int bits)
{
	digitalWrite(LatchPin, LOW);
	for (int i = bits - 1; i >= 0; --i) {
		digitalWrite(ClockPin, LOW);
		digitalWrite(SerialPin, (val & (1u << i)) ? HIGH : LOW);
		delayMicroseconds(10);
		digitalWrite(ClockPin, HIGH);
		delayMicroseconds(10);
	}
	digitalWrite(LatchPin, HIGH);
	delayMicroseconds(10);
}

const int Digits[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

int main(int argc, char *argv[])
{
	printf("Animating digit, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(SerialPin, OUTPUT);
	pinMode(ClockPin, OUTPUT);
	pinMode(LatchPin, OUTPUT);
	
	int digit = 0;
	while (!has_input()) {
		SendData(~Digits[digit], NumBits);
		//SendData(~0x4f, NumBits);
		digit = (digit + 1) % ARRAY_SIZE(Digits);
		delay(500);
	}
	
	SendData(~0x00, NumBits);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
