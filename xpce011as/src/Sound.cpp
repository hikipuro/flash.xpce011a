//#include "stdafx.h"
#include "config.h"
#include "typedef.h"
#ifdef SOUND
#include <stdio.h>
#include <memory.h>
//#include "ds.h"
#include "pce.h"

#define max(a, b)	((a) > (b) ? (a) : (b))
#define min(a, b)	((a) < (b) ? (a) : (b))

extern IO	io;
extern M6502	M;

extern DWORD	snd_dwSampleRate;
//extern DWORD	snd_dwBits;
extern DWORD	snd_dwChannels;
extern int		snd_CoopMode;
extern int		snd_Latency;

extern int		ds_nSampleRate;
extern int		ds_nLockLatency;
extern int		ds_nChannels;
extern int		ds_CoopMode;

//static CRITICAL_SECTION	cs;
static BOOL bInitialized = FALSE;
static void WriteBuffer(WORD *buf, int ch, DWORD dwSize);

int InitSound(int Mode)
    /* Returns 1 on success, 0 otherwise. Mode=0 to skip     */
    /* initialization (will be silent), Mode=1 to use Adlib, */
    /* Mode=2..7 to use SoundBlaster wave synthesis.         */
{
	//InitializeCriticalSection(&cs);

	printf("InitSound: %d\n", Mode);
	
	if (Mode == 0)
		return TRUE;

	BOOL	bRet = TRUE;

	ds_nSampleRate = snd_dwSampleRate;
//	ds_nBits = snd_dwBits;
	ds_nChannels = snd_dwChannels;
	ds_CoopMode = snd_CoopMode;
	ds_nLockLatency = snd_Latency;
	//bRet = InitDSound(AfxGetMainWnd()->GetSafeHwnd());

	if (bRet)
	{
		bInitialized = TRUE;
	}

	return bRet;
}

void TrashSound(void)                   /* Shut down sound  */
{
	//DeleteCriticalSection(&cs);

	if (!bInitialized)
		return;
	bInitialized = FALSE;

	//StopSound();
	//ReleaseDSound();
}

#define SOUND_BUF_MS	200
DWORD	dwOldPos[6];
WORD	sbuf[6][44100*SOUND_BUF_MS/1000*sizeof(WORD)*2];
DWORD	CycleOld;
extern int	BaseClock;

void write_psg(int ch)
{
	//printf("write_psg: %d\n", ch);
	DWORD	Cycle;
	DWORD	dwNewPos;

	//printf("write_psg: %d\n", bInitialized);
	if (!bInitialized)
		return;

	//EnterCriticalSection(&cs);
	if ((int)((DWORD)M.User - CycleOld) < 0)
		CycleOld = (DWORD)M.User;
	Cycle = (DWORD)M.User - CycleOld;
	dwNewPos = (DWORD)ds_nSampleRate/25*(Cycle/2)/(BaseClock/50);
	if (dwNewPos > (DWORD)ds_nSampleRate*SOUND_BUF_MS/1000)
	{
//		TRACE("sound buffer overrun\n");
		dwNewPos = ds_nSampleRate*SOUND_BUF_MS*3/4/1000;
	}
	
	//printf("write_psg: %d, %d\n", dwNewPos, dwOldPos[ch]);
	if (dwNewPos > dwOldPos[ch])
	{
		WriteBuffer(&sbuf[ch][dwOldPos[ch]*ds_nChannels], ch, (dwNewPos-dwOldPos[ch])*ds_nChannels);
		dwOldPos[ch] = dwNewPos;
	}
	//LeaveCriticalSection(&cs);
}

static int mseq(DWORD *rand_val)
{
	if (*rand_val & 0x00080000)
	{
		*rand_val = ((*rand_val ^ 0x0004) << 1) + 1;
		return 1;
	}
	else
	{
		*rand_val <<= 1;
		return 0;
	}
}

