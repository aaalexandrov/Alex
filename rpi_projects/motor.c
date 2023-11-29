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

const int DirPins[] = {0, 2};
const int PwmPin = 3;

const int PcfPin = 100;
const int I2cAddr = 0x48; // this address can be checked with "gpio i2cd" at the terminal

int main(int argc, char *argv[])
{
	printf("Rotate potentiometer to chage motor direction and speed, press enter to exit\n");
	
	wiringPiSetup();

	pcf8591Setup(PcfPin, I2cAddr);
	
	pinMode(DirPins[0], OUTPUT);
	pinMode(DirPins[1], OUTPUT);
	pinMode(PwmPin, OUTPUT);
	softPwmCreate(PwmPin, 0, 100);
	
	while (!has_input()) {
		int val = analogRead(PcfPin + 0) - 128;
		
		if (val > 4) {
			digitalWrite(DirPins[0], LOW);
			digitalWrite(DirPins[1], HIGH);
		} else if (val < -4) {
			digitalWrite(DirPins[0], HIGH);
			digitalWrite(DirPins[1], LOW);
		} else {
			digitalWrite(DirPins[0], LOW);
			digitalWrite(DirPins[1], LOW);
		}
		
		printf("Input read: %d\n", val);
		softPwmWrite(PwmPin, abs(val) * 100 / 128);
	}
	
	digitalWrite(DirPins[0], LOW);
	digitalWrite(DirPins[1], LOW);
	softPwmStop(PwmPin);
	digitalWrite(PwmPin, LOW);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
