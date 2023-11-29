#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <wiringPi.h>
#include <pcf8574.h>
#include <lcd.h>

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

const int I2CAddr = 0x27;
const int Rows = 2;
const int Cols = 16;

enum Pins {
	PinBase = 80,
	PinRS = PinBase,
	PinRW,
	PinEN,
	PinLED,
	PinD4,
	PinD5,
	PinD6,
	PinD7,
	PinLast,
	PinCount = PinLast - PinBase
};

void ScrollMessage(int lcd, int row, int col, char const *msg)
{
	int len = strlen(msg);
	for (int i = 0; i < Cols; ++i) {
		lcdPosition(lcd, i, row);
		int pos = i - col;
		lcdPutchar(lcd, (0 <= pos && pos < len) ? msg[pos] : ' ');
	}
}

int main(int argc, char *argv[])
{
	printf("Displaying on LCD, press enter to exit\n");
	
	wiringPiSetup();
	
	pcf8574Setup(PinBase, I2CAddr); 
	
	for (int i = PinBase; i < PinLast; ++i) {
		pinMode(i, OUTPUT);
	}
	
	digitalWrite(PinLED, HIGH);
	digitalWrite(PinRW, LOW);
	
	int lcd = lcdInit(Rows, Cols, 4, PinRS, PinEN, PinD4, PinD5, PinD6, PinD7, 0, 0, 0, 0);

	char msg[] = "Hi there, the big brown fox that jumped over that lazy dog!!1";
	int len = strlen(msg);
	int pos = 0;
	while (!has_input()) {
		ScrollMessage(lcd, 0, pos, msg);
		--pos;
		if (pos < -len) {
			pos = Cols - 1;
		}
		delay(300);
	}
		
	digitalWrite(PinLED, LOW);
	digitalWrite(PinRW, HIGH);
	
	consume_line();
	printf("Bye!\n");
	
	return 0;
}