void WriteSoundData(WORD *buf, int ch, DWORD dwSize)
{
	//printf("WriteSoundData: %d, %d\n", ch, dwSize);
	DWORD	dwNewPos;

	//EnterCriticalSection(&cs);
	dwNewPos = dwSize/ds_nChannels;
	if (dwOldPos[ch] < dwNewPos)
		WriteBuffer(&sbuf[ch][dwOldPos[ch]*ds_nChannels], ch, (dwNewPos-dwOldPos[ch])*ds_nChannels);
	CycleOld = (DWORD)M.User;
	memcpy(buf, sbuf[ch], dwSize*sizeof(WORD));
	if (dwOldPos[ch] >= dwNewPos)
	{
		if (dwOldPos[ch] >= (DWORD)ds_nSampleRate*SOUND_BUF_MS/(1000*100/95))
		{
			DWORD	size = ds_nSampleRate*SOUND_BUF_MS/4/1000;
			memcpy(sbuf[ch], sbuf[ch]+dwNewPos*ds_nChannels, size*sizeof(WORD)*ds_nChannels);
			dwOldPos[ch] = size;
		}
		else
		{
			memcpy(sbuf[ch], sbuf[ch]+dwNewPos*ds_nChannels, (dwOldPos[ch]-dwNewPos)*sizeof(WORD)*ds_nChannels);
			dwOldPos[ch] = dwOldPos[ch]-dwNewPos;
		}
	}
	else
		dwOldPos[ch] = 0;
	//LeaveCriticalSection(&cs);
}
/*
#ifdef _DEBUG
void debug_dump(char *str, ...)
{
	va_list	arg;
	va_start(arg, str);
	FILE *fp = fopen("c:\\sound.dmp", "a+");
	vfprintf(fp, str, arg);
	fclose(fp);
	va_end(arg);
}
#endif
*/

