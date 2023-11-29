#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <wiringPi.h>
#include <softTone.h>

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

struct BtnState {
	int pin;
	int state;
	unsigned stateTime;
};

void btn_update(struct BtnState *btn)
{
	int state = digitalRead(btn->pin);
	if (state != btn->state) {
		btn->state = state;
		btn->stateTime = millis();
	}
}

void btn_init(struct BtnState *btn, int pin)
{
	btn->pin = pin;
	btn->state = -1;
	btn_update(btn);
}

const int BuzzPin = 0;
const int BtnPin = 1;

int main(int argc, char *argv[])
{
	printf("Press button to toggle sound, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(BuzzPin, OUTPUT);
	softToneCreate(BuzzPin);
	
	pinMode(BtnPin, INPUT);
	pullUpDnControl(BtnPin, PUD_UP);
	struct BtnState btn;
	btn_init(&btn, BtnPin);

	int buzzState = LOW;
	unsigned consumedTime = -1;
	
	while (!has_input()) {
		btn_update(&btn);
		unsigned now = millis();
		if (btn.state == LOW && btn.stateTime != consumedTime && now - btn.stateTime >= 50) {
			buzzState = !buzzState;
			consumedTime = btn.stateTime;
		}
		int freq = 0;
		if (buzzState == HIGH) {
			freq = 2000 + (int)roundf(500 * sinf((now - consumedTime) * M_PI / 1000.0f)); // 2kHz is the resonant frequency of the buzzer
		} 
		softToneWrite(BuzzPin, freq);
		delay(1);
	}
	softToneStop(BuzzPin);
	digitalWrite(BuzzPin, LOW);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
