typedef	unsigned char	uchar;
typedef unsigned int	uint;

#define	TICKSINHALFSEC	(TICKSINSEC/2)
#define	TICK		4			// ms, roughly
#define	DEPMIN		(50 / TICK)		// depress debounce period
#define	RELMIN		(100 / TICK)		// release debounce period
#define	RPTTHRESH	((400 / TICK) + 1)	// repeat threshold after debounce
#define	RPTPERIOD	(250 / TICK)		// repeat period
#define	BUTTON_TIMEOUT	(64000u / TICK)		// revert to Time mode after 64 seconds

#ifdef	STC89C52
#include "stc89c52.h"
#endif	// STC89C52
