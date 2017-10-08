#include "stdio.h"
#include "winsock2.h"
#include "Windows.h"
#pragma comment (lib,"ws2_32")

/*********数据结构****************/

#define uchar unsigned char 
#define uint unsigned int
#define ushort unsigned short	//stm32上面2字节长度数据类型
#define APP_LEN 8192			//定义接受缓冲区长度
#define APP_BLKNUM_LEN 128		//定义接受缓冲每次的长度
#define SEARCHOK 0		
#define LEN 300

/*********定义全局数据 变量****************/

SOCKET s;						//定义套接字
sockaddr_in sa;

char RxData[LEN];				//定义接受缓存区
char TxData[LEN];				//发送	
char RxBuff[APP_LEN];			//定义应用层缓冲区
char RxBuffNum[APP_BLKNUM_LEN];	//定义缓冲区长度数组存起来每次的位置

uint	RecvLen;				//定义checkMsg后的传输层层数据长度
uint	TransLen;				//定义要发送的数据长度
uint    PosiLen;				//交换位置
char MLnkFCB;					//定义Master发过来的LnkFCB的value----  &0x20,只用一位，到时候可以定义为bit类型
bool TranStartFlg=0;			//传输层开始标志
char TranBlkNum=0;				//传输层块序号
char MyTranBlks=0;				//自己的块号，用于存储转存到RxBuff的块数

//ushort数据类型需要根据情况做
ushort dnp_crc_table[256] = {
 0x0000, 0x365e, 0x6cbc, 0x5ae2, 0xd978, 0xef26, 0xb5c4, 0x839a,
 0xff89, 0xc9d7, 0x9335, 0xa56b, 0x26f1, 0x10af, 0x4a4d, 0x7c13,
 0xb26b, 0x8435, 0xded7, 0xe889, 0x6b13, 0x5d4d, 0x07af, 0x31f1,
 0x4de2, 0x7bbc, 0x215e, 0x1700, 0x949a, 0xa2c4, 0xf826, 0xce78,
 0x29af, 0x1ff1, 0x4513, 0x734d, 0xf0d7, 0xc689, 0x9c6b, 0xaa35,
 0xd626, 0xe078, 0xba9a, 0x8cc4, 0x0f5e, 0x3900, 0x63e2, 0x55bc,
 0x9bc4, 0xad9a, 0xf778, 0xc126, 0x42bc, 0x74e2, 0x2e00, 0x185e,
 0x644d, 0x5213, 0x08f1, 0x3eaf, 0xbd35, 0x8b6b, 0xd189, 0xe7d7,
 0x535e, 0x6500, 0x3fe2, 0x09bc, 0x8a26, 0xbc78, 0xe69a, 0xd0c4,
 0xacd7, 0x9a89, 0xc06b, 0xf635, 0x75af, 0x43f1, 0x1913, 0x2f4d,
 0xe135, 0xd76b, 0x8d89, 0xbbd7, 0x384d, 0x0e13, 0x54f1, 0x62af,
 0x1ebc, 0x28e2, 0x7200, 0x445e, 0xc7c4, 0xf19a, 0xab78, 0x9d26,
 0x7af1, 0x4caf, 0x164d, 0x2013, 0xa389, 0x95d7, 0xcf35, 0xf96b,
 0x8578, 0xb326, 0xe9c4, 0xdf9a, 0x5c00, 0x6a5e, 0x30bc, 0x06e2,
 0xc89a, 0xfec4, 0xa426, 0x9278, 0x11e2, 0x27bc, 0x7d5e, 0x4b00,
 0x3713, 0x014d, 0x5baf, 0x6df1, 0xee6b, 0xd835, 0x82d7, 0xb489,
 0xa6bc, 0x90e2, 0xca00, 0xfc5e, 0x7fc4, 0x499a, 0x1378, 0x2526,
 0x5935, 0x6f6b, 0x3589, 0x03d7, 0x804d, 0xb613, 0xecf1, 0xdaaf,
 0x14d7, 0x2289, 0x786b, 0x4e35, 0xcdaf, 0xfbf1, 0xa113, 0x974d,
 0xeb5e, 0xdd00, 0x87e2, 0xb1bc, 0x3226, 0x0478, 0x5e9a, 0x68c4,
 0x8f13, 0xb94d, 0xe3af, 0xd5f1, 0x566b, 0x6035, 0x3ad7, 0x0c89,
 0x709a, 0x46c4, 0x1c26, 0x2a78, 0xa9e2, 0x9fbc, 0xc55e, 0xf300,
 0x3d78, 0x0b26, 0x51c4, 0x679a, 0xe400, 0xd25e, 0x88bc, 0xbee2,
 0xc2f1, 0xf4af, 0xae4d, 0x9813, 0x1b89, 0x2dd7, 0x7735, 0x416b,
 0xf5e2, 0xc3bc, 0x995e, 0xaf00, 0x2c9a, 0x1ac4, 0x4026, 0x7678,
 0x0a6b, 0x3c35, 0x66d7, 0x5089, 0xd313, 0xe54d, 0xbfaf, 0x89f1,
 0x4789, 0x71d7, 0x2b35, 0x1d6b, 0x9ef1, 0xa8af, 0xf24d, 0xc413,
 0xb800, 0x8e5e, 0xd4bc, 0xe2e2, 0x6178, 0x5726, 0x0dc4, 0x3b9a,
 0xdc4d, 0xea13, 0xb0f1, 0x86af, 0x0535, 0x336b, 0x6989, 0x5fd7,
 0x23c4, 0x159a, 0x4f78, 0x7926, 0xfabc, 0xcce2, 0x9600, 0xa05e,
 0x6e26, 0x5878, 0x029a, 0x34c4, 0xb75e, 0x8100, 0xdbe2, 0xedbc,
 0x91af, 0xa7f1, 0xfd13, 0xcb4d, 0x48d7, 0x7e89, 0x246b, 0x1235
};
//*********定义结构体**********
typedef struct GlbSendData{
	char cfm;
}GlbSendData;
GlbSendData GSD;
/*****以上定义数据变量，下面定义函数部分***/

