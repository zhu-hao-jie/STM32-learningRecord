
/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/ 
#include "main.h"
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mqttclient.h"
#include "cJSON_Process.h"
#include "./dwt_delay/core_delay.h"  
#include "bsp_dht11.h"

/**************************** ������ ********************************/

static TaskHandle_t AppTaskCreate_Handle = NULL;/* ���������� */
static TaskHandle_t Temperature_Task_Handle = NULL;
static TaskHandle_t Test2_Task_Handle = NULL;
//static TaskHandle_t Test3_Task_Handle = NULL;
/********************************** �ں˶����� *********************************/
 
QueueHandle_t MQTT_Data_Queue =NULL;

/******************************* ȫ�ֱ������� ************************************/

DHT11_Data_TypeDef DHT11_Data;

/******************************* �궨�� ************************************/

#define  MQTT_QUEUE_LEN    4   /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define  MQTT_QUEUE_SIZE   4   /* ������ÿ����Ϣ��С���ֽڣ� */

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void AppTaskCreate(void);/* ���ڴ������� */

static void Temperature_Task(void* pvParameters);/* Temperature_Task����ʵ�� */
static void Test2_Task(void* pvParameters);/* Test2_Task����ʵ�� */
//static void Test3_Task(void* pvParameters);/* Test3_Task����ʵ�� */
extern void TCPIP_Init(void);


/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();

  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� AppTaskCreate
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
 
  /* ����Test_Queue */
  MQTT_Data_Queue = xQueueCreate((UBaseType_t ) MQTT_QUEUE_LEN,/* ��Ϣ���еĳ��� */
                                 (UBaseType_t ) MQTT_QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
  if(NULL != MQTT_Data_Queue)
    printf("����MQTT_Data_Queue��Ϣ���гɹ�!\r\n");
  
  TCPIP_Init();
  
  mqtt_thread_init();

  printf("ʹ�ÿ��������ٶ��ƣ������ϱ���ʪ������\n\n");
  
  printf("��������ģ�����£�\n\t ����<--����-->·��<--����-->������\n\n ·�������������ӵ�����(������)\n\n");
  
  printf("ʵ����ʹ��MQTTЭ�鴫������(����TCPЭ��) ����������ΪMQTT Client\n\n");
  
  printf("IP��ַ����User/arch/sys_arch.h�ļ����޸�\n\n");
    
  printf("�ο�Ұ��Lwip MQTT ����\n\n");
   
  printf("�ڿ������dht11�ӿڽ���DHT11��ʪ�ȴ�����\n\n");  
  
  printf("�ٶ�������/IP��ַ : %s \t �˿ں� : %d \n\n",HOST_NAME,HOST_PORT);  
  
  printf("�ٶ���CLIENT_ID : %s\n\n",CLIENT_ID); 
  
  printf("�ٶ���USER_NAME : %s\n\n",USER_NAME); 
  
  printf("�ٶ���PASSWORD : %s\n\n",PASSWORD);  
  
  printf("�ٶ���TOPIC : %s\n\n",TOPIC);  
  
  printf("�ٶ���TEST_MESSAGE : %s\n\n",TEST_MESSAGE);    
  
  taskENTER_CRITICAL();           //�����ٽ���
 
  /* ����Temperature_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )Temperature_Task, /* ������ں��� */
                        (const char*    )"Test1_Task",/* �������� */
                        (uint16_t       )1024,   /* ����ջ��С */
                        (void*          )NULL,	/* ������ں������� */
                        (UBaseType_t    )2,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&Temperature_Task_Handle);/* ������ƿ�ָ�� */
  if(pdPASS == xReturn)
    PRINT_DEBUG("Create Temperature_Task success...\r\n");
  
  /* ����Test2_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )Test2_Task,  /* ������ں��� */
                        (const char*    )"Test2_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&Test2_Task_Handle);/* ������ƿ�ָ�� */ 
						
  if(pdPASS == xReturn)
    PRINT_DEBUG("Create Test2_Task success...\n\n");
						
 /* ����Test3_Task���� */
