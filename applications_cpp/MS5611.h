#ifndef MS5611_H
#define MS5611_h
#include "stm32f4xx.h"
#include "Sensor.h"

#define SEA_PRESS 1013.25f

#define MS5611_NAME "MS5611"

#define  MS561101BA_SlaveAddress 0x77  //����������IIC�����еĴӵ�ַ
#define  MS561101BA_D1 0x40 
#define  MS561101BA_D2 0x50 
#define  MS561101BA_RST 0x1E 

#define  MS561101BA_D1_OSR_256 0x40 
#define  MS561101BA_D1_OSR_512 0x42 
#define  MS561101BA_D1_OSR_1024 0x44 
#define  MS561101BA_D1_OSR_2048 0x46 
#define  MS561101BA_D1_OSR_4096 0x48 

#define  MS561101BA_D2_OSR_256 0x50 
#define  MS561101BA_D2_OSR_512 0x52 
#define  MS561101BA_D2_OSR_1024 0x54 
#define  MS561101BA_D2_OSR_2048 0x56 
#define  MS561101BA_D2_OSR_4096 0x58 

#define  MS561101BA_PROM_RD 0xA2

class MS5611:public Sensor
{
public:
	static MS5611* getInstance()
	{
		return instance;
	}
	virtual ~MS5611();

	virtual bool initialize(void);
	virtual bool testConnection(void);
	virtual uint8_t getData(void* data1,void* data2 = null,void* data3 = null,void* data4 = null,void* data5 = null,void* data6 = null);

	bool getPressure(float &press);
	bool getTemperature(float &temp);
	bool getPressure();
	bool getTemperature();
	bool getAltitude(float &altitude);

	void ConvertPressure();
	void ConvertTemperature();
	void setGround(void);
	bool readPROM(void);
	bool reset();
private:
	uint16_t C[6];
	int32_t dT;
	int32_t temperature,pressure,groundPress;
	static MS5611* const instance;
	MS5611();
};


#endif
