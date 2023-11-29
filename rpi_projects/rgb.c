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

const int LedPins[] = {0, 2, 1};

void init_rgb()
{
	for (int i = 0; i < 3; ++i) {
		softPwmCreate(LedPins[i], 100, 100);
	}
}

void set_rgb(int r, int g, int b)
{
	int rgb[] = {r, g, b};
	for (int i = 0; i < 3; ++i) {
		softPwmWrite(LedPins[i], 100 - rgb[i]);
	}
}

void deinit_rgb()
{
	for (int i = 0; i < 3; ++i) {
		softPwmStop(LedPins[i]);
		pinMode(LedPins[i], OUTPUT);
		digitalWrite(LedPins[i], HIGH);
	}
}

int main(int argc, char *argv[])
{
	printf("LED color animating, press enter to exit\n");
	
	wiringPiSetup();

	init_rgb();
	
	int r = 0, g = 0, b = 0;
	while (!has_input()) {
		r += 10;
		if (r > 100) {
			r = 0;
			g += 10;
			if (g > 100) {
				g = 0;
				b += 10;
				if (b > 100) {
					b = 0;
				}
			}
		}
		set_rgb(r, g, b);
		delay(20);
	}
	
	deinit_rgb();

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
