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

const int LedPin = 1;
const int SensorPin = 0;

int main(int argc, char *argv[])
{
	printf("Move around the motion sensor to light LED, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(LedPin, OUTPUT);
	pinMode(SensorPin, INPUT);
	pullUpDnControl(SensorPin, PUD_DOWN);
	
	while (!has_input()) {
		if (digitalRead(SensorPin) == HIGH) {
			digitalWrite(LedPin, HIGH);
		} else {
			digitalWrite(LedPin, LOW);
		}
	}
	digitalWrite(LedPin, LOW);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
