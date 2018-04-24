#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <fcntl.h>//�ļ����ƶ���
#include <termios.h>//POSIX�淶�µ��ն˿��ƶ���
#include <cstring>//ʹ��bzero��0
#include <unistd.h>//unix ��׼����,��sleep()

#include <pthread.h>
#include <sched.h>//ʹ�����ȼ�����

#include <fstream>
//#include <chrono>
#include <time.h>//����ʱ���
/*#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>*/
//#include <ros/ros.h>//publish ros topic
//#include <map>

const int INVALID_HANDLE_VALUE=-1;
typedef int HANDLE;
typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
//zzh defined color cout, must after include opencv2
#define redSTR "\033[31m"
#define brightredSTR "\033[31;1m"
#define greenSTR "\e[32m"
#define brightgreenSTR "\e[32;1m"
#define blueSTR "\e[34m"
#define brightblueSTR "\e[34;1m"
#define yellowSTR "\e[33;1m"
#define brownSTR "\e[33m"
#define azureSTR "\e[36;1m"
#define whiteSTR "\e[0m"
//const std::string gstrHome="/media/ubuntu/SD Root";
//const std::string gstrORBFile="/home/ubuntu";
//const BYTE COMMAND=0x0C;//31 bytes
const BYTE COMMAND=0x03;//23 bytes
#define ENC_AUTO_START "# 5"

extern double gdCmdVel[2];//0 for y1, 1 for y2(right wheel target speed)
extern bool gbCmdChanged;

class CSerialPort//�����ɹ����ط�0������Ϊ0
{
private:
	HANDLE  m_hComm;//���ھ��
	volatile HANDLE m_hListenThread;//�߳̾��
	static bool st_bExit;//�߳��˳���־����
	//unsigned short tableCRC16[256];//CRC���ֽ�ֵ��
	static UINT ListenThread(void* pParam);
	/*���ڼ����߳�:�������Դ��ڵ����ݺ���Ϣ,�����б���Ϊ��̬,����ᴫ�ݵڶ�������thisָ��
	pParam�̲߳���;return:UINT WINAPI�̷߳���ֵ*/
	pthread_mutex_t m_csCommSync;//�����������������ٽ���Դ����ǰ���ڻ�����ͬʱֻ��һ���̷߳���

	std::ofstream mfout;

public:
	static unsigned char mbstartRecord;
	static double m_tm_stamp;
	BYTE *lpBytes;unsigned long lpNo;
	BYTE nFlag;unsigned long nSizeOfBytes;
	//���ݻ�����lpBytes(��ʼ��ΪNULL,�ر�ʱ�Զ�delete)����ǰλ��lpNo��״̬��־nFlag����������С
	//nFlag(��������ʱΪ0):(0Ϊ������ʼ̬,1Ϊ��ȡ��ʽ,2Ϊ�ջ�ȡ���ʽ,3Ϊ������ȡ̬)
	//ros::NodeHandle nh;//ros node handle, for publishing topics

	CSerialPort(void);
	//CSerialPort(ros::NodeHandle nh);
	~CSerialPort(void);
	bool InitPort(const char* portNo,UINT baud=B115200,char parity='N',UINT databits=8,UINT stopsbits=1);
	/*��ʼ�����ں���:portNo���ڱ��,Ĭ��ֵ��COM1,������Ҫ����9;baud������;parity��żУ��,'Y'��ʾ��Ҫ;databits����λ
	stopsbitsֹͣλ;dwCommEventsĬ��ΪEV_RXCHAR,��ֻҪ�շ�����һ���ַ�,�����һ���¼�;	
	PS:���ȵ��ñ��������д��ڳ�ʼ������������������ϸ��DCB����,��ʹ�����غ���������������ʱ���Զ��رմ���*/
	bool OpenListenThread();//���������߳�:�Դ������ݼ���,�������յ����ݴ�ӡ����Ļ;
	void CloseListenThread();//�رռ����߳�
	bool ReadChar(char& cRecv);//��ȡ���ڽ��ջ�����������;cRecv������ݵı���
	bool ReadData(BYTE* pData,unsigned int* nLen);
	bool WriteData(BYTE *pData,unsigned int nLen);//pDataָ����Ҫд�봮�ڵ����ݻ�����;nLen��Ҫд������ݳ���
	//bool WriteDataCRC16(BYTE *pData,unsigned int nLen);//���Ȳ�����CRC��
	//bool CheckCRC16(BYTE *pData,unsigned int nLen);//���Ȳ�����CRC��,������������2�ֽڸ�CRC��
	UINT GetBytesInCOM(int mode=0);//��ȡ���ڻ������е��ֽ���,��д����ǰ��ü�����������Ӳ������
	void ClosePort();
	//void MakeTableCRC16(bool LowBitFirst=1);//1��ʾ��λ�ȴ��Ͱ汾;Ĭ�϶����ֽ��ȴ���

	bool GetbListen()
	{
		return m_hListenThread!=INVALID_HANDLE_VALUE;
	}
	bool GetbLink()
	{
		return m_hComm!=INVALID_HANDLE_VALUE;
	}
	bool BufferBytes(BYTE chr);//���ֽ��ַ����뻺����
};

#endif //SERIALPORT_H_
