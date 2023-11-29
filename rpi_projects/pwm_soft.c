#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <wiringPi.h>
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

const int LedPin = 1;

int main(int argc, char *argv[])
{
	printf("LED brightness animating, press enter to exit\n");
	
	wiringPiSetup();

	softPwmCreate(LedPin, 0, 100);
	
	while (!has_input()) {
		for (int i = 0; i <= 100; i += 1) {
			softPwmWrite(LedPin, i);
			delay(16);
		}
	}
	
	pinMode(LedPin, OUTPUT);
	digitalWrite(LedPin, HIGH);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
