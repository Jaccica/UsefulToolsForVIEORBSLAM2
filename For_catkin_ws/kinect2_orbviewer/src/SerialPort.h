#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <fcntl.h>//�ļ����ƶ���
#include <termios.h>//POSIX�淶�µ��ն˿��ƶ���
#include <cstring>//ʹ��bzero��0
#include <unistd.h>//unix ��׼����,��sleep()

#include <pthread.h>
#include <sched.h>//ʹ�����ȼ�����

#include <string>
#include <iomanip>
#include <fstream>//write a file without useless open/close
#include <chrono>
//#include <time.h>//����ʱ���
/*#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>*/
#include <ros/ros.h>//publish ros topic
#include <map>
#include <vector>//for fast recording

#include "System.h"

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
const std::string gstrHome="";//we use "rosbag record topicname" to record images
//const std::string gstrHome="/home/ubuntu";
//const std::string gstrHome="/media/ubuntu/SD Root";
const std::string gstrORBFile="/home/ubuntu";//place for encoder/IMU data & input files
//const BYTE COMMAND=0x0C;//31 bytes
const BYTE COMMAND=0x03;//23 bytes
#define ENC_AUTO_START "# 5"//5ms

extern ORB_SLAM2::System* gpSLAM;
extern double gdCmdVel[2];//0 for y1, 1 for y2(right wheel target speed)
extern bool gbCmdChanged;
template<class DATATYPE>
struct NodeOdom{
  double tmstamp;
  DATATYPE* data;int size;
  NodeOdom(DATATYPE* pdata,int num,double tm){
    data=new DATATYPE[num];tmstamp=tm;size=num;
    for (int i=0;i<size;++i){data[i]=pdata[i];}
  }
  ~NodeOdom(){delete[] data;}
  NodeOdom& operator=(const NodeOdom& node){
    if (node.size!=size) return *this;
    tmstamp=node.tmstamp;
    for (int i=0;i<size;++i){data[i]=node.data[i];}
  }
  NodeOdom(const NodeOdom& node){
    data=new DATATYPE[node.size];
    tmstamp=node.tmstamp;size=node.size;
    for (int i=0;i<size;++i){data[i]=node.data[i];}
  }
};
extern std::vector<NodeOdom<short>> gvecEnc;//for fast recording
extern std::vector<NodeOdom<double>> gvecIMU;

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
	static std::map<double,std::vector<double>> mmOdomHis[2];//pair<tm_stamp,st_odom[n]> n=2 for hall[0] n=13 for imu[1], for polling mode
	static unsigned char mbstartRecord;
	static double m_tm_stamp;//for polling mode
	BYTE *lpBytes;unsigned long lpNo;
	BYTE nFlag;unsigned long nSizeOfBytes;
	//���ݻ�����lpBytes(��ʼ��ΪNULL,�ر�ʱ�Զ�delete)����ǰλ��lpNo��״̬��־nFlag����������С
	//nFlag(��������ʱΪ0):(0Ϊ������ʼ̬,1Ϊ��ȡ��ʽ,2Ϊ�ջ�ȡ���ʽ,3Ϊ������ȡ̬)
	ros::NodeHandle nh;//ros node handle, for publishing topics

	CSerialPort(void);
	CSerialPort(ros::NodeHandle nh);
	~CSerialPort(void);
	bool InitPort(const char* portNo,UINT baud=B115200,char parity='N',UINT databits=8,UINT stopsbits=1);
	/*��ʼ�����ں���:portNo���ڱ��,Ĭ��ֵ��COM1,������Ҫ����9;baud������;parity��żУ��,'Y'��ʾ��Ҫ;databits����λ
	stopsbitsֹͣλ;dwCommEventsĬ��ΪEV_RXCHAR,��ֻҪ�շ�����һ���ַ�,�����һ���¼�;	
	PS:���ȵ��ñ��������д��ڳ�ʼ������������������ϸ��DCB����,��ʹ�����غ���������������ʱ���Զ��رմ���*/
	bool OpenListenThread();//���������߳�:�Դ������ݼ���,�������յ����ݴ�ӡ����Ļ;
	void CloseListenTread();//�رռ����߳�
	bool ReadChar(char& cRecv);//��ȡ���ڽ��ջ�����������;cRecv������ݵı���
	bool ReadData(BYTE* pData,unsigned int* nLen);
	bool WriteData(BYTE *pData,unsigned int nLen);//pDataָ����Ҫд�봮�ڵ����ݻ�����;nLen��Ҫд������ݳ���
	//bool WriteDataCRC16(BYTE *pData,unsigned int nLen);//���Ȳ�����CRC��
	//bool CheckCRC16(BYTE *pData,unsigned int nLen);//���Ȳ�����CRC��,������������2�ֽڸ�CRC��
	UINT GetBytesInCOM(int mode=0);//��ȡ���ڻ������е��ֽ���,��д����ǰ��ü�����������Ӳ������,1Ϊ���룬2Ϊ�����0���߶����
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
	void Openfout(const std::string &filename=""){
	  if (filename!="") mfout.open(filename,std::ios_base::out);
	  else mfout.open(gstrORBFile+"/test_save/odometrysensor.txt");
	  if (mbstartRecord==1)
	    mfout<<"# odometry data\n# file: 'rgbd_dataset_zzh.bag'\n# timestamp vl vr (qx qy qz qw) (magxyz axyz wxyz)"<<std::endl;
	  else
	    mfout<<"# Enc/IMU data\n# file: 'rgbd_dataset_zzh.bag'\n# timestamp [vl vr] / [(qw qx qy qz) magxyz axyz wxyz]"<<std::endl;//please notice different order in q
	  mfout<<std::fixed;
	}
	void Closefout(){mfout.close();}
	void writeToFile(double data[],int num_data,const double &tmstamp=-1);
	void publishTFOdom(double* odomdata,double tm_stamp,char mode);//mode==0 means only encoder estimation;1 means enc&IMU Complementary Filter fusion estimation
};

#endif //SERIALPORT_H_
