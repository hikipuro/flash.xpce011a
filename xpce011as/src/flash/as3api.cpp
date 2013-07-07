#include <stdio.h>
#include "AS3/AS3.h"
#include "config.h"
#include "typedef.h"
#include "pce.h"

int UPeriod;
int BaseClock = 7170000;
int Debug=1;
int vmode=0;

extern int IPeriod;
//extern int BaseClock;
extern int TimerPeriod;
//extern int Debug,vmode;
extern int scanlines_per_frame;
extern IO io;
extern M6502 M;
extern byte cart_reload;
extern byte populus;
extern char *pCartName;

int main() {
	IPeriod = BaseClock/(scanlines_per_frame*60);
	TimerPeriod = BaseClock/1000*3*1024/21480;
	vmode = 0;
	Debug = 0;
	
	//if (argc<3) return -1;
	if (!InitMachine()) {
		return -1;
	}
	
	/*
	if (!InitPCE(argv[1], argv[2])) {
		printf("Emulation started");
		while (1) {
			//SetInfoString("Emulation started");
			RunPCE();
			TrashPCE(argv[2]);
			if (!cart_reload)
				break;
			cart_reload = 0;
			InitPCE(pCartName, argv[2]);
		}
	}
	TrashMachine();
	return 0;
	*/
	
	AS3_GoAsync();
}
