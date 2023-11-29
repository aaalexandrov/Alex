#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>

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

uint64_t GetMicros()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return ts.tv_sec * 1000000ull + ts.tv_nsec / 1000;
	
}

void DelayMicros(uint32_t us)
{
	uint64_t start = GetMicros();
	while (GetMicros() - start < us);
}


// values for BCM2711, Raspberry PI 4B

const uintptr_t Peripherals_Phys_Base = 0x7e000000;
const uintptr_t Peripherals_Virt_Base = 0xfe000000;
const uintptr_t Peripherals_Phys_Size = 0x01000000;

const uintptr_t GPIO_Base_Offset = 0x200000;
const uintptr_t GPFSEL2_Reg = 0x08;
const uint32_t  GPIO21_Mask = 0x7 << 3;

const uintptr_t GPCLK_Base_Offset = 0x101000;
const uintptr_t CLK1_Reg_Ctl = 0x78;
const uintptr_t CLK1_Reg_Div = 0x7c;
const uintptr_t CLK_BUSY_Bit = (0x1 << 7);
const uintptr_t CLK_ENAB_Bit = (0x1 << 4);
const uint32_t CLK_Frac_Bits = 12;

const uint64_t PLLD_Freq = 750000000;
// channels above 93MHz seem to not work
// this is probably due to the clock MASH filter which can only operate 
// with frequencies below 25MHz according to the documentation, which
// we're exceeding by quite a margin
const uint64_t Radio_Freq = 92000000;
const uint64_t Radio_Freq_Spread = 75000;

void *MapPeripherals()
{
	int memFd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memFd < 0)
		return NULL;
	void *mapped = mmap(NULL, Peripherals_Phys_Size, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, Peripherals_Virt_Base);
	close(memFd);
	if (mapped == MAP_FAILED)
		return NULL;
	return mapped;
}

void SetClockRegisters(volatile uint32_t *ctl, uint32_t ctlValue, volatile uint32_t *div, uint32_t divValue)
{
	// set clock password in high byte
	ctlValue = (0x5a << 24) | (ctlValue & 0xffffff);
	divValue = (0x5a << 24) | (divValue & 0xffffff);
	
	uint32_t ctlCur = *ctl;
	*ctl = ctlCur & ~CLK_ENAB_Bit; 
	
	// documentation says to wait for the BUSY bit to go down, however it seems it never does, so just insert some delay here
	//while (*ctl & CLK_BUSY_Bit);
	DelayMicros(100);
	
	*div = divValue;
	*ctl = ctlValue;
}

uint32_t CalculateDivisor(float value)
{
	uint64_t freq = Radio_Freq + (int32_t)(roundf(value * Radio_Freq_Spread));
	return (PLLD_Freq << CLK_Frac_Bits) / freq;
}

int main(int argc, char *argv[])
{
	uint8_t *peripherals = (uint8_t*)MapPeripherals();
	if (!peripherals) {
		printf("Failed mapping peripheral registers, aborting\n");
		return -1;
	}

	printf("Playing FM radio sound at %.1fMhz, press enter to stop\n", Radio_Freq / 1000000.0);

	volatile uint32_t *gpfsel2 = (volatile uint32_t*)(peripherals + GPIO_Base_Offset + GPFSEL2_Reg);
	uint32_t fsel2org = *gpfsel2;
	*gpfsel2 = (fsel2org & ~GPIO21_Mask) | (0x2 << 3); // GPIO21 output set to alt6 mode, which is CLK1
	
	volatile uint32_t *clk1Ctl = (volatile uint32_t*)(peripherals + GPCLK_Base_Offset + CLK1_Reg_Ctl);
	volatile uint32_t *clk1Div = (volatile uint32_t*)(peripherals + GPCLK_Base_Offset + CLK1_Reg_Div);
	uint32_t clk1CtlOrg = *clk1Ctl;
	uint32_t clk1DivOrg = *clk1Div;
	
	// set 1 stage MASH filter, ENAB & source 6 = PLLD
	uint32_t clk1CtlPlld = (0x1 << 9) | CLK_ENAB_Bit | 0x6;
	uint32_t divisor = CalculateDivisor(0.0f);

	SetClockRegisters(clk1Ctl, clk1CtlPlld, clk1Div, divisor);
	
	uint64_t start = GetMicros();
	uint64_t prev = start;
	float phase = 0;
	uint64_t updates = 0;
	while (!has_input()) {
		uint64_t us = GetMicros();
		uint64_t dt = us - prev;
		prev = us;
		
		uint32_t freq = 1000 + 500 * sinf((us - start) * 2 * M_PI / 5000000);
		float dphase = dt * 2 * M_PI * freq / 1000000;
		phase = fmodf(phase + dphase, 2 * M_PI);
		float v = sinf(phase);
		divisor = CalculateDivisor(v);
		*clk1Div = (0x5a << 24) | divisor;
		
		++updates;
		
		DelayMicros(20);
	}
	
	uint64_t end = GetMicros();
	
	*gpfsel2 = fsel2org;
	SetClockRegisters(clk1Ctl, clk1CtlOrg, clk1Div, clk1DivOrg);
	
	printf("Clock updates per sec: %2f\n", updates / (double)(end - start) * 1000000);
	
	consume_line();
	printf("Bye\n");
	
	return 0;
}
