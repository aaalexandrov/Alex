#include <stdio.h>
#include <wiringPi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>

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

const int LedPins[] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10};

int main(int argc, char *argv[])
{
	printf("LEDs animating, press enter to exit\n");
	
	wiringPiSetup();

	for (int i = 0; i < ARRAY_SIZE(LedPins); ++i) {
		pinMode(LedPins[i], OUTPUT);
		digitalWrite(LedPins[i], HIGH);
	}
	
	unsigned startTime = millis();
	
	while (!has_input()) {
		unsigned now = millis();
		int l0 = roundf((sinf((now - startTime) / 1000.0f * 2 * M_PI) + 1) / 2 * (ARRAY_SIZE(LedPins) - 1));
		int l1 = roundf((sinf((now - startTime) / 1000.0f * 0.3f * M_PI) + 1) / 2 * (ARRAY_SIZE(LedPins) - 1));
		if (l0 > l1) {
			int t = l0;
			l0 = l1;
			l1 = t;
		}
		
		for (int i = 0; i < ARRAY_SIZE(LedPins); ++i) {
			digitalWrite(LedPins[i], l0 <= i && i <= l1 ? LOW : HIGH);
		}
		delay(50);
	}
	
	for (int i = 0; i < ARRAY_SIZE(LedPins); ++i) {
		digitalWrite(LedPins[i], HIGH);
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
