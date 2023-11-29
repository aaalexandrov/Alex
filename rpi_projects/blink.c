#include <stdio.h>
#include <wiringPi.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>

static const int STDIN = 0;
struct termios OrgTerm = {};

void term_echo_off()
{
	// Use termios to turn off line buffering and echo
	tcgetattr(STDIN, &OrgTerm);
	struct termios term = OrgTerm;
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN, TCSANOW, &term);
	setbuf(stdin, NULL);
}

int kbhit(void) 
{
    int nbbytes;
    ioctl(STDIN, FIONREAD, &nbbytes);
    return nbbytes;
}

void term_restore()
{
	// consume all characters entered
	int num = kbhit();
	for (int i = 0; i < num; ++i) {
		getchar();
	}
	tcsetattr(STDIN, TCSANOW, &OrgTerm);
}

const int LedPin = 0;

int main(int argc, char *argv[])
{
	printf("Blinking, press any key to exit\n");
	term_echo_off();
	
	wiringPiSetup();
	
	pinMode(LedPin, OUTPUT);
	
	while (!kbhit()) {
		digitalWrite(LedPin, HIGH);
		delay(1000);
		digitalWrite(LedPin, LOW);
		delay(1000);
	}

	term_restore();
	printf("Bye!\n");
	
	return 0;
}
