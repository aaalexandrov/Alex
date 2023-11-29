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

const int MotorPins[] = {1, 4, 5, 6};
const int NumPins = ARRAY_SIZE(MotorPins);

const int RevolutionCycles = 512;
const int RevolutionMaxuS = 12000;

const int StepMask[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09};
//const int StepMask[] = {0x01, 0x02, 0x04, 0x08};
const int NumSteps = ARRAY_SIZE(StepMask);

void SetStep(int mask)
{
	for (int i = 0; i < NumPins; ++i) {
		digitalWrite(MotorPins[i], (mask & (1 << i)) ? HIGH : LOW);
	}
}

void CycleSteps(bool back, int us)
{
	for (int i = 0; i < NumSteps; ++i) {
		int step = StepMask[back ? NumSteps - i - 1 : i];
		SetStep(step);
		delayMicroseconds(us);
	}
}

int main(int argc, char *argv[])
{
	printf("Rotating stepping motor, press enter to exit\n");
	
	wiringPiSetup();
	
	for (int i = 0; i < NumPins; ++i) {
		pinMode(MotorPins[i], OUTPUT);
	}
	
	bool back = false;
	while (!has_input()) {
		for (int c = 0; c < RevolutionCycles; ++c) {
			CycleSteps(back, RevolutionMaxuS / NumSteps);
		}
		back = !back;
		delay(1000);
	}
	
	for (int i = 0; i < NumPins; ++i) {
		digitalWrite(MotorPins[i], LOW);
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
