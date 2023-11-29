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

const int DigitPins[] = {6, 5, 4, 1};

const int NumBits = 8;

void SendData(unsigned val, int bits)
{
	digitalWrite(LatchPin, LOW);
	for (int i = bits - 1; i >= 0; --i) {
		digitalWrite(ClockPin, LOW);
		digitalWrite(SerialPin, (val & (1u << i)) ? HIGH : LOW);
		delayMicroseconds(1);
		digitalWrite(ClockPin, HIGH);
		delayMicroseconds(1);
	}
	digitalWrite(LatchPin, HIGH);
	delayMicroseconds(1);
}

const int Digits[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

void ScanDigits(unsigned val)
{
	for (int i = 0; i < 4; ++i, val >>= 8) {
		unsigned char n = (val & 0xf);
		unsigned v = Digits[n];
		if (val & 0xf0) {
			v |= 0x80;
		}
		SendData(~v, NumBits);
		digitalWrite(DigitPins[i], LOW);
		delay(1);
		digitalWrite(DigitPins[i], HIGH);
	}
}

unsigned EncodeSeconds(int ms)
{
	ms /= 100;
	unsigned res = 0;
	res = ms % 10;
	ms /= 10;
	res |= (0x80 | (ms % 10)) << 8;
	ms /= 10;
	res |= (ms % 10) << 16;
	ms /= 10;
	res |= (ms % 10) << 24;
	return res;
}

int main(int argc, char *argv[])
{
	printf("Counting seconds, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(SerialPin, OUTPUT);
	pinMode(ClockPin, OUTPUT);
	pinMode(LatchPin, OUTPUT);
	
	for (int i = 0; i < 4; ++i) {
		pinMode(DigitPins[i], OUTPUT);
		digitalWrite(DigitPins[i], HIGH);
	}
	
	int start = millis();
	while (!has_input()) {
		ScanDigits(EncodeSeconds(millis() - start));
	}
	
	for (int i = 0; i < 4; ++i) {
		pinMode(DigitPins[i], OUTPUT);
		digitalWrite(DigitPins[i], HIGH);
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
