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

struct BtnState {
	char label;
	int state;
	unsigned stateTime;
	bool consumed;
};

void btn_init(struct BtnState *btn, char label)
{
	btn->label = label;
	btn->state = -1;
	btn->consumed = false;
}

void btn_update(struct BtnState *btn, int pin)
{
	int state = digitalRead(pin);
	if (state != btn->state) {
		btn->state = state;
		btn->stateTime = millis();
		btn->consumed = false;
	}
}

const int ColPins[] = {3, 2, 0, 7};
const int RowPins[] = {6, 5, 4, 1};

#define Cols ARRAY_SIZE(ColPins)
#define Rows ARRAY_SIZE(RowPins)

char Labels[Rows][Cols] = {
		{'1', '2', '3', 'A'},
		{'4', '5', '6', 'B'},
		{'7', '8', '8', 'C'},
		{'*', '0', '#', 'D'},
	};

struct BtnState Buttons[Rows][Cols];

int main(int argc, char *argv[])
{
	printf("Press button to toggle LED, press enter to exit\n");
	
	wiringPiSetup();
	
	for (int r = 0; r < Rows; ++r) {
		for (int c = 0; c < Cols; ++c) {
			btn_init(&Buttons[r][c], Labels[r][c]);
		}
		pinMode(RowPins[r], INPUT);
		pullUpDnControl(RowPins[r], PUD_UP);
	}
	
	for (int c = 0; c < Cols; ++c) {
		// all column pins are by default in high impedance (input) state
		// we activate them one at a time
		// because they might be shorted through the keys of the keypad matrix
		pinMode(ColPins[c], INPUT);
	}
	
	while (!has_input()) {
		for (int c = 0; c < Cols; ++c) {
			digitalWrite(ColPins[c], LOW);
			pinMode(ColPins[c], OUTPUT);
			delayMicroseconds(10);
			
			for (int r = 0; r < Rows; ++r) {
				struct BtnState *btn = &Buttons[r][c];
				btn_update(btn, RowPins[r]);
				int now = millis();
				if (!btn->consumed && now - btn->stateTime >= 20) {
					printf("Button '%c': %d\n", btn->label, btn->state);
					btn->consumed = true;
				}
			}
			
			digitalWrite(ColPins[c], HIGH);
			pinMode(ColPins[c], INPUT);
		}
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