/**********函数申明:下************/
void P(uint);
void P2(uint);
void myRead(char *,const char *);
uint myWrite(char *,uint);
char buildFrame(char *,char *,uint);

//初始化链接
void initTCP(){
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
	s=socket(PF_INET,SOCK_STREAM, IPPROTO_TCP);	//定义全局变量Socket
	
	sa.sin_family=PF_INET;
	sa.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");	//设置目标地址
	sa.sin_port=htons(20000);						//设置端口
	//开始连接
	connect(s,(SOCKADDR*)&sa,sizeof(SOCKADDR));		//l连接服务器
}
//*************CRC校验部分*********************/

ushort  dnp_crc16(ushort crc, unsigned int text)
{
   unsigned char index = (unsigned char)(text ^ crc);             /*xor with previous to form index */
   crc = ((crc & ~0xff) >> 8) ^ (dnp_crc_table[index]);
   return crc;
}
char CRC_DNP(unsigned char*buf, unsigned char len)
{
   ushort  crc = 0;
   while (len--)
      crc = dnp_crc16(crc, (unsigned int)*buf++);

   crc ^= 0xFFFF;
   //把计算过的crc码赋值给末尾两个字节
   *buf=crc&0x00ff;	
   *(1+buf)=(crc&0xff00)>>8;
   printf ("crc=%02x %02x\n",crc&0x00ff,(crc&0xff00)>>8);
   return 0;
}
char CRC_DNP2(unsigned char*buf, unsigned char len)
{
   ushort  crc = 0;
   while (len--)
      crc = dnp_crc16(crc, (unsigned int)*buf++);

   crc ^= 0xFFFF;
   //校验crc验证数据是否正确
   if((*buf==crc&0x00ff)&&(*(++buf)==(crc&0xff00)>>8)){
		return -1;			//校验出错
   }   
   return 0;				//正常
}
/**************************************************/
//检测链路层函数
char dealLink(uint count)
{
    uint i,j;
	uchar transLastLen,transBlocks,k;
	uchar linkAdd=0;	//链路层额外增加的字节个数，包括头、控制字、校验、长度位
	
	//判断报文总长
	if(count<10){
		return -10;		//报文总长度小于10
	}
	
	//判断帧头
	for(i=0;i<count;i++)
	{
		if(0x05 == RxData[i] && 0x64 == RxData[i+1])	// 帧头正确
		{
			//判断控制字
			if((uchar)RxData[i+3] <= 0x80)				//转为无符号进行比较
				return -1;								//链路层控制字不对,这是从端，所以DIR为1，所以>=0x80

			//判断目标地址
			/*
			if((RxData[i+4]!=LocalAddr.l)&&(RxData[i+5]!=LocalAddr.h)){
				return -2;													//地址不对
			}
			*/
			
			//判断长度是否小于最小长度
			if((uchar)RxData[i+2] < 0x05){			
				return -3;							// 链路层长度字节显示不够
			}else{									// >=5
				if((uchar)RxData[i+2] > 5)			// >5
				{
					transBlocks = ((uchar)RxData[i+2]-6)/16 + 1;			//计算传输块
					transLastLen = ((uchar)RxData[i+2]-6) % 16+1;			//计算传输最后一块的余数
				}
				else								// ==5
				{
					transBlocks = 0;
				}
			}
			
			linkAdd =5 + transBlocks*2;				//统计本包包装数据的总长度(除了链路层和CRC校验)
			
			//报文长度和实际长度不等，return 
			
			if((count-5-transBlocks*2)!=(uchar)RxData[i+2]){
				printf("%d=============%d\n",(uchar)(count-5-transBlocks*2),(uchar)RxData[i+2]);
				return -8;
			}
			
			//判断CRC校验-链路头
			if(i + 10 <= count )	//避免本包数据总长度 超出缓冲区  <=10才去判断CRC对不对
			{
				if(!(0==CRC_DNP2((uchar *)&RxData[0],8)))	return -5;		//cRC链路层校验失败
			}	

			//处理-传输层
			if((uchar)RxData[i+2]>0x05)								// 存在传输层
			{

				//提取传输层条数、提取传输层末条字节个数
				//传输层分条计算CRC校验
				for(j=0;j<transBlocks;j++)
				{
					if(transBlocks-1 == j)
					{
						if(CRC_DNP2((uchar *)&RxData[10+j*18],transLastLen))
							return -6;									//crc传输校验失败返回-6
					}			
					else
					{
						if(CRC_DNP2((uchar *)&RxData[10+j*18],16))				//校验最后两个字节
							return -7;
					}
				}
				
				//链路层crc校验成功后，把缓冲区重新整理，除去包装数据（链路层）
				for(j=0;j<5;j++)	
						RxData[j] = RxData[j+3];
				
				//传输层CRC校验成功后，提取传输层报文
				for(j=0;j<transBlocks;j++)
				{
					if(transBlocks-1 == j)
					{
						for(k=0;k<transLastLen;k++)
							RxData[5+ k + j*16] = RxData[10 + k + j*18];
					}
					else
					{
						for(k=0;k<16;k++)
							RxData[5+ k + j*16] = RxData[10 + k + j*18];
					}
				}
				//解析正确,保存长度
				RecvLen=count-linkAdd;
				return 1;		//有传输层，解析ok 
			}else{
				for(k=0;k<5;k++)
				RxData[k] = RxData[k+3];
				RecvLen=5;		//有效数据长度5
				return 0;		//没传输层
			}
		}else{
			return -9;			//头不对
		}
    }
	return 0;
}
//sendAck()	从端给主端发送确认和否认confirm
void sendAck(uchar type){
	/************************链路层功能码*********************
	源方站链路层功能码(PRM=1)
       功能码     帧类型               服务功能         FCV位
         0     SEND/期待CONFIRM     使远方链路复位        0
         1     SEND/期待CONFIRM     使用户过程复位        0
         2     SEND/期待CONFIRM     对链路的测试功能      1
         3     SEND/期待CONFIRM     用户数据              1
         4     SEND/不期待回信      非确认的用户数据      0
         9     查询/期待响应        查询链路状态          0
        5-8     未用	
		10-15	未用
	  ****************************************************
	 副方站链路层功能码(PRM=0)
	功能码      帧类型              服务功能
	  0         肯定确认           ACK=肯定的确认
	  1         否定确认           NACK=报文未收到;链路忙
	  11        响应			   链路的状态（DFC=0或DFC=1）
	 2-10       未用           
    12-13       未用 
     14							   链路服务不工作了
     15							   未用链路服务,或未实现链路服务
	*******************************************************/
	char rfc=-1;
	uint len=5;
	switch(type){
		case 2: //SEND/期待CONFIRM     对链路的测试功能

			 break;
		case 3: //SEND/期待CONFIRM     用户数据  

			 break;
		case 0: //SEND/期待CONFIRM     使远方链路复位
			 rfc=1;
			 break;
		case 1: //SEND/期待CONFIRM     使用户过程复位    
			 rfc=1;
			 break;
		case 4: //SEND/不期待回信      非确认的用户数据

			 break;
		case 9:	//查询/期待响应        查询链路状态   

			 break;
	}
	if(-1!=rfc){
		//就一帧，发送RxData
		GSD.cfm=rfc;
		buildFrame(&RxData[3],&TxData[0],len);
	}
}
//checkLnkFC()函数声明:检查链路层功能码
char checkLnkFC(char lfc){

	if(lfc&0x80!=0x80) return -1;					//错误，方向不对
	//fcv是1,或者是0
	printf("功能码:%02x %c %02x\n服务功能:",(unsigned char)lfc,26,(unsigned char)lfc&0x10);
	char ff=lfc&0x10;								//必须重赋值妈的
	/*		---链路层功能码---
		源方站链路层功能码(PRM=1)
       功能码     帧类型               服务功能         FCV位
         0     SEND/期待CONFIRM     使远方链路复位        0
         1     SEND/期待CONFIRM     使用户过程复位        0
         2     SEND/期待CONFIRM     对链路的测试功能      1
         3     SEND/期待CONFIRM     用户数据              1
         4     SEND/不期待回信      非确认的用户数据      0
         9     查询/期待响应        查询链路状态          0
        5-8     未用	
		10-15	未用
	*/
	if(ff==0x10){									//判断FCV位什么情况
		MLnkFCB=lfc&0x20;							//拿到方向码,此位是上一次的取反
		switch(lfc&0x0f){
			case 2:
				printf("对链路的测试功能\n");
				return 2;
				break;
			case 3:
				printf("用户数据 \n");
				return 3;
				break;
		}
	}else{
		switch(lfc&0x0f){
			case 0:
				printf("使远方链路复位\n");
				return 0;
				break;
			case 1:
				printf("使用户过程复位\n");
				return 1;
				break;
			case 4:
				printf("不期待回信,非确认的用户数据\n");
				return 4;
				break;
			case 9:
				printf("查询链路状态\n");				
				return 9;
				break;
		}
	}
	return -2;
}

