#include <stdio.h>
#include <wiringPi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>

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
const int BtnPin = 1;

int main(int argc, char *argv[])
{
	printf("Press button to light LED, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(LedPin, OUTPUT);
	
	pinMode(BtnPin, INPUT);
	pullUpDnControl(BtnPin, PUD_UP);
	
	while (!has_input()) {
		if (digitalRead(BtnPin) == LOW) {
			//printf("Button low\n");
			digitalWrite(LedPin, HIGH);
		} else {
			//printf("Button HIGH\n");
			digitalWrite(LedPin, LOW);
		}
	}
	digitalWrite(LedPin, LOW);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
