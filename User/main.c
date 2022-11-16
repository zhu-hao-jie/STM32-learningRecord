
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 
#include "main.h"
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mqttclient.h"
#include "cJSON_Process.h"
#include "./dwt_delay/core_delay.h"  
#include "bsp_dht11.h"

/**************************** 任务句柄 ********************************/

static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static TaskHandle_t Temperature_Task_Handle = NULL;
static TaskHandle_t Test2_Task_Handle = NULL;
//static TaskHandle_t Test3_Task_Handle = NULL;
/********************************** 内核对象句柄 *********************************/
 
QueueHandle_t MQTT_Data_Queue =NULL;

/******************************* 全局变量声明 ************************************/

DHT11_Data_TypeDef DHT11_Data;

/******************************* 宏定义 ************************************/

#define  MQTT_QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  MQTT_QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void Temperature_Task(void* pvParameters);/* Temperature_Task任务实现 */
static void Test2_Task(void* pvParameters);/* Test2_Task任务实现 */
//static void Test3_Task(void* pvParameters);/* Test3_Task任务实现 */
extern void TCPIP_Init(void);


/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  /* 开发板硬件初始化 */
  BSP_Init();

  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */    
}


/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
 
  /* 创建Test_Queue */
  MQTT_Data_Queue = xQueueCreate((UBaseType_t ) MQTT_QUEUE_LEN,/* 消息队列的长度 */
                                 (UBaseType_t ) MQTT_QUEUE_SIZE);/* 消息的大小 */
  if(NULL != MQTT_Data_Queue)
    printf("创建MQTT_Data_Queue消息队列成功!\r\n");
  
  TCPIP_Init();
  
  mqtt_thread_init();

  printf("使用开发板接入百度云，并且上报温湿度数据\n\n");
  
  printf("网络连接模型如下：\n\t 电脑<--网线-->路由<--网线-->开发板\n\n 路由器必须能连接到外网(能上网)\n\n");
  
  printf("实验中使用MQTT协议传输数据(依赖TCP协议) ，开发板作为MQTT Client\n\n");
  
  printf("IP地址均在User/arch/sys_arch.h文件中修改\n\n");
    
  printf("参考野火Lwip MQTT 例程\n\n");
   
  printf("在开发板的dht11接口接上DHT11温湿度传感器\n\n");  
  
  printf("百度云域名/IP地址 : %s \t 端口号 : %d \n\n",HOST_NAME,HOST_PORT);  
  
  printf("百度云CLIENT_ID : %s\n\n",CLIENT_ID); 
  
  printf("百度云USER_NAME : %s\n\n",USER_NAME); 
  
  printf("百度云PASSWORD : %s\n\n",PASSWORD);  
  
  printf("百度云TOPIC : %s\n\n",TOPIC);  
  
  printf("百度云TEST_MESSAGE : %s\n\n",TEST_MESSAGE);    
  
  taskENTER_CRITICAL();           //进入临界区
 
  /* 创建Temperature_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Temperature_Task, /* 任务入口函数 */
                        (const char*    )"Test1_Task",/* 任务名字 */
                        (uint16_t       )1024,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Temperature_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    PRINT_DEBUG("Create Temperature_Task success...\r\n");
  
  /* 创建Test2_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Test2_Task,  /* 任务入口函数 */
                        (const char*    )"Test2_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&Test2_Task_Handle);/* 任务控制块指针 */ 
						
  if(pdPASS == xReturn)
    PRINT_DEBUG("Create Test2_Task success...\n\n");
						
 /* 创建Test3_Task任务 */
//  xReturn = xTaskCreate((TaskFunction_t )Test3_Task,  /* 任务入口函数 */
//                        (const char*    )"Test3_Task",/* 任务名字 */
//                        (uint16_t       )512,  /* 任务栈大小 */
//                        (void*          )NULL,/* 任务入口函数参数 */
//                        (UBaseType_t    )2, /* 任务的优先级 */
//                        (TaskHandle_t*  )&Test3_Task_Handle);/* 任务控制块指针 */ 
//  if(pdPASS == xReturn)
//    PRINT_DEBUG("Create Test3_Task sucess...\n\n");
//  
//  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
//  taskEXIT_CRITICAL();            //退出临界区
}



/**********************************************************************
  * @ 函数名  ： Temperature_Task
  * @ 功能说明： Test1_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Temperature_Task(void* parameter)
{	
  uint8_t res;
  BaseType_t xReturn = pdPASS;
  //系统延时初始化
  CPU_TS_TmrInit();
  //DHT11初始化
  DHT11_Init();
  DHT11_Data_TypeDef* send_data;
  while (1)
  {
    taskENTER_CRITICAL();           //进入临界区
    res = DHT11_Read_TempAndHumidity(&DHT11_Data);
    taskEXIT_CRITICAL();            //退出临界区
    send_data = &DHT11_Data;
    if(SUCCESS == res)
    {
      printf("humidity = %f , temperature = %f\n",
             DHT11_Data.humidity,DHT11_Data.temperature);
//      printf("发送消息send_data1！\n");
      xReturn = xQueueSend( MQTT_Data_Queue, /* 消息队列的句柄 */
                            &send_data,/* 发送的消息内容 */
                            0 );        /* 等待时间 0 */
      if(xReturn == pdTRUE)
        PRINT_DEBUG("发送消息数据成功");
    }

    LED1_TOGGLE;
//    PRINT_DEBUG("LED1_TOGGLE\n");
    vTaskDelay(1000);/* 延时1000个tick */
  }
}

/**********************************************************************
  * @ 函数名  ： Test2_Task
  * @ 功能说明： Test2_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Test2_Task(void* parameter)  //LED 灯作为传输指示
{	 
  while (1)
  {
    LED2_TOGGLE;
    vTaskDelay(2000);/* 延时2000个tick */
  }
}



/**********************************************************************
  * @ 函数名  ： Test3_Task
  * @ 功能说明： Test3_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

//static void Test3_Task(void* parameter)
//{	
//  uint8_t res;
//  BaseType_t xReturn = pdPASS;
//  //系统延时初始化
//  CPU_TS_TmrInit();
//  /* 初始化摄像头GPIO及IIC */
//  OV5640_HW_Init();
//  /* 读取摄像头芯片ID，确定摄像头正常连接 */
//  OV5640_ReadID(&OV5640_Camera_ID);
//    uint32_t DMA_Memory0BaseAddr;
//	uint16_t DMA_BufferSize;
	
//  while (1)
 // {
//    taskENTER_CRITICAL();           //进入临界区
//    OV5640_DMA_Config(DMA_Memory0BaseAddr, DMA_BufferSize)
//    taskEXIT_CRITICAL();            //退出临界区
///    
//    if(SUCCESS == res)
 //   {
//      printf("video = %f\n",
//             &DMA_Memory0BaseAddr);
//      printf("发送消息send_data1！\n");
//      xReturn = xQueueSend( MQTT_Data_Queue, /* 消息队列的句柄 */
//                            &DMA_Memory0BaseAddr;,/* 发送的消息内容 */
//                            0 );        /* 等待时间 0 */
 //     if(xReturn == pdTRUE)
//        PRINT_DEBUG("发送消息数据成功");
//    }

 //   LED1_TOGGLE;
//    PRINT_DEBUG("LED1_TOGGLE\n");
 //   vTaskDelay(1000);/* 延时1000个tick */
 // }
//}


/********************************END OF FILE****************************/