//  xReturn = xTaskCreate((TaskFunction_t )Test3_Task,  /* ������ں��� */
//                        (const char*    )"Test3_Task",/* �������� */
//                        (uint16_t       )512,  /* ����ջ��С */
//                        (void*          )NULL,/* ������ں������� */
//                        (UBaseType_t    )2, /* ��������ȼ� */
//                        (TaskHandle_t*  )&Test3_Task_Handle);/* ������ƿ�ָ�� */ 
//  if(pdPASS == xReturn)
//    PRINT_DEBUG("Create Test3_Task sucess...\n\n");
//  
//  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
//  taskEXIT_CRITICAL();            //�˳��ٽ���
}



/**********************************************************************
  * @ ������  �� Temperature_Task
  * @ ����˵���� Test1_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void Temperature_Task(void* parameter)
{	
  uint8_t res;
  BaseType_t xReturn = pdPASS;
  //ϵͳ��ʱ��ʼ��
  CPU_TS_TmrInit();
  //DHT11��ʼ��
  DHT11_Init();
  DHT11_Data_TypeDef* send_data;
  while (1)
  {
    taskENTER_CRITICAL();           //�����ٽ���
    res = DHT11_Read_TempAndHumidity(&DHT11_Data);
    taskEXIT_CRITICAL();            //�˳��ٽ���
    send_data = &DHT11_Data;
    if(SUCCESS == res)
    {
      printf("humidity = %f , temperature = %f\n",
             DHT11_Data.humidity,DHT11_Data.temperature);
//      printf("������Ϣsend_data1��\n");
      xReturn = xQueueSend( MQTT_Data_Queue, /* ��Ϣ���еľ�� */
                            &send_data,/* ���͵���Ϣ���� */
                            0 );        /* �ȴ�ʱ�� 0 */
      if(xReturn == pdTRUE)
        PRINT_DEBUG("������Ϣ���ݳɹ�");
    }

    LED1_TOGGLE;
//    PRINT_DEBUG("LED1_TOGGLE\n");
    vTaskDelay(1000);/* ��ʱ1000��tick */
  }
}

/**********************************************************************
  * @ ������  �� Test2_Task
  * @ ����˵���� Test2_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void Test2_Task(void* parameter)  //LED ����Ϊ����ָʾ
{	 
  while (1)
  {
    LED2_TOGGLE;
    vTaskDelay(2000);/* ��ʱ2000��tick */
  }
}



/**********************************************************************
  * @ ������  �� Test3_Task
  * @ ����˵���� Test3_Task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/

//static void Test3_Task(void* parameter)
//{	
//  uint8_t res;
//  BaseType_t xReturn = pdPASS;
//  //ϵͳ��ʱ��ʼ��
//  CPU_TS_TmrInit();
//  /* ��ʼ������ͷGPIO��IIC */
//  OV5640_HW_Init();
//  /* ��ȡ����ͷоƬID��ȷ������ͷ�������� */
//  OV5640_ReadID(&OV5640_Camera_ID);
//    uint32_t DMA_Memory0BaseAddr;
//	uint16_t DMA_BufferSize;
	
//  while (1)
 // {
//    taskENTER_CRITICAL();           //�����ٽ���
//    OV5640_DMA_Config(DMA_Memory0BaseAddr, DMA_BufferSize)
//    taskEXIT_CRITICAL();            //�˳��ٽ���
///    
//    if(SUCCESS == res)
 //   {
//      printf("video = %f\n",
//             &DMA_Memory0BaseAddr);
//      printf("������Ϣsend_data1��\n");
//      xReturn = xQueueSend( MQTT_Data_Queue, /* ��Ϣ���еľ�� */
//                            &DMA_Memory0BaseAddr;,/* ���͵���Ϣ���� */
//                            0 );        /* �ȴ�ʱ�� 0 */
 //     if(xReturn == pdTRUE)
//        PRINT_DEBUG("������Ϣ���ݳɹ�");
//    }

 //   LED1_TOGGLE;
//    PRINT_DEBUG("LED1_TOGGLE\n");
 //   vTaskDelay(1000);/* ��ʱ1000��tick */
 // }
//}


/********************************END OF FILE****************************/