//checkTxFC函数处理传输层控制字
uchar checkTranFC(char tfc){
	return (uchar)tfc&0xc0;					//
} 
//getTxBlkNum(传输控制字)	拿到传输块号码
char getTranBlkNum(char v){
	return v&0x3f;
}
//rstRxBuff()		清除各个缓冲区
void rstTranTem(){
	uint i;
	for(i=0;i<sizeof(RxBuff);++i){
		RxBuff[i]&=0x00;
	}
	for(i=0;i<sizeof(RxBuffNum);++i){
		RxBuffNum[i]&=0x00;
	}
	TranBlkNum=MyTranBlks=PosiLen=RecvLen=0;	//清理传输块
}
//ptAppLayer(源，类型)	打印应用层处理后的数据
void ptAppLayer(uint sl,char t){
	if(1==t){												//处理单帧
		printf("$传输层解析OK_1_以下先打印一遍数据\n");
		P(sl);
	}else{													//处理缓冲区
		printf("$传输层解析OK_2_以下先打印一遍数据\n");
		while(RxBuffNum[sl]!=0){
			P2(RxBuffNum[sl]);
			sl++;
		}
	}
}
//saveRxBuff缓冲区存入应用层buff
void saveRxBuff(char v){
	uint i;
	if(1==v){
		for(i=0;i<RxBuffNum[MyTranBlks];i++){						
			RxBuff[i]=RxData[i];
		}
	}else{
		uint k;
		k=RxBuffNum[MyTranBlks-1];
		for(i=0;i<RxBuffNum[MyTranBlks];i++){
			RxBuff[k+i+PosiLen]=RxData[i];
		}
		PosiLen+=k;
	}
}

