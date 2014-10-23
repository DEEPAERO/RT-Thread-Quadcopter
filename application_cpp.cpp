#include "Led.h"
#include "stdio.h"
#include "rtthread.h"
#include "stm32f4xx.h"
#include "Quadx.h"
#include "Communication.h"
#include "cpu_usage.h"
#include "MPU6050.h"
#include "HMC5883L.h"
#include "MS5611.h"
#include "Receiver.h"
#include "Motor.h"
#include "I2Cdev.h"

struct param_t
{
	struct pid_t PID[4];
	int16_t accXOffset,accYOffset,accZOffset;
	int16_t gyroXOffset,gyroYOffset,gyroZOffset;
	int16_t magXOffset,magYOffset,magZOffset;
};




struct ctrl_t
{
	bool att,thro,coor;
	bool alt,trace,quadx;
};
struct ctrl_t ctrl = {0};

extern "C" 
{
	extern int16_t targetX,targetY;
	void rt_thread_entry_trace(void* parameter);
	uint8_t ov_7725_init(void);
}

void hardware_init(void);

void rt_thread_entry_main(void* parameter)
{
/*************************************
	declare variables
*************************************/	
	ctrl.quadx = ctrl.att = ctrl.thro = true; ctrl.alt = ctrl.coor = ctrl.trace = false;
	uint8_t rxData[RX_DATA_SIZE] = {0},txData[TX_DATA_SIZE];
	uint8_t major,minor;

/*************************************
	hardware init
*************************************/
	hardware_init();
	
/*************************************
	create thread
*************************************/
	/*led_thread*/
	rt_thread_t led_thread = rt_thread_create("led",
												rt_thread_entry_led_test,
												RT_NULL,
												512,//max used = 140 
												1,
												10);
	/*communication_thread*/
	rt_thread_t communication_thread = rt_thread_create("communication",
												rt_thread_entry_communication,
												RT_NULL,
												512,
												9,
												10);
	/*quadx_get_thread*/
	rt_thread_t quadx_get_thread = rt_thread_create("quadx_get_attitude",
												rt_thread_entry_quadx_get_attitude,
												RT_NULL,
												1024,
												7,
												10);
	/*quadx_control_thread*/
	rt_thread_t quadx_control_thread = rt_thread_create("quadx_control_attitude",
												rt_thread_entry_quadx_control_attitude,
												RT_NULL,
												1024,
												7,
												10);
												
	/*track_thread*/
	rt_thread_t trace_thread = rt_thread_create("trace",
												rt_thread_entry_trace,
												RT_NULL,
												1024,
												10,
												500);

/*************************************
	start thread
*************************************/			
	if(led_thread != RT_NULL) rt_thread_startup(led_thread);
	if(communication_thread != RT_NULL) rt_thread_startup(communication_thread);
	if(quadx_get_thread != RT_NULL) rt_thread_startup(quadx_get_thread);
	if(quadx_control_thread != RT_NULL) rt_thread_startup(quadx_control_thread);
//	if(trace_thread != RT_NULL) rt_thread_startup(trace_thread);
	
	//�ó�cpu����β�ȴ�����
	rt_thread_delay(100);
/*************************************
	main loop
*************************************/
	while(1)
	{
		//recv
		if(rt_mq_recv(rxQ,rxData,RX_DATA_SIZE,0) == RT_EOK)
		{
			if(rxData[0]>=0xda&&rxData[0]<=0xdd)
			{
				PID[rxData[0] - 0xda].P = rxData[1] / 10.0f;//P[0,20],����0.1
				PID[rxData[0] - 0xda].I = rxData[2] / 1000.0f;//I[0,0.250],����0.001
				PID[rxData[0] - 0xda].D = rxData[3] / 10.0f;//D[0,20],����0.1
			}
			else if(rxData[0]==0xca)
			{
				//TODO: restart
				NVIC_SystemReset();
			}
			else if(rxData[0]==0xcb)
			{
				//����ģʽ
				if(rxData[1] == 0xf1) ctrl.quadx = true;
				else if(rxData[1] == 0xf0) ctrl.quadx = false;
			}
			else if(rxData[0]==0xcc)
			{
				//ͼ�����ģʽ
				if(rxData[1] == 0xf1) ctrl.alt = true;
				else if(rxData[1] == 0xf0) ctrl.alt = false;
			}
			else if(rxData[0]==0xcd)
			{
				if(rxData[1] == 0xf1) ctrl.att = true;
				else if(rxData[1] == 0xf0) ctrl.att = false;
				if(rxData[2] == 0xf1) ctrl.thro = true;
				else if(rxData[2] == 0xf0) ctrl.thro = false;
				if(rxData[3] == 0xf1) ctrl.coor = true;
				else if(rxData[3] == 0xf0) ctrl.coor = false;
			}
			else if(rxData[0]==0xce)
			{
				//���ָ߶�ģʽ
				if(rxData[1] == 0xf1) ctrl.alt = true;
				else if(rxData[1] == 0xf0) ctrl.alt = false;
			}
			else if(rxData[0]==0xcf)
			{
				if(!ctrl.quadx)
				{
					Led::interval = 100;
					if(rxData[1] == 0xf1)
					{
						MPU6050 *accelgyro = new MPU6050();
						accelgyro->setOffset();
						delete accelgyro;
					}
					else if(rxData[1] == 0xf0)
					{
						HMC5883L *mag = new HMC5883L();
						mag->setOffset();
						delete mag;
					}
					Led::interval = 500;
				}
			}
			else
			{
				rt_kprintf("Unknown command!\r\n");
			}
		}
		//send
		if(ctrl.att)
		{
			uint8_t i;
			txData[0] = 0xea;
			for(i=0;i<3;i++)
				((int16_t*)(txData+1))[i] = att[i] * 10;//�Ƕȳ�10���з���
			((uint16_t*)(txData+1))[i] = att[i] * 50;//�׳�50���޷���
			rt_mq_send(txQ,txData,TX_DATA_SIZE);
		}
		if(ctrl.thro)
		{
			uint8_t i;
			txData[0] = 0xeb;
			for(i=0;i<4;i++)
				((uint16_t*)(txData+1))[i] = motorValue[i];//������ˣ��޷���
			rt_mq_send(txQ,txData,TX_DATA_SIZE);
		}
		if(ctrl.coor)
		{
			txData[0] = 0xec;
			((int16_t*)(txData+1))[0] = targetX;
			((int16_t*)(txData+1))[1] = targetY;
			rt_mq_send(txQ,txData,TX_DATA_SIZE);
		}
		
//		char str[100];
//		sprintf(str,"%+f\t%+f\t%+f\t%+f\r\n",att[PITCH],att[ROLL],att[YAW],att[THROTTLE]);
//		sprintf(str,"%+d\t%+d\t%+d\t%+d\r\n",motorValue[0],motorValue[1],motorValue[2],motorValue[3]);
//		cpu_usage_get(&major,&minor);
//		sprintf(str,"major: %d\tminor: %d\r\n",major,minor);
//		rt_kprintf("%s",str);
		rt_thread_delay(50);
	}
}

void hardware_init(void)
{
	Led *led = new Led();
	led->initialize();
	led->on();
	rt_thread_delay(2000);
		
	Receiver::initialize();
	I2Cdev::initialize();
	Motor::initialize();
	
	MPU6050 *accgyro = new MPU6050();
	while(!accgyro->initialize()) ;
	delete accgyro;
	
	HMC5883L *mag = new HMC5883L();
	while(!mag->initialize());
	delete mag;
	
	MS5611 *baro = new MS5611();
	while(!baro->initialize());
	delete baro;
	
//	while(!ov_7725_init());
}

void param_init(void)
{
	
}

void param_save(void)
{
	
}

int  rt_application_init(void)
{	
	cpu_usage_init();
	rt_thread_t main_thread;
	main_thread = rt_thread_create("main",
									rt_thread_entry_main,
									RT_NULL,
									512,//max used = 140
									8,
									10);
	if(main_thread != RT_NULL)
		rt_thread_startup(main_thread);
	return 0;
}
