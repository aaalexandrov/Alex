#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <wiringPi.h>

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

const int NumBits = 10;

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

int main(int argc, char *argv[])
{
	printf("Animating LED strip, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(SerialPin, OUTPUT);
	pinMode(ClockPin, OUTPUT);
	pinMode(LatchPin, OUTPUT);
	
	unsigned val = 1u << (NumBits - 1);
	while (!has_input()) {
		SendData(val, NumBits);
		val >>= 1;
		if (!val) {
			val = 1u << (NumBits - 1);
		}
		delay(100);
	}
	
	SendData(0x00, NumBits);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