//------处理应用层------
void dealAppFun(uchar *appData){
	//printf("appData is %02x\n",appData[0]);
	uchar ackCode = 0xff;			//定义响应码
	uchar funCode = 0xff;
	uchar objCode = 0xff;
	uchar objGroup = 0xff;	
	uchar objVaria = 0xff;
	uchar qual = 0xff;				// 限定词
	uint sendLen = 0x05;
	//开始赋值
	funCode  = appData[1];			//功能吗
	objGroup = appData[2];			//组  字节
	objVaria = appData[3];			//变体字节
	qual = appData[4];				//限定词
	printf("\ndealAppFun函数中funCode=%02x,objGroup=%02x,objVaria=%02x\n",funCode,objGroup,objVaria);
	if(0 == funCode)				//仅在链路层回复确认
	{
		ackCode = 0;
	}else if(1 == funCode){			//读数据
		/******************************************************************
		   组别	变体	数据对象描述
		  //1	1	不带品质描述的单点输入，即压缩格式的遥信量，8点/字节
			1	2	带品质描述的单点输入，1字节/点的遥信量
			2	1	带品质描述不带时标的单点变位信息，1字节/点
			2	2	带品质描述带绝对时标的单点变位信息，7字节/点
		  //12	1	继电器输出控制（遥控）对象，11字节/点
			30	2	16位带品质描述不带时标的静态当前模拟量，3字节/点
			30	4	16位不带品质描述不带时标的静态当前模拟量，2字节/点
			32	2	16位带品质描述不带时标的变化当前模拟量，3字节/点
		  //20	1	32位带品质描述不带时标的静态累加脉冲量，5字节/点
		  //20	5	32位不带品质描述不带时标的静态累加脉冲量，4字节/点
		  //22	1	32位带品质描述不带时标且变化了的累加脉冲量，5字节/点
			50	1	表示日历钟的绝对时间对象，6字节/点
			60	1	0级数据，即静态数据，仅用于召唤命令，无确切的对象
			60	2	1级数据，优先级最高的变化数据，仅用于召唤命令
			60	3	2级数据，优先级仅次于1级数据的变化数据，仅用于召唤命令
			60	4	3级数据，优先级次于2级数据的变化数据，仅用于召唤命令
		******************************************************************/
		if(1 == objGroup && 2 == objVaria)	// 带品质描述的单点输入，1字节/点的遥信量
		{
			
		}
		else if(2 == objGroup && 1 == objVaria)	// 带品质描述不带时标的单点变位信息，1字节/点
		{
			
		}
		else if(2 == objGroup && 2 == objVaria)	// 带品质描述带绝对时标的单点变位信息，7字节/点
		{
			
		}
		else if(30 == objGroup && 2 == objVaria)	// 16位带品质描述不带时标的静态当前模拟量，3字节/点
		{
			
		}
		else if(30 == objGroup && 4 == objVaria)	// 16位不带品质描述不带时标的静态当前模拟量，2字节/点
		{
			
		}
		else if(32 == objGroup && 2 == objVaria)	// 16位带品质描述不带时标的变化当前模拟量，3字节/点
		{
			
		}
		else if(60 == objGroup && 1 == objVaria)	// 0级数据，即静态数据，仅用于召唤命令，无确切的对象(总召)
		{
			if(qual == 6)	//无变程 、无附加描述
			{
					
			}

		}else{
			printf("应用层组别码其他情况,不做处理了,撤退。。。\n");
			return ;
		}
		/*	********************		限定词表
			限定值	变体类型	变 体 部 分						附加的点号描述部分
			0x00	起止模式	2个字节，第一个字节为起始点号（含），第二个字节为终止点号（含）。	无
			0x01	起止模式	4个字节，前2个字节为起始点号（含），后2个字节为终止点号（含）。	无
			0x02	起止模式	8个字节，前4个字节为起始点号（含），后4个字节为终止点号（含）。	无
			0x07	数量模式	1个字节，记其值为N，所涉及到的点号为：0 ~ N-1	无
			0x08	数量模式	2个字节，记其值为N，所涉及到的点号为：0 ~ N-1	无
			0x09	数量模式	4个字节，记其值为N，所涉及到的点号为：0 ~ N-1	无0x17	数量模式	1个字节，记其值为N	为N个点号，每个点号占1个字节
			0x18	数量模式	2个字节，记其值为N	为N个点号，每个点号占1个字节
			0x19	数量模式	4个字节，记其值为N	为N个点号，每个点号占1个字节
			0x27	数量模式	1个字节，记其值为N	为N个点号，每个点号占2个字节
			0x28	数量模式	2个字节，记其值为N	为N个点号，每个点号占2个字节
			0x29	数量模式	4个字节，记其值为N	为N个点号，每个点号占2个字节
			0x37	数量模式	1个字节，记其值为N	为N个点号，每个点号占4个字节
			0x38	数量模式	2个字节，记其值为N	为N个点号，每个点号占4个字节
			0x39	数量模式	4个字节，记其值为N	为N个点号，每个点号占4个字节
			0x06	全部模式	无。仅用于召唤命令。涉及的点号为接收方支持的所召唤的数据类型的所有点	无
		*******************************/
	}else if(2 == funCode){	// 写数据
		
	}else if(3==funCode){	//遥控预置
		
	}else if(4==funCode){	//遥控执行
		
	}else if(5==funCode){	//直接输出，不需要有先前的选择命令（遥控直接执行）
		
	}else{
		printf("应用层功能码其他情况,不做处理了。。。\n");
		//return ;
	}

	/************开始拼包发送**************/
	if(ackCode < 0xff)
	{
		/*
		sFcLinkSend.Byte |= ackCode;
		buildSendFrame(Uart1RxBuff.Buff,Uart1TxBuff.Buff,5, dstAddr);
		*/
	}
	buildFrame(&RxBuff[0],&TxData[0],8);
}

