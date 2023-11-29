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

const int ServoPin = 1;
const int Servo0time = 5; // softPwm times in multiples of 100uS
const int Servo180time = 25;

int main(int argc, char *argv[])
{
	printf("Servo moving, press enter to exit\n");
	
	wiringPiSetup();

	softPwmCreate(ServoPin, 0, 200);
	
	while (!has_input()) {
		for (int i = 0; i <= 180; i += 1) {
			int time = i * (Servo180time - Servo0time) / 180 + Servo0time;
			softPwmWrite(ServoPin, time);
			delay(10);
		}
		delay(1000);
		for (int i = 180; i >= 0; i -= 1) {
			int time = i * (Servo180time - Servo0time) / 180 + Servo0time;
			softPwmWrite(ServoPin, time);
			delay(10);
		}
		delay(1000);
	}
	
	pinMode(ServoPin, OUTPUT);
	digitalWrite(ServoPin, LOW);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
