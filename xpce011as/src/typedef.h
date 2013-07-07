#ifndef	__TYPEDEF_INCLUDED__
#define	__TYPEDEF_INCLUDED__

#include <inttypes.h>

typedef	int		BOOL;

typedef	unsigned char	BYTE;
typedef	unsigned short	WORD;
typedef	unsigned long	DWORD;
typedef	uint64_t QWORD;

typedef	unsigned char	UBYTE;
typedef	unsigned short	UWORD;
typedef	unsigned long	ULONG;
typedef	unsigned long	UDWORD;
typedef	uint64_t UQWORD;

typedef	signed char	SBYTE;
typedef	signed short	SWORD;
typedef	signed long	SLONG;
typedef	signed long	SDWORD;
typedef	int64_t	SQWORD;

typedef	signed int	INT, *LPINT;
typedef	unsigned int	UINT;
typedef	signed short	SHORT;
typedef	signed long	LONG;

typedef	BYTE		*LPBYTE;
typedef	WORD		*LPWORD;
typedef	DWORD		*LPDWORD;

typedef	SBYTE		*LPSBYTE;
typedef	SWORD		*LPSWORD;
typedef	SLONG		*LPSLONG;
typedef	SDWORD		*LPSDWORD;

typedef	UBYTE		*LPUBYTE;
typedef	UWORD		*LPUWORD;
typedef	ULONG		*LPULONG;
typedef	UDWORD		*LPUDWORD;

typedef	SHORT		*LPSHORT;
typedef	LONG		*LPLONG;

#define FALSE 0
#define TRUE 1

typedef void VOID, *PVOID;
typedef char CHAR;
typedef unsigned char UCHAR, *PUCHAR;
typedef float FLOAT;
//typedef size_t SIZE_T;

#endif // __TYPEDEF_INCLUDED__
