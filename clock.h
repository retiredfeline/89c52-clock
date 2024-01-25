typedef	unsigned char	uchar;
typedef unsigned int	uint;

#define	TICKSINHALFSEC	(TICKSINSEC/2)
#define	TICK		4			// ms, roughly
#define	DEPMIN		(100 / TICK)		// debounce period
#define	RPTTHRESH	((400 / TICK) + 1)	// repeat threshold after debounce
#define	RPTPERIOD	(250 / TICK)		// repeat period

#ifdef	STC89C52
#include "stc89c52.h"
#endif	// STC89C52

#ifdef	AVR_AT90S8515
#include "at90s8515.h"
#endif	// AVR_AT90S8515
