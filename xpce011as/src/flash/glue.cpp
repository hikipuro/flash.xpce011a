#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "AS3/AS3.h"
#include "config.h"
#include "typedef.h"
#include "pce.h"

#define	NODATA	0xff

unsigned char* XBuf;
unsigned char* bytes;
short* soundBytes;
short* soundBytesWork;
int PalTable[256];
unsigned char keys;

BOOL snd_bSound;

DWORD snd_dwSampleRate = 44100;
DWORD snd_dwChannels = 2;
int snd_CoopMode = 2;
int snd_Latency = 40;

int		ds_nBits		= 16;
int		ds_nChannels	= 2;
int		ds_nSampleRate	= 44100;
int		ds_nLockLatency	= 200;
int		ds_CoopMode		= 2;

#define SOUND_BUF_MS	200
extern WORD sbuf[6][44100*SOUND_BUF_MS/1000*sizeof(WORD)*2];

/*
extern int UPeriod;
extern int scanlines_per_frame;
extern IO io;
extern byte cart_reload;
extern byte populus;
*/

extern int ROM_size;
//extern byte* ROM;
extern byte *VRAM,*ROM,*vchange,*vchanges,*PCM,*WRAM,*DMYROM,*IOAREA;
extern byte PopRAM[0x10000];
extern byte *Page[8],*ROMMap[256];
extern unsigned long *VRAM2,*VRAMS;
extern byte RAM[0x8000];
extern M6502 M;

void loadRom() __attribute__((used,
	annotate("as3import:flash.utils.ByteArray"),
	annotate("as3sig:public function loadRom(num:int, bytes:ByteArray):void")));

void nextFrame() __attribute__((used,
	annotate("as3sig:public function nextFrame():void")));

void getBytes() __attribute__((used,
	annotate("as3import:flash.utils.ByteArray"),
	annotate("as3sig:public function getBytes():ByteArray")));

void getSoundBytes() __attribute__((used,
	annotate("as3import:flash.utils.ByteArray"),
	annotate("as3sig:public function getSoundBytes():ByteArray")));

void setKeys() __attribute__((used,
	annotate("as3sig:public function setKeys(down:int, up:int, left:int, right:int, start:int, select:int, b:int, a:int):void")));

int InitSound(int Mode);
BOOL InitMachine(void);
void TrashMachine(void);
void PutImage(int X, int Y, int W, int H);
int JoySticks(void);
void ResetPCE(M6502 *M);
void WriteSoundData(WORD *buf, int ch, DWORD dwSize);

