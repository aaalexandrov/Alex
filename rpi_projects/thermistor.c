#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <wiringPi.h>
#include <pcf8591.h>
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

const int PcfPin = 100;
const int I2cAddr = 0x48; // this address can be checked with "gpio i2cd" at the terminal

const double Tzero = 273.15;
const double B = 3950; // Thermistor thermal index
const double Tnom = 25; // Thermistor nominal temperature
const double Rnom = 10; // Thermistor resistance at nominal temperature, kOhm
const double Rseries = 10; // Series resistor, kOhm

// Thermistor resistance Rt = Rnom*e^(B*(1/T-1/Tnom)), temperatures in Kelvin

int main(int argc, char *argv[])
{
	printf("Measuring temperature, press enter to exit\n");
	
	wiringPiSetup();
	
	pcf8591Setup(PcfPin, I2cAddr);
	
	while (!has_input()) {
		int val = analogRead(PcfPin + 0);
		double voltage = val / 255.0 * 3.3;
		double Rt = Rseries * voltage / (3.3 - voltage);
		double T = 1.0 / (1.0 / (Tnom + Tzero) + log(Rt / Rnom) / B);
		double Tc = T - Tzero;
		printf("Input value: %d, voltage: %g, temperature: %g\n", val, voltage, Tc);
		delay(20);
	}
	
	consume_line();
	printf("Bye!\n");
	
	return 0;
}
