#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
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

const int TrigPin = 2;
const int EchoPin = 0;

const int MaxDist = 220; // cm
const float SpeedOfSound = 340.0f; // m/s
const int MaxPulse = (int)(MaxDist * 2 * 1000000.0f / 100.0f / 340.0f);

int pulseIn(int pin, int level, int timeout)
{
   struct timeval tn, t0, t1;
   long micros;
   gettimeofday(&t0, NULL);
   micros = 0;
   while (digitalRead(pin) != level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(pin) == level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros = micros + (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros = micros + (tn.tv_usec - t1.tv_usec);
   return micros;
}


int measurePulse(int pin, int level, int timeout)
{
	int start = micros();
	while (digitalRead(pin) != level) {
		int now = micros();
		if (now - start >= timeout) {
			return 0;
		}
	}
	
	start = micros();
	while (digitalRead(pin) == level) {
		int now = micros();
		if (now - start >= timeout) {
			return 0;
		}
	}
	
	return micros() - start;
}

float read_distance()
{
	digitalWrite(TrigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(TrigPin, LOW);
	int us = pulseIn(EchoPin, HIGH, MaxPulse * 2);
	return (double)us * 100 / 1000000.0 * SpeedOfSound / 2;
}

int main(int argc, char *argv[])
{
	printf("Ranging distance, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(TrigPin, OUTPUT);
	digitalWrite(TrigPin, LOW);
	
	pinMode(EchoPin, INPUT);
	
	while (!has_input()) {
		float dist = read_distance();
		printf("Distance: %.2f cm\n", dist);
		delay(1000);
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
