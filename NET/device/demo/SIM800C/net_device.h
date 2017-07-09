#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_







typedef struct
{
	
	unsigned short err : 2; 		//��������
	unsigned short initStep : 4;	//��ʼ������
	unsigned char cardType : 3;		//�ֻ���Ϊ1��������Ϊ5
	unsigned short dataType : 4;	//�趨���ݷ�������--16��
	unsigned short reboot : 1;		//����������־
	unsigned short reverse : 2;		//Ԥ��

	unsigned short ipdBytes;

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO netDeviceInfo;



#define NET_DEVICE_PWRK_ON		GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define NET_DEVICE_PWRK_OFF		GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define GET_STATUS   GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)

#define NET_DEVICE_TRANS		0 //1-ʹ��͸��ģʽ		0-ʧ��͸��ģʽ




void NET_DEVICE_IO_Init(void);

_Bool NET_DEVICE_Exist(void);

_Bool NET_DEVICE_Init(char *ip, char *port);

void NET_DEVICE_Reset(void);

_Bool NET_DEVICE_ReLink(char *ip, char *port);

_Bool NET_DEVICE_SendCmd(char *cmd, char *res, _Bool mode);

void NET_DEVICE_SendData(unsigned char *data, unsigned short len);

unsigned char *NET_DEVICE_GetIPD(unsigned short timeOut);

void NET_DEVICE_ClrData(void);

unsigned char NET_DEVICE_Check(void);

void NET_DEVICE_ReConfig(unsigned char step);

void NET_DEVICE_Set_DataMode(unsigned char mode);

unsigned char NET_DEVICE_Get_DataMode(void);

#endif