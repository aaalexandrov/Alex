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

const int LedPin = 0;

const int PcfPin = 100;
const int I2cAddr = 0x48; // this address can be checked with "gpio i2cd" at the terminal

int main(int argc, char *argv[])
{
	printf("Rotate potentiometer to chage LED brightness, press enter to exit\n");
	
	wiringPiSetup();
	
	softPwmCreate(LedPin, 0, 100);
	pcf8591Setup(PcfPin, I2cAddr);
	
	while (!has_input()) {
		int val = analogRead(PcfPin + 0);
		const int vmin = 80;
		const int vmax = 130;
		val = (val - vmin) * 255 / (vmax - vmin);
		if (val < 0)
			val = 0;
		if (val > 255)
			val = 255;
		printf("Input read: %d\n", val);
		softPwmWrite(LedPin, val * 100 / 255);
		delay(20);
	}
	
	softPwmStop(LedPin);
	pinMode(LedPin, OUTPUT);
	digitalWrite(LedPin, LOW);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
