#include "Nema_decode.h"
#include "rtthread.h"

void rt_thread_entry_getgpsdata(void* parameter){
	    int32_t lat,log;  //��γ������
		rt_device_t uart1_device = rt_device_find("uart1");
	
		if(uart1_device!=RT_NULL){
			rt_device_open(uart1_device,RT_DEVICE_OFLAG_RDWR);
			rt_kprintf("usart1 suu\n");
		}
		else
		   rt_kprintf("uart fail\n");
		
		GPS_GetData  gps_data(uart1_device); //��ʼ��ʱ�����豸����
		while(true){
			//�˺�����������GPS�����������ݣ�����1�Ǿ��ȣ�����2��γ��
			gps_data.Get_Coor(&log,&lat);
			rt_kprintf("the Longitude:%d  the latitude:%d\n",log,lat);
			rt_thread_delay(1000);
		}
}


GPS_GetData::GPS_GetData(rt_device_t& Uart_device){
			memset(&Gpgga,0,sizeof(GPS_GPGGA));
			uart_device=Uart_device;
}

GPS_GetData::~GPS_GetData(){}
	

GPS_GPGGA  GPS_GetData::Nema_decode_gpgga()
{
	
	char str[10];
	char cc=0;
	
	if(uart_device==NULL){
		memset(&Gpgga,0,sizeof(GPS_GPGGA));
		return Gpgga;
	}
	
	while((cc=Get_next())!=0)
	{
		switch(cc)
		{
		case '$':
			if(((cc=Get_next())=='G')&&((cc=Get_next())=='P'))
			{   
				memset(str,0,sizeof(str));
				while((cc=Get_next())!=','){
					strncat(str,&cc,1);
				}
				switch(Get_DataType(str)){
				case GPGGA:
					Get_Gps_GPGGA();
					return Gpgga;
				case GPGSA:
					Get_Gps_GPGSA();
					break;
				case GPGSV:
					Get_Gps_GPGSV();
					break;	  
				case GPRMC:
					Get_Gps_GPRMC();
					break;
				case GPVTG:
					Get_Gps_GPVTG();
					break;
				case GPGLL:
					Get_Gps_GPGLL();
					break;
				case GPZDA:
					Get_Gps_GPZDA();
					break;
				default:
					break;
				}
			}

			else{
				break;	
			}
		default:
			continue;
		}
	}
	return Gpgga;
}

inline char GPS_GetData::Get_next()
{  
	char ch;
	while(0 == rt_device_read(uart_device,0,&ch,1))
		rt_thread_delay(10);
	return ch;
}

void GPS_GetData::Get_Gps_GPGSV(void)
{

}
void GPS_GetData::Get_Gps_GPGLL(void)
{

}
void GPS_GetData::Get_Gps_GPRMC(void)
{

}
void GPS_GetData::Get_Gps_GPVTG(void)
{

}

void GPS_GetData::Get_Gps_GPGSA(void)
{

}
void GPS_GetData::Get_Gps_GPZDA(void){
	

}

void GPS_GetData::Get_Coor(int32_t* lng,int32_t* lat){
	Nema_decode_gpgga();
	*lng=Gpgga.Longitude;
	*lat=Gpgga.Latitude;	
}

void GPS_GetData::Get_Gps_GPGGA(void){
	char temp[15];
	char ch;
	unsigned int i=0;

	memset(&Gpgga,0,sizeof(Gpgga));

	//��ʱ�� hhmmss.sss,
	memset(temp,0,sizeof(temp));
	ch=Get_next();
	if(ch!=','){
		strncat(temp,&ch,1);
		ch=Get_next();
		strncat(temp,&ch,1);
		Gpgga.Utc_Time.hour=atoi(temp);	

		memset(temp,0,sizeof(temp));
		for(i=0;i<2;i++){
			ch=Get_next();
			strncat(temp,&ch,1);
		}
		Gpgga.Utc_Time.minute=atoi(temp);

		memset(temp,0,sizeof(temp));
		for(i=0;i<5;i++){
			ch=Get_next();
			strncat(temp,&ch,1);
		}
		Gpgga.Utc_Time.second=atof(temp);
		ch=Get_next();
	}
	// ��γ�� :ddmm.mmmm,
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.Latitude=GPS_coord_to_degrees(temp);
	
	
	//��γ�Ȱ���  N��S
	memset(temp,0,sizeof(temp));
	ch=Get_next();
	if(ch!=','){
		Gpgga.Latitude_Directon=ch;
		ch=Get_next();
	}

	//������:dddmm.mmmm,
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	
	Gpgga.Longitude=GPS_coord_to_degrees(temp);
	
	//�����Ȱ��� E?W
	memset(temp,0,sizeof(temp));
	ch=Get_next();
	if(ch!=','){
		Gpgga.Longitude_Direction=ch;
		ch=Get_next();
	}

	//����λ����
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	 }
	Gpgga.Location_Quality=atoi(temp);
	


	//��ָ��������
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.Satelite_Quantity=atoi(temp);
		
	//��ˮƽ��ȷ��
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.Level_Accuracy=atof(temp);

	// �����뺣ƽ�� �߶�
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.Sea_Dials=atof(temp);
	//�߶ȵ�λ1
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.UnitOfHeight1='M';      //ע���
	//��� ����������Ժ�ƽ��߶�
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.Relative_Height=atof(temp);

	//�߶ȵ�λ2
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.UnitOfHeight2='M';    //ע���
	// ���GPS��������
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!=','){
		strncat(temp,&ch,1);
	}
	Gpgga.RTCM=atoi(temp);
	//��ֲο���վ���
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!='*'){
		strncat(temp,&ch,1);
	}
	Gpgga.BaseStation_Number=atoi(temp);
	//У���
	memset(temp,0,sizeof(temp));
	while((ch=Get_next())!='\n'){
		strncat(temp,&ch,1);
	}
	Gpgga.Sum=atoi(temp);
	//rt_kprintf("c:%d\n",Gpgga.Longitude);
	return;
}

int  GPS_GetData::Get_DataType(char* data){
	char GGA[4]="GGA";
	char GSA[4]="GSA";
	char GSV[4]="GSV";
	char RMC[4]="RMC";
	char VTG[4]="VTG";
	char GLL[4]="GLL";
	char ZDA[4]="ZDA";
	if((strncmp(data,GGA,3))==0){
		return GPGGA;
	}
	else if((strncmp(data,GSA,3))==0){
		return GPGSA;
	}
	else if(strncmp(data,GSV,3)==0){
		return GPGSV;
	}
	else if(strncmp(data,RMC,3)==0){
		return GPRMC;
	}
	else if(strncmp(data,VTG,3)==0){
		return GPVTG;
	}
	else if(strncmp(data,GLL,3)==0){
		return GPGLL;
	}
	else if(strncmp(data,ZDA,3)==0){
		return GPZDA;
	}
	else
		return 0;
}

int GPS_GetData::GPS_coord_to_degrees(char* s) {
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

void GPS_GetData::Get_Altitude(int32_t &alt0){
	alt0=Gpgga.Sea_Dials;
	
}

void GPS_GetData::Get_GPS_Direction(char &lng_dir,char &lat_dir){ //�õ�N/S �� E/W
	lat_dir=Gpgga.Latitude_Directon;
	lng_dir=Gpgga.Longitude_Direction;
}

