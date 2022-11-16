#include "ap3216c.h"
#include "myiic.h"
#include "./dwt_delay/core_delay.h"


//初始化AP3216C，内部温度传感器
//返回值:0,初始化成功
//      1,初始化失败
u8 AP3216C_Init(void)
{
	u8 temp=0;
	IIC_Init();         				//初始化IIC
	AP3216C_WriteOneByte(0x00,0X04);	//复位AP3216C
	Delay_ms(50);						//AP33216C复位至少10ms
	AP3216C_WriteOneByte(0x00,0X03);	//开启ALS、PS+IR   
	temp=AP3216C_ReadOneByte(0X00);		//读取刚刚写进去的0X03
	if(temp==0X03)return 0;				//AP3216C正常
	else return 1;						//AP3216C失败
} 

//读取AP3216C的数据
//读取原始数据，包括ALS,PS和IR
//注意！如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
void AP3216C_ReadData(u16* ir,u16* ps,u16* als)
{
    uint8_t buf[6];
    uint8_t i;
    for(i=0;i<6;i++)	
    {
        buf[i]=AP3216C_ReadOneByte(0X0A+i);		//循环读取所有传感器数据
    }
    if(buf[0]&0X80)*ir=0;						//IR_OF位为1,则数据无效
	else *ir=((u16)buf[1]<<2)|(buf[0]&0X03); 	//读取IR传感器的数据  
	*als=((uint16_t)buf[3]<<8)|buf[2];				//读取ALS传感器的数据   
    if(buf[4]&0x40)*ps=0;    					//IR_OF位为1,则数据无效
	else *ps=((uint16_t)(buf[5]&0X3F)<<4)|(buf[4]&0X0F); //读取PS传感器的数据   
}

//IIC写一个字节  
//reg:寄存器地址
//data:要写入的数据
//返回值:0,正常
//    其他,错误代码
uint8_t AP3216C_WriteOneByte(uint8_t reg,uint8_t data)
{
    IIC_Start();
    IIC_Send_Byte(AP3216C_ADDR|0X00);//发送器件地址+写命令
    if(IIC_Wait_Ack())          //等待应答
    {
        IIC_Stop();
        return 1;
    }
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
    IIC_Send_Byte(data);        //发送数据
    if(IIC_Wait_Ack())          //等待ACK
    {
        IIC_Stop();
        return 1;
    }
    IIC_Stop();
    return 0;
}

//IIC读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
uint8_t AP3216C_ReadOneByte(uint8_t reg)
{
    uint8_t res;
    IIC_Start();
    IIC_Send_Byte(AP3216C_ADDR|0X00); //发送器件地址+写命令
    IIC_Wait_Ack();             //等待应答
    IIC_Send_Byte(reg);         //写寄存器地址
    IIC_Wait_Ack();             //等待应答
	IIC_Start();                
    IIC_Send_Byte(AP3216C_ADDR|0X01); //发送器件地址+读命令
    IIC_Wait_Ack();             //等待应答
    res=IIC_Read_Byte(0);		//读数据,发送nACK  
    IIC_Stop();                 //产生一个停止条件
    return res;  
}

