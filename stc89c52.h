#ifndef STC89C52_H
#define STC89C52_H

/* Port mode bits */

/*  BYTE Register  */
__sfr __at (0x91) P1M1 ;
__sfr __at (0x92) P1M0 ;
__sfr __at (0x93) P0M1 ;
__sfr __at (0x94) P0M0 ;
__sfr __at (0x95) P2M1 ;
__sfr __at (0x96) P2M0 ;
__sfr __at (0xB1) P3M1 ;
__sfr __at (0xB2) P3M0 ;

#endif
