#define	ADDR	0x68U
#define	SDA	P1_2
#define	SCL	P1_3

extern void i2cinit(void);
extern void i2cstart(void);
extern void i2crestart(void);
extern void i2cstop(void);
extern void i2cack(void);
extern void i2cnak(void);
extern unsigned char i2csendaddr(void);
extern unsigned char i2creadaddr(void);
extern unsigned char i2csend(unsigned char);
extern unsigned char i2cread(void);
