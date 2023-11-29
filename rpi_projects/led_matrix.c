#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <wiringPi.h>

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

const int SerialPin = 0;
const int ClockPin = 3;
const int LatchPin = 2;

const int NumBits = 16;

void SendData(unsigned val, int bits)
{
	digitalWrite(LatchPin, LOW);
	for (int i = bits - 1; i >= 0; --i) {
		digitalWrite(ClockPin, LOW);
		digitalWrite(SerialPin, (val & (1u << i)) ? HIGH : LOW);
		delayMicroseconds(10);
		digitalWrite(ClockPin, HIGH);
		delayMicroseconds(10);
	}
	digitalWrite(LatchPin, HIGH);
	delayMicroseconds(10);
}

const unsigned char Image[8] = {0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00};
const unsigned char Smiley[8] = {0x1c, 0x22, 0x51, 0x45, 0x45, 0x51, 0x22, 0x1c};
const unsigned char Zeros[8] = {};

void ScanImage(unsigned char const img[8], bool invert, int rowOffs, int colOffs)
{
	for (int r = 0; r < 8; ++r) {
		unsigned char row = img[(r + rowOffs) % 8];
		if (invert) {
			row = ~row;
		}
		row = (row >> colOffs) | (row << (8 - colOffs));
		unsigned val = (1u << (r + 8)) | (unsigned char)(~row);
		SendData(val, NumBits);
	}
}

int main(int argc, char *argv[])
{
	printf("Animating LED matrix, press enter to exit\n");
	
	wiringPiSetup();
	
	pinMode(SerialPin, OUTPUT);
	pinMode(ClockPin, OUTPUT);
	pinMode(LatchPin, OUTPUT);
	
	int rowOffs = 0;
	int colOffs = 0;
	int rowTime, colTime;
	rowTime = colTime = millis();
	while (!has_input()) {
		ScanImage(Smiley, false, rowOffs, colOffs);
		int now = millis();
		if (now - rowTime > 250) {
			rowOffs = (rowOffs + 1) % 8;
			rowTime = now;
		}
		if (now - colTime > 1500) {
			colOffs = (colOffs + 1) % 8;
			colTime = now;
		}
	}
	
	ScanImage(Zeros, false, 0, 0);

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
