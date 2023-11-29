#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

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

const int MPUAddr = 0x68;

const int Reg_GyroConfig = 0x1b;
const int Reg_AccelConfig = 0x1c;
const int Reg_Accel = 0x3b;
const int Reg_Temp = 0x41;
const int Reg_Gyro = 0x43;
const int Reg_PwrMgmt1 = 0x6b;
const int Reg_WhoAmI = 0x75;

int ReadWord(int mpu, int reg)
{
	int v = (wiringPiI2CReadReg8(mpu, reg) << 8) + wiringPiI2CReadReg8(mpu, reg + 1);
	if (v & 0x8000)
		v = -(65536 - v);
	return v;
}

void Calibrate(int mpu, int reg, int count, int offsets[3])
{
	long sum[3] = {};
	for (int m = 0; m < count; ++m) {
		for (int i = 0; i < 3; ++i) {
			sum[i] += ReadWord(mpu, reg + i * 2);
		}
		delay(1);
	}
	
	for (int i = 0; i < 3; ++i) {
		offsets[i] = -sum[i] / count;
	}
}

int main(int argc, char *argv[])
{
	printf("Measuring acceleration, gyro and temperature, press enter to exit\n");
	
	int mpu = wiringPiI2CSetup(MPUAddr);
	
	int id = wiringPiI2CReadReg8(mpu, Reg_WhoAmI);
	printf("Id: %x\n", id);
	
	wiringPiI2CWriteReg8(mpu, Reg_PwrMgmt1, 0x00); // disable sleep, enable temperature sensor
	wiringPiI2CWriteReg8(mpu, Reg_GyroConfig, 0x00); // set max gyro sensitivity
	int accelCfg = wiringPiI2CReadReg8(mpu, Reg_AccelConfig);
	accelCfg &= ~(0x3 << 3); // max accel sensitivity
	wiringPiI2CWriteReg8(mpu, Reg_AccelConfig, accelCfg);
	
	printf("Calibrating, don't move the sensor\n");
	int accelOffs[3], gyroOffs[3];
	Calibrate(mpu, Reg_Accel, 1000, accelOffs);
	accelOffs[2] += 16384; // we should be reading 1g down along Z, not zero
	Calibrate(mpu, Reg_Gyro, 1000, gyroOffs);
	
	
	while (!has_input()) {
		int accel[3], gyro[3], temp;
		float faccel[3], fgyro[3], ftemp;
		for (int i = 0; i < 3; ++i) {
			accel[i] = ReadWord(mpu, Reg_Accel + i * 2) + accelOffs[i];
			faccel[i] = accel[i] / 16384.0f;
		}

		for (int i = 0; i < 3; ++i) {
			gyro[i] = ReadWord(mpu, Reg_Gyro + i * 2) + gyroOffs[i];
			fgyro[i] = gyro[i] / 131.0f;
		}
		temp = ReadWord(mpu, Reg_Temp);
		ftemp = temp / 340.0f + 36.53f;
	
		printf("Accel X: %.2f g, y: %.2f g, z: %.2f g\n", faccel[0], faccel[1], faccel[2]);
		printf("Gyro x: %.2f deg/s, y: %.2f deg/s, z: %.2f deg/s\n", fgyro[0], fgyro[1], fgyro[2]);
		printf("Temp %.2f C\n\n", ftemp);
		delay(100);
	}

	consume_line();
	printf("Bye!\n");
	
	return 0;
}
