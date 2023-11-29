#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <pcf8591.h>
#include <softPwm.h>

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

const int LedPins[] = {3, 2, 0};

const int PcfPin = 100;
const int I2cAddr = 0x48; // this address can be checked with "gpio i2cd" at the terminal

int main(int argc, char *argv[])
{
	printf("Rotate potentiometer to chage LED brightness, press enter to exit\n");
	
	wiringPiSetup();
	
	for (int i = 0; i < 3; ++i) {
		softPwmCreate(LedPins[i], 100, 100);
	}
	
	pcf8591Setup(PcfPin, I2cAddr);
	
	while (!has_input()) {
		for (int i = 0; i < 3; ++i) {
			int val = analogRead(PcfPin + i);
			softPwmWrite(LedPins[i], 100 - val * 100 / 255);
		}
		delay(20);
	}
	
	for (int i = 0; i < 3; ++i) {
		softPwmStop(LedPins[i]);
		pinMode(LedPins[i], OUTPUT);
		digitalWrite(LedPins[i], HIGH);
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
