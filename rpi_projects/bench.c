#include <stdio.h>
#include <wiringPi.h>

int main(int argc, char *argv[])
{
	printf("Hi!\n");
	
	wiringPiSetup();
	
	int start = micros();
	
	const int count = 1000000;
	
	pinMode(0, OUTPUT);
	
	for (int i = 0; i < count; ++i) {
		digitalWrite(0, HIGH);
		//micros();
	}
	
	int duration = micros() - start;
	
	printf("%d iterations in %dus, which makes %.2f iterations per second\n", count, duration, count * 1000000.0 / duration);
	
	return 0;
}
