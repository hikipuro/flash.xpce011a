// Debug.cpp

#include <stdio.h>
//#include "stdafx.h"
#include "M6502.h"
#include "typedef.h"

#ifdef _DEBUG

extern byte *Page[];
extern byte RAM[];

static BOOL CommandGo(M6502 *R);
static BOOL CommandExamine(M6502 *R);
static BOOL CommandBreak(M6502 *R);
static BOOL CommandDump(M6502 *R);
static BOOL CommandTrace(M6502 *R);
static int DebugPrompt(void);
static BOOL Buf_ReadWord(WORD *p);
static void PrintRegister(M6502 *R);
static char buf[1024];
static int buf_index;

BOOL DebugTrace(M6502 *R)
{
	BOOL	ret;
	int		i;

	//for (i = 0; i < 255; i++)
	//	GetAsyncKeyState(i);

	PrintRegister(R);
	do
	{
		ret = TRUE;
		switch (DebugPrompt())
		{
		case 'G':
			ret = CommandGo(R);
			break;

		case 'X':
			ret = CommandExamine(R);
			break;

		case 'B':
			ret = CommandBreak(R);
			break;

		case 'D':
			ret = CommandDump(R);
			break;

		case 'T':
			ret = CommandTrace(R);
			break;

		case 'Q':
			return FALSE;

//		default:
//			assert(FALSE);
//			return FALSE;
		}
	} while (ret);

	return TRUE;
}

BOOL CommandGo(M6502 *R)
{
	R->Trace = FALSE;
	return FALSE;
}

BOOL CommandExamine(M6502 *R)
{
	PrintRegister(R);
	return TRUE;
}

BOOL CommandBreak(M6502 *R)
{
	WORD	w;

	if (!Buf_ReadWord(&w))
		printf("Current break point is %04X\n", R->Trap);
	else
	{
		R->Trap = w;
		printf("Break point set at %04X\n", R->Trap);
	}
	return TRUE;
}

BOOL CommandDump(M6502 *R)
{
	static WORD	adr = 0;
	int		i, j;
	WORD	w;

	if (Buf_ReadWord(&w))
		adr = w;

	for (i = 0; i < 8; i++)
	{
		printf("%04X:", adr);
		for (j = 0; j < 8; j++)
		{
			printf(" %02X", Page[adr>>13][adr]);
			adr++;
		}
		printf("\n");
	}
	return TRUE;
}

BOOL CommandTrace(M6502 *R)
{
	return FALSE;
}

int DebugPrompt(void)
{
	static short KeyState[0x70];
	int		index = 0;
	int		i;

	printf(">");
	do
	{
		do
		{
			for (i = 0; i < 0x70; i++)
			{
				//KeyState[i] = GetAsyncKeyState(i);
				if (KeyState[i] & 1)
					break;
			}
		} while (i == 0x70);
		if (i >= 0x20)
		{
			buf[index++] = i;
			printf("%c", i);
		}
		/*
		if (i == VK_BACK)
		{
			if (index > 0)
			{
				index--;
				printf("\n>");
				for (i = 0; i < index; i++)
					printf("%c", buf[i]);
			}
		}
		*/
	} while(0); //while (i != VK_RETURN);
	buf[index] = 0;
	printf("\n");
	buf_index = 1;
	return buf[0];
}

BOOL Buf_ReadWord(WORD *p)
{
	WORD	num = 0;
	BOOL	valid = FALSE;

	/*
	while (isxdigit(buf[buf_index]))
	{
		num = num*16 + (buf[buf_index]>='A' ? buf[buf_index]-'A'+10 : buf[buf_index]-'0');
		valid = TRUE;
		buf_index++;
	}
	*/
	*p = num;
	return valid;
}

void PrintRegister(M6502 *R)
{
	int		i;

	printf("PC:%04X\n"
		  "A:%02X, P:%02X, X:%02X, Y:%02X, S:%02X\n"
		  "MPR:", R->PC.W, R->A, R->P&~(Z_FLAG|V_FLAG|N_FLAG)|(R->NF&N_FLAG)|(R->VF&V_FLAG)|(R->ZF? 0:Z_FLAG),
					R->X, R->Y, R->S);
	for (i = 0; i < 8; i++)
		printf(" %02X", R->MPR[i]);
	printf("\n");
}

#endif // _DEBUG