int initPCE()
{
	int i,ROMmask;
	//cart_name = name;
	//if (CartLoad(name)) return -1;
#define	VRAMSIZE	0x20000

	DMYROM=(BYTE *)malloc(0x2000);
	memset(DMYROM,NODATA,0x2000);
	WRAM=(BYTE *)malloc(0x2000);
	memset(WRAM,0,0x2000);
	VRAM=(BYTE *)malloc(VRAMSIZE);
	VRAM2=(unsigned long *)malloc(VRAMSIZE);
	VRAMS=(unsigned long *)malloc(VRAMSIZE);
	//memset(VRAM,0,VRAMSIZE);
	IOAREA=(BYTE *)malloc(0x2000);
	memset(IOAREA,0xFF,0x2000);
	vchange = (BYTE *)malloc(VRAMSIZE/32);
	memset(vchange,1,VRAMSIZE/32);
	vchanges = (BYTE *)malloc(VRAMSIZE/128);
	memset(vchanges,1,VRAMSIZE/128);
	ROMmask = 1;
	while(ROMmask<ROM_size) ROMmask<<=1;
	ROMmask--;
	printf("ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);
	for(i=0;i<0xF7;i++)
	{
		if (ROM_size == 0x30)
		{
			switch (i&0x70)
			{
			case 0x00:
			case 0x10:
			case 0x50:
				ROMMap[i]=ROM+(i&ROMmask)*0x2000;
				break;
			case 0x20:
			case 0x60:
				ROMMap[i]=ROM+((i-0x20)&ROMmask)*0x2000;
				break;
			case 0x30:
			case 0x70:
				ROMMap[i]=ROM+((i-0x10)&ROMmask)*0x2000;
				break;
			case 0x40:
				ROMMap[i]=ROM+((i-0x20)&ROMmask)*0x2000;
				break;
			}
		}
		else
			ROMMap[i]=ROM+(i&ROMmask)*0x2000;
	}

/*
	if (populus)
	{
		ROMMap[0x40] = PopRAM + (0)*0x2000;
		ROMMap[0x41] = PopRAM + (1)*0x2000;
		ROMMap[0x42] = PopRAM + (2)*0x2000;
		ROMMap[0x43] = PopRAM + (3)*0x2000;
	}
*/
	
	ROMMap[0xF7]=WRAM;
	ROMMap[0xF8]=RAM;
	ROMMap[0xF9]=RAM+0x2000;
	ROMMap[0xFA]=RAM+0x4000;
	ROMMap[0xFB]=RAM+0x6000;
	ROMMap[0xFF]=IOAREA; //NULL; /* NULL = I/O area */

/*
	FILE *fp;
	fp = fopen(backmemname, "rb");
	if (fp == NULL) {
		//LogDump("Can't open %s\n", backmemname);
		printf("Can't open %s\n", backmemname);
	} else {
		fread(WRAM, 0x2000, 1, fp);
		fclose(fp);
	}
*/
	
	return 0;
}


void loadRom() {
    inline_as3("%0 = bytes.bytesAvailable;" : "=r"(ROM_size));
    ROM = (unsigned char *)malloc(ROM_size);
	inline_as3("CModule.writeBytes(%0, bytes.bytesAvailable, bytes);" :: "r"(ROM));
	
	printf("ROM_size: %d\n", ROM_size);
	printf("ROM[0]: %02x\n", ROM[0]);
	
	initPCE();
	ResetPCE(&M);
}

void nextFrame() {
	Run6502(&M);
}

void getBytes() {
	inline_as3(
		"var byteArray:ByteArray = new ByteArray();\n"
		//"byteArray.endian = 'littleEndian';\n"
		"CModule.readBytes(%0, %1, byteArray);\n"
		"return byteArray;\n" :: "r"(bytes), "r"(WIDTH * HEIGHT * 4)
	);
}

void getSoundBytes() {
	//printf("getSoundBytes: \n");
	int size = 2048;
	
	memset(soundBytes, 0, size * 4);
	for (int i = 0; i < 6; i++) {
		WriteSoundData((WORD*)soundBytesWork, i, size * 2);
		for (int n = 0; n < size * 2; n++) {
			*(soundBytes + n) += *(soundBytesWork + n);
		}
	}
	inline_as3(
		"var byteArray:ByteArray = new ByteArray();\n"
		"CModule.readBytes(%0, %1, byteArray);\n"
		"return byteArray;\n" :: "r"(soundBytes), "r"(size * 4)
	);
}

void setKeys() {
	int down = 0, up = 0, left = 0, right = 0;
	int a = 0, b = 0, select = 0, start = 0;
	
	inline_as3(
		"%0 = down;\n"
		"%1 = up;\n"
		"%2 = left;\n"
		"%3 = right;\n"
		"%4 = a;\n"
		"%5 = b;\n"
		"%6 = select;\n"
		"%7 = start;\n"
		: 
		"=r"(down),
		"=r"(up),
		"=r"(left),
		"=r"(right),
		"=r"(a),
		"=r"(b),
		"=r"(select), 
		"=r"(start)
	); 
	
	if (start > 0) {
		start = 1;
	}
	
	keys = 0;
	keys |=	((up & 1) << 4) | 
			((right & 1) << 5) | 
			((down & 1) << 6) |
			((left & 1) << 7) | 
			((start & 1) << 3) | 
			((select & 1) << 2) |
			((b & 1) << 1) |
			(a & 1);
}

BOOL InitMachine(void)
{
	snd_bSound = 1;
	XBuf = (unsigned char*)malloc(sizeof(char) * WIDTH * HEIGHT);
	if (!XBuf) {
		return 0;
	}
	
#ifdef SOUND
	InitSound(snd_bSound);
#endif // SOUND

#ifdef USE_JOYSTICK
	InitDInput();
#endif // USE_JOYSTICK

	//InitInfo();

	bytes = (unsigned char*)malloc(WIDTH * HEIGHT * 4);
	soundBytes = (short*)malloc(2048 * 4);
	soundBytesWork = (short*)malloc(2048 * 4);
	
	for (int n = 0; n < 256; n++) {
		PalTable[n] = 0xFF000000;
		PalTable[n] |= (((n >> 2) & 7) << 21);
		PalTable[n] |= ((n >> 5) << 13);
		PalTable[n] |= ((n & 3) << 6);
	}

	return 1;
}


void PutImage(int X, int Y, int W, int H)
{
	//printf("PutImage: %d, %d, %d, %d\n", X, Y, W, H);
	
	unsigned char* s = XBuf;
	unsigned int* d = (unsigned int*)bytes;
	
	for (int i = Y; i < Y + H; i++) {
		s = (unsigned char*)(XBuf + i * WIDTH + X);
		for (int j = X; j < X + W; j++) {
			int index = (i - Y) * W + (j - X);
			*(d + index) = PalTable[*s++];
		}
	}
}

int Joysticks(void)
{
	return keys;
	int JS = 0;

/*
	GetKeyboardState(aKeyState);

#define	KEY(__a)	(aKeyState[__a]&0x80)

	JS = 0x0000;

	static int	oldKeyA, oldKeyS;
//	static int	isRensyaA, isRensyaS;
	static int	press_countA, press_countS;
	static int	oldKey1, oldKey2;
	static int	oldKeyF5, oldKeyF6;
	extern byte BGONSwitch, SPONSwitch;
	static int oldKeyAltEnter;
	static int oldKeyF8;
	static int oldKeyF7;
	static int oldKeyF3, oldKeyF4, oldKeyF2;
	static int down, left;
//	int		KeyA, KeyS;
	int		Key1, Key2;
	int		KeyF5, KeyF6;
	int		KeyAltEnter;
	int		KeyF8;
	int		KeyF7;
	int		KeyF3, KeyF4, KeyF2;
	DWORD	ev;
	BOOL	bRensyaA = FALSE;
	BOOL	bRensyaB = FALSE;
	BOOL	bFrameSkipChanged = FALSE;
	char	szBuf[256];

	if (WAIT_OBJECT_0 == WaitForSingleObject(hEventNotify, 0))
		ev = dwEvent;
	else
		ev = 0;

	if (ev & EV_CARTLOAD)
	{
		if (0 ==_access(pCartName, 00))
		{
			cart_reload = 1;
			return 0x10000;
		}
	}

#ifdef USE_JOYSTICK
	if (lpJoystick)
	{
		HRESULT	diret;
		DIJOYSTATE	js;

		lpJoystick->Poll();
		diret = lpJoystick->GetDeviceState(sizeof(DIJOYSTATE), &js);
		if (FAILED(diret))
		{
			if (bActiveApp && (diret == DIERR_INPUTLOST || diret == DIERR_NOTACQUIRED))
			{
				if (!AcquireInput())
					goto JOYSTICK_ERROR_BREAK;
				lpJoystick->Poll();
				diret = lpJoystick->GetDeviceState(sizeof(DIJOYSTATE), &js);
			}
			if (FAILED(diret))
			{
			/*	TCHAR	szBuf[1024];
				wsprintf(szBuf, "GetDeviceState failed (code=%d)\n", diret);
				//TRACE("GetDeviceState failed\n");
				TRACE(szBuf);
				AfxMessageBox(szBuf);
			* /	goto JOYSTICK_ERROR_BREAK;
			}
		}
		if (js.lX < 0)
			JS |= JOY_LEFT;
		else if (js.lX > 0)
			JS |= JOY_RIGHT;
		if (js.lY < 0)
			JS |= JOY_UP;
		else if (js.lY > 0)
			JS |= JOY_DOWN;
		if (js.rgbButtons[ksJoy1] & 0x80)
			JS |= JOY_A;
		if (js.rgbButtons[ksJoy2] & 0x80)
			JS |= JOY_B;
		if (js.rgbButtons[ksJoy11] & 0x80)
			bRensyaA = TRUE;
		if (js.rgbButtons[ksJoy22] & 0x80)
			bRensyaB = TRUE;
		if (js.rgbButtons[ksJoyRun] & 0x80)
			JS |= JOY_START;
		if (js.rgbButtons[ksJoySel] & 0x80)
			JS |= JOY_SELECT;
JOYSTICK_ERROR_BREAK:
		;
	}
#endif // USE_JOYSTICK

	KeyAltEnter = (KEY(VK_MENU) && KEY(VK_RETURN)) ? 1 : 0;
	if (KeyAltEnter && oldKeyAltEnter == 0 || (ev & EV_FULLSCREEN))
	{
		PostMessage(hwnd, WM_CHANGEWINDOWMODE, 0, 0);
		WaitForSingleObject(hEventEndCommand, INFINITE);
	}
	oldKeyAltEnter = KeyAltEnter;

	KeyF8 = KEY(VK_F8) ? 1 : 0;
	if (KeyF8 && oldKeyF8 == 0 || (ev & EV_ZOOM))
		ZoomWindow();
	oldKeyF8 = KeyF8;

	KeyF7 = KEY(VK_F7) ? 1 : 0;
	if (KeyF7 && oldKeyF7 == 0)
		bShowFps = !bShowFps;
	oldKeyF7 = KeyF7;
/*
	KeyA = KEY('A');
	if (KeyA && oldKeyA == 0)
		isRensyaA = 1 - isRensyaA;
	oldKeyA = KeyA ? 1 : 0;
	KeyS = KEY('S');
	if (KeyS && oldKeyS == 0)
		isRensyaS = 1 - isRensyaS;
	oldKeyS = KeyS ? 1 : 0;
* /
	if(KEY(ksKey11) || bRensyaA)
	{
		if (press_countA++ >= 2)
		{
			JS |= JOY_A;
			press_countA = 0;
		}
	}
	if(KEY(ksKey22) || bRensyaB)
	{
		if (press_countS++ >= 2)
		{
			JS |= JOY_B;
			press_countS = 0;
		}
	}

	if(KEY(VK_ESCAPE) || KEY(VK_MENU)&&KEY(VK_F4) || KEY(VK_F10) || (ev & EV_STOP))
		JS|=0x10000;
	if(KEY(ksKey1))
		JS|=JOY_A;
	if(KEY(ksKey2))
		JS |= JOY_B;
	if(KEY(ksKeyRun) && !KeyAltEnter)	JS |= JOY_START;
	if(KEY(ksKeySel))		JS |= JOY_SELECT;
	if(KEY(ksKeyDown))	JS |= JOY_DOWN;
	if(KEY(ksKeyUp))		JS |= JOY_UP;
	if ((JS&(JOY_DOWN|JOY_UP))==(JOY_DOWN|JOY_UP))
	{
		if (down)
			JS &= ~JOY_DOWN;
		else
			JS &= ~JOY_UP;
	}
	else
		down = JS&JOY_DOWN;
	if(KEY(ksKeyLeft))	JS |= JOY_LEFT;
	if(KEY(ksKeyRight))	JS |= JOY_RIGHT;
	if ((JS&(JOY_LEFT|JOY_RIGHT))==(JOY_LEFT|JOY_RIGHT))
	{
		if (left)
			JS &= ~JOY_LEFT;
		else
			JS &= ~JOY_RIGHT;
	}
	else
		left = JS&JOY_LEFT;

	Key1 = KEY('1');
	if(Key1 && oldKey1 == 0)
		BGONSwitch = 1 - BGONSwitch;
	oldKey1 = Key1 ? 1 : 0;
	Key2 = KEY('2');
	if(Key2 && oldKey2 == 0)
		SPONSwitch = 1 - SPONSwitch;
	oldKey2 = Key2 ? 1 : 0;

	KeyF5 = KEY(VK_F5);
	if (KeyF5 && oldKeyF5 == 0 || (ev & EV_SAVE))
		SaveState();
	oldKeyF5 = KeyF5 ? 1 : 0;
	KeyF6 = KEY(VK_F6);
	if (KeyF6 && oldKeyF6 == 0 || (ev & EV_LOAD))
	LoadState();
	oldKeyF6 = KeyF6 ? 1 : 0;

	KeyF3 = KEY(VK_F3);
	if (KeyF3 && oldKeyF3 == 0 || (ev & EV_DOWNSKIPRATE))
	{
		if (UPeriod > 0)
			UPeriod--;
		bFrameSkipChanged = TRUE;
	}
	oldKeyF3 = KeyF3 ? 1 : 0;
	KeyF4 = KEY(VK_F4);
	if (KeyF4 && oldKeyF4 == 0 || (ev & EV_UPSKIPRATE))
	{
		if (UPeriod < 15)
			UPeriod++;
		bFrameSkipChanged = TRUE;
	}
	oldKeyF4 = KeyF4 ? 1 : 0;
	KeyF2 = KEY(VK_F2);
	if (KeyF2 && oldKeyF2 == 0 || (ev & EV_AUTOFRAMESKIP))
	{
		bAutoFrameSkip = !bAutoFrameSkip;
		bFrameSkipChanged = TRUE;
	}
	oldKeyF2 = KeyF2 ? 1 : 0;

	if (bFrameSkipChanged)
	{
		if (bAutoFrameSkip)
			wsprintf(szBuf, "FrameSkip:%d(auto)", UPeriod);
		else
			wsprintf(szBuf, "FrameSkip:%d", UPeriod);
		SetInfoString(szBuf);
	}

	if (ev & EV_RESET)
		ResetPCE(&M);
*/
	return JS;
}