void WriteBuffer(WORD *buf, int ch, DWORD dwSize)
{
	//printf("WriteBuffer: %d, %d\n", ch, dwSize);
//#ifdef _DEBUG
//	if (ch == 0)
//		debug_dump("%08X-%08X\n", buf-sbuf[ch], buf-sbuf[ch]+dwSize);
//#endif
	static DWORD	n[6] = {0,0,0,0,0,0};
#define	N	32
	static DWORD	k[6] = {0,0,0,0,0,0};
	static DWORD	t;
//	static BOOL		bInit = TRUE;
	static DWORD	r[6];
	static DWORD	rand_val[6] = {0,0,0,0,0x51F631E4,0x51F631E4};
	static WORD		wave[32];
	/* a*2^(b*x) */
	/* a = 400, b = log2(16384/a), x = 0.0 ~ 1.0 */
	static int		vol_tbl[32] =
	{
		100,451,508,573,646,728,821,925,
		1043,1175,1325,1493,1683,1898,2139,2411,
		2718,3064,3454,3893,4388,4947,5576,6285,
		7085,7986,9002,10148,11439,12894,14535,16384,
	};
	DWORD	dwPos;
	int		lvol, rvol;
	DWORD	Tp;

	if (!(io.PSG[ch][4]&0x80))
	{
		n[ch] = k[ch] = 0;
		for (dwPos = 0; dwPos < dwSize; dwPos++)
			*buf++ = 0;
		return;
	}

//	TRACE("buf=%08X,size=%08X\n", buf, dwSize);

	if (io.PSG[ch][4]&0x40)
	{
//		TRACE("wave=%d\n", io.wave[ch][0]);
		wave[0] = ((short)io.wave[ch][0]-16)*702;
		if (ds_nChannels == 2)
		{
			lvol = ((io.psg_volume>>3)&0x1E) + (io.PSG[ch][4]&0x1F) + ((io.PSG[ch][5]>>3)&0x1E);
			lvol = lvol-60;
			if (lvol < 0) lvol = 0;
			lvol = (int)(short)wave[0]*vol_tbl[lvol]/16384;
			rvol = ((io.psg_volume<<1)&0x1E) + (io.PSG[ch][4]&0x1F) + ((io.PSG[ch][5]<<1)&0x1E);
			rvol = rvol-60;
			if (rvol < 0) rvol = 0;
			rvol = (int)(short)wave[0]*vol_tbl[rvol]/16384;
			for (dwPos = 0; dwPos < dwSize; dwPos += 2)
			{
				*buf++ = (WORD)lvol;
				*buf++ = (WORD)rvol;
			}
		}
		else
		{
			lvol = max((io.psg_volume>>3)&0x1E, (io.psg_volume<<1)&0x1E) + (io.PSG[ch][4]&0x1F) +
						max((io.PSG[ch][5]>>3)&0x1E, (io.PSG[ch][5]<<1)&0x1E);
			lvol = lvol-60;
			if (lvol < 0) lvol = 0;
			lvol = (int)(short)wave[0]*vol_tbl[lvol]/16384;
			for (dwPos = 0; dwPos < dwSize; dwPos++)
			{
				*buf++ = (WORD)lvol;
			}
		}
	}
	else
	{
//for (dwPos = 0; dwPos < dwSize; dwPos++)
//	*buf++ = 0;
//return;
		if (ch >= 4 && (io.PSG[ch][7]&0x80))
		{
			DWORD Np = (io.PSG[ch][7]&0x1F);
			if (ds_nChannels == 2)
			{
				lvol = ((io.psg_volume>>3)&0x1E) + (io.PSG[ch][4]&0x1F) + ((io.PSG[ch][5]>>3)&0x1E);
				lvol = lvol-60;
				if (lvol < 0) lvol = 0;
				lvol = vol_tbl[lvol];
				rvol = ((io.psg_volume<<1)&0x1E) + (io.PSG[ch][4]&0x1F) + ((io.PSG[ch][5]<<1)&0x1E);
				rvol = rvol-60;
				if (rvol < 0) rvol = 0;
				rvol = vol_tbl[rvol];
				for (dwPos = 0; dwPos < dwSize; dwPos += 2)
				{
					k[ch] += 3000+Np*512;
					t = k[ch] / (DWORD)ds_nSampleRate;
					if (t >= 1)
					{
						r[ch] = mseq(&rand_val[ch]);
						k[ch] -= ds_nSampleRate*t;
					}
					*buf++ = (WORD)((r[ch] ? 10*702 : -10*702)*lvol/16384);
					*buf++ = (WORD)((r[ch] ? 10*702 : -10*702)*rvol/16384);
				}
			}
			else
			{
				lvol = max((io.psg_volume>>3)&0x1E, (io.psg_volume<<1)&0x1E) + (io.PSG[ch][4]&0x1F) +
							max((io.PSG[ch][5]>>3)&0x1E, (io.PSG[ch][5]<<1)&0x1E);
				lvol = lvol-60;
				if (lvol < 0) lvol = 0;
				lvol = vol_tbl[lvol];
				for (dwPos = 0; dwPos < dwSize; dwPos++)
				{
					k[ch] += 3000+Np*512;
					t = k[ch] / (DWORD)ds_nSampleRate;
					if (t >= 1)
					{
						r[ch] = mseq(&rand_val[ch]);
						k[ch] -= ds_nSampleRate*t;
					}
					*buf++ = (WORD)((r[ch] ? 10*702 : -10*702)*lvol/16384);
				}
			}
			return;
		}

		for (int i = 0; i < 32; i++)
			wave[i] = ((short)io.wave[ch][i]-16)*702;

		Tp = io.PSG[ch][2]+((DWORD)io.PSG[ch][3]<<8);
		if (Tp == 0)
		{
			for (dwPos = 0; dwPos < dwSize; dwPos++)
				*buf++ = 0;
			return;
		}

		if (ds_nChannels == 2)
		{
			lvol = ((io.psg_volume>>3)&0x1E) + (io.PSG[ch][4]&0x1F) + ((io.PSG[ch][5]>>3)&0x1E);
			lvol = lvol-60;
			if (lvol < 0) lvol = 0;
			lvol = vol_tbl[lvol];
			rvol = ((io.psg_volume<<1)&0x1E) + (io.PSG[ch][4]&0x1F) + ((io.PSG[ch][5]<<1)&0x1E);
			rvol = rvol-60;
			if (rvol < 0) rvol = 0;
			rvol = vol_tbl[rvol];
			for (dwPos = 0; dwPos < dwSize; dwPos += 2)
			{
				*buf++ = (WORD)((int)(short)wave[n[ch]]*lvol/16384);
				*buf++ = (WORD)((int)(short)wave[n[ch]]*rvol/16384);
				k[ch] += N*1118608/Tp;
				t = k[ch] / (10*(DWORD)ds_nSampleRate);
				n[ch] = (n[ch]+t)%N;
				k[ch] -= 10*ds_nSampleRate*t;
			}
		}
		else
		{
			lvol = max((io.psg_volume>>3)&0x1E, (io.psg_volume<<1)&0x1E) + (io.PSG[ch][4]&0x1F) +
						max((io.PSG[ch][5]>>3)&0x1E, (io.PSG[ch][5]<<1)&0x1E);
			lvol = lvol-60;
			if (lvol < 0) lvol = 0;
			lvol = vol_tbl[lvol];
			for (dwPos = 0; dwPos < dwSize; dwPos++)
			{
				*buf++ = (WORD)((int)(short)wave[n[ch]]*lvol/16384);
				k[ch] += N*1118608/Tp;
				t = k[ch] / (10*(DWORD)ds_nSampleRate);
				n[ch] = (n[ch]+t)%N;
				k[ch] -= 10*ds_nSampleRate*t;
			}
		}
	}
}

#endif // SOUND