//buidFrame(收的,发的,count:userData长度)函数to make Confirm
char buildFrame(char *buff,char *send,uint count){
	if(count<5) return -1;		//中断
	uint len;
	uchar j,k;
	uchar transNum,lastLen;

	send[0]=0x05;
	send[1]=0x64;
	send[2]=count;
	//从全局拿
	send[3]=GSD.cfm;
	//目的地址、源地址互换
	send[4]=buff[3];						
	send[5]=buff[4];						
	send[6]=buff[1];
	send[7]=buff[2];
	//crc处理（暂未处理）
	//send[8]=0xcc;	
	CRC_DNP((uchar *)&send[0],8);
	
	//把块和余数计算出来
	if(count>5){
		transNum =(count-6)/16 + 1;
		lastLen =count-5-(transNum-1)*16;
	}else{
		transNum = 0;
		lastLen = 0;
	}
	
	//校验传输层CRC
	if(transNum == 0){
		len = 10;
	}else{
		for(j=0;j<transNum;j++){
			if(transNum-1 == j){				//只有一个末块
				for(k=0;k<lastLen;k++){
					send[10+k+j*18] = buff[5+k+j*16];
				}
				CRC_DNP((uchar*)&send[10+j*18],lastLen);
				len = 10 + j*18 + k + 2;		//发送总长度、一次计算
			}
			else{								// >2块
				for(k=0;k<16;k++){
					send[10+k+j*18] = buff[5+k+j*16];
					CRC_DNP((uchar*)&send[10+j*18],16);
				}
			}
		}
	}	
	//
	
	printf("Seconder将要发送:\n");
	myWrite(&send[0],len);
	return 0;
}
//处理整个报文函数入口
uchar dealDNP(uint len)
{ 
    char FlgCheck;
	char LnkFC;
	FlgCheck = dealLink(len);
	//帧头判断通过
	printf("$链路层解析OK_返回:%d\n",FlgCheck);
	printf("User Data长度为:%d\n",RecvLen);
	printf("User Data为:");
	P(RecvLen);
	
	//错误情况都是负数,return -1回去主函数
	if(FlgCheck<0){
		//链路层解析出错，复位传输层
		rstTranTem();
		return -1;
	}
	
	//处理不带传输层的数据
	if(0==FlgCheck){
		LnkFC=checkLnkFC(RxData[0]);

		if(LnkFC<0){
			printf("链路层控制字解析出错:%d\n",LnkFC);
		}else{
			printf("链路层控制字解析成功OK,无传输层,Confirm功能码为:%d\n",LnkFC);
			sendAck(LnkFC);				//交给函数处理并且发送给master
		}
	}
	//处理带传输层的数据
	if(1==FlgCheck){
		printf("_开始处理传输层\n");
		if(!TranStartFlg){
			//第一块
			if(0x80==checkTranFC(RxData[5])){
				printf("传输层拿到首块标志\n");
				TranStartFlg=1;							//已经开始
				TranBlkNum=getTranBlkNum(RxData[5]);	//记录传输层块号码
				RxBuffNum[MyTranBlks]=RecvLen;
				saveRxBuff(1);							//存buff 			
				printf("传了首块,MyTranBlks序号为:%d,值为:%d\n",MyTranBlks,RxBuffNum[MyTranBlks]);
				return 0;								//返回去继续等待接受报文
			}
			//就一块
			if(0xc0==checkTranFC(RxData[5])){
				//打印出来
				printf("传输层就一块\n");
				ptAppLayer(RecvLen,1);					//applayer
				//处理应用层结束, 开始拼包发送
				//没经过saveRxBuff函数
				dealAppFun((uchar *)&RxData[7]);		//第7位开始应用层
				return 1;
			}
			printf("不是第一块,继续等待第一块\n");
			return -1;
		}else{
			//中间部分&尾部
			if(TranBlkNum+1==getTranBlkNum(RxData[5])){		//号码对上了,0 ~ 63范围				
				TranBlkNum++;								//把给的块数加一
				MyTranBlks++;								//把自己的相对块加一  
				RxBuffNum[MyTranBlks]=RecvLen;				//这个是把每次的存入缓冲区的长度存起来，基本每次都一样，末块可能会少一些
				
				saveRxBuff(2);								//给接受应用层缓冲区存起来
				if(TranBlkNum>=63)	TranBlkNum=0;			//0--63 
				printf("追加了一块,MyTranBlks序号为:%d,对应的值为:%d\n",MyTranBlks,RxBuffNum[MyTranBlks]);
				
				//设置 延时时间(在单片机设置延时时间)
					
				//结束设置延时时间
			}else{

				printf("传输层块号码不对，中断...\n");
				TranStartFlg=0;								//复位首块标志
				rstTranTem();
				return -2;									//传输层号码不对应，中断,放弃处理
			}	
			//检测到末块就推出 
			if(0x40==checkTranFC(RxData[5])){		
				printf("检测到传输层末块\n");
				TranStartFlg=0;								//结束标志切回来
				ptAppLayer(0,2);							//打印

				//处理应用层结束, 开始拼包发送
				dealAppFun((uchar *)&RxBuff[6]);
				return 1;
				rstTranTem();								//复位
			}
		}
		
		//结束处理应用层代码段

	} //end FlgCheck==1判断

	return 0;
}

