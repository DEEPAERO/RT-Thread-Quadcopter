#include "Nema_decode.h"
#include "rtthread.h"
#include "head.h"
#include "nmea/nmea.h"
#include "stdio.h"
#include "Attitude.h"
#include "ctype.h"

bool fixed = false;
int GPS_coord_to_degrees(char* s);

void rt_thread_entry_getgpsdata(void* parameter)
{
	uint8_t buffer[512] = {0};
	uint16_t len = 0;
	rt_device_t uart1_device = rt_device_find("uart1");

	if(uart1_device!=RT_NULL)
	{
		rt_device_open(uart1_device,RT_DEVICE_OFLAG_RDWR);
//		rt_kprintf("usart1 suu\n");
	}
//	else
//		rt_kprintf("uart fail\n");
		
    nmeaINFO info;          //GPS�����õ�����Ϣ
    nmeaPARSER parser;      //����ʱʹ�õ����ݽṹ  
    uint8_t new_parse=0;    //�Ƿ����µĽ������ݱ�־

    /* �����������������Ϣ�ĺ��� */
//    nmea_property()->trace_func = &trace;
//    nmea_property()->error_func = &error;

    /* ��ʼ��GPS���ݽṹ */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while(1)
    {
		len += rt_device_read(uart1_device,0,buffer+len,500-len);
		if(len > 128)
		{
			new_parse = nmea_parse(&parser, (const char*)&buffer[0], len, &info);
			len = 0;
		}
		if(new_parse )                //���µĽ�����Ϣ   
		{    
//			char str[100];

//			/* �������õ�����Ϣ */
//			sprintf(str,"\r\nʱ��%d,%d,%d,%d,%d,%d", info.utc.year+1900, info.utc.mon+1,info.utc.day,info.utc.hour,info.utc.min,info.utc.sec);
//			rt_kprintf(str);
//			sprintf(str,"\r\nγ�ȣ�%f,����%f",info.lat,info.lon);
//			rt_kprintf(str);
//			sprintf(str,"\r\n����ʹ�õ����ǣ�%d,�ɼ����ǣ�%d",info.satinfo.inuse,info.satinfo.inview);
//			rt_kprintf(str);
//			sprintf(str,"\r\n���θ߶ȣ�%f �� ", info.elv);
//			rt_kprintf(str);
//			sprintf(str,"\r\n�ٶȣ�%f km/h ", info.speed);
//			rt_kprintf(str);
//			sprintf(str,"\r\n����%f ��", info.direction);
//			rt_kprintf(str);

			if(info.fix == 2 || info.fix == 3)
			{
				char str[16];
				sprintf(str,"%f",info.lon);
				att.longitude = GPS_coord_to_degrees(str);
				sprintf(str,"%f",info.lat);
				att.latitude = GPS_coord_to_degrees(str);
				fixed = true;
			}
			else
				fixed = false;			
			new_parse = 0;
		}
		else
			fixed = false;
		DELAY_MS(200);
	}
}

int GPS_coord_to_degrees(char* s) {
	char *p, *q;
	short deg = 0, min = 0;
	unsigned int frac_min = 0;
	short i;

	// scan for decimal point or end of field
	for (p = s; isdigit(*p); p++) ;
	q = s;

	// convert degrees
	while ((p - q) > 2) {
		if (deg) deg *= 10;
		deg += ((*q++)-'0');
	}
	// convert minutes
	while (p > q) {
		if (min) min *= 10;
		min += ((*q++)-'0');
	}
	// convert fractional minutes
	// expect up to four digits, result is in
	// ten-thousandths of a minute
	if (*p == '.') {
		q = p + 1;
		for (i = 0; i < 5; i++) {
			frac_min *= 10;
			if (isdigit(*q))
				frac_min += *q++ - '0';
		}
	}
	return deg * 10000000UL + (min * 1000000UL + frac_min*10UL) / 6;
}
