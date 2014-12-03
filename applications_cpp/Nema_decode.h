#ifndef __NEMA_DECODE_H
#define __NEMA_DECODE_H

#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "rtthread.h"
#include "stm32f4xx.h"

enum GPSDATA_TYPE{
 GPGGA=1,   //ȫ��λ���� 
 GPGSA=2,	//����PRN����  
 GPGSV=3,	//����״̬��Ϣ
 GPRMC=4,	//�Ƽ���С���� 
 GPVTG=5,   //�����ٶ���Ϣ
 GPGLL=6,   //���������Ϣ 
 GPZDA=7	//UTCʱ�������
};

typedef struct UTC{
	int year;
	int mon;
	int day;
	int hour;
	int minute;
	double second;
}UTC_TIME;

typedef struct GPS_GPGGA {
		UTC_TIME	Utc_Time;   		//��������ʱ�� 
		int 	  	Latitude;			// γ��ֵ
		char 		Latitude_Directon;  // ��γ����γ 
		int		  	Longitude; 			//  ����ֵ
		char 		Longitude_Direction;//���������� 
		int 		Location_Quality;  	//��λ����  0����Ч 1����׼ 2����� 6������ 
		int 		Satelite_Quantity; 	//ʹ��������
		double 		Level_Accuracy;  	//ˮƽ��ȷ�� 0.5-99.9 
		double 		Sea_Dials;			//�����뺣ƽ��߶�
		char 		UnitOfHeight1;		// �߶ȵ�λ
		double 		Relative_Height;	//�����������Ժ�ƽ��߶�
		char 		UnitOfHeight2;		// �߶ȵ�λ
		int			RTCM;				// ���GPS�������ޣ��������RTCM ���������� 
		int 		BaseStation_Number;	//��ֲο���վ��� 
		int 		Sum;				//  У��� 
}GPS_GPGGA;

typedef struct GPGSA{
	int l;
}GPS_GPGSA;

typedef struct GPGSV{
	int num;
}GPS_GPGSV;

typedef struct GPRMC{
	int num;
}GPS_GPRMC;

typedef struct GPVTG{
	int num;
}GPS_GPVTG;

typedef struct GPGLL{
	int num;
}GPS_GPGLL;

typedef struct GPZDA{
	int num;
}GPS_GPZDA;


class GPS_GetData{
	public:
		GPS_GPGGA Gpgga;
		rt_device_t uart_device;
	public:
		GPS_GetData(){}
	    GPS_GetData(rt_device_t& Uart_device);
		~GPS_GetData();
		void Get_Gps_GPGGA(void);
		void Get_Gps_GPGLL(void);
		void Get_Gps_GPGSV(void);
		void Get_Gps_GPRMC(void);
		void Get_Gps_GPVTG(void);
		void Get_Gps_GPGSA(void);
		void Get_Gps_GPZDA(void);
		int  Get_DataType(char* data); 
		inline	char Get_next(void);
		int GPS_coord_to_degrees(char* s);
		GPS_GPGGA  Nema_decode_gpgga();
		void Get_Coor(int32_t* lng,int32_t* lat);
		void Get_Altitude(int32_t &alt0);
		void Get_Speed(int32_t &speed);
		void Get_GPS_Direction(char &lng_dir,char &lat_dir); //�õ�N/S �� E/W
};

void rt_thread_entry_getgpsdata(void* parameter);
#endif