/**************主函数*************/
int main(){
	initTCP();													//初始化TCP链接
	int len=0,si=0;											//定义接受数据长度
	uint k=0;												//定义k存放字符位置
	uint total=0;
	while(1){
		//send(s,"begin",5,NULL);							//来一个start信号
		
		char tem;
		if((si=recv(s,(char *)&tem,1,NULL))!=-1){
			//连续发送两个0xfe代表对方退出连接
			if((uchar)tem==0xfe){							//头一个字节不会等于0xfe这么大
				recv(s,(char *)&tem,1,NULL);
				if((uchar)tem==0xfe){
					printf("对方失去连接！\n");
					break;
				}
			}
			si=tem*256;
			recv(s,(char *)&tem,1,NULL);
			si+=tem;	
			printf("将要接收 %d 个字节\n",si);

			//******清理变量，下次使用*********//
			len=k=total=0;

			for(len=0;len<si;len++){							//一次收一个字符比较合适 
				if(1!=recv(s,(char *)&tem,1,NULL)) break;

				RxData[k++]=tem;							
				total++;										//拿到总接受长度	
			}
		
			P(total);
			dealDNP(total);									//交给自动机处理(报文长度)
		}
		//********给master回送***********
		

		//send(s,(char *)&RxData[0],1,NULL);
		//send(s,(char *)&RxData[1],1,NULL);
	}
	closesocket(s);	
	WSACleanup();	
	return 0;
}
//自定义一个发送方法，根据长度发字节
uint myWrite(char *str,uint len){	
	uint i;
	//打印出来看一下
	for(i=0;i<len;i++){
		printf("%02x ",(uchar)*(str+i));
	}
	i=send(s,str,len,NULL);
	printf("\n发送了%d个字节长度\n",i);
	return i;
}
//把收到的数据处理为z报文
void myRead( char * dst, const char * src)
{
   char * p = dst;
   while(*p) p++;                 
   while(*p++ = *src++);
 //  return( dst );
}
void P(uint total){
	uint k;
	for(k=0;k<total;k++)	printf("%02X ",(unsigned char)RxData[k]);
	printf("\n");
}
void P2(uint total){
	uint k;
	for(k=0;k<=total;k++)	printf("%02X ",(unsigned char)RxBuff[k]);
	printf("\n");
}

