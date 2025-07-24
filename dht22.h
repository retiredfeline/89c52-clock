typedef struct dhtresult {
	uint	humidity;
	int	temperature;
	uchar	invalid;
} dhtresult;
extern void dht22_init(void);
extern void int2bcd(int, char *);
extern void getdht22data(dhtresult *);
