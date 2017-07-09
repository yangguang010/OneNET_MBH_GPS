/**
  ******************************************************************************
  * @file    gps_config.c
  * @author  fire
  * @version V1.0
  * @date    2014-08-xx
  * @brief   gps模块接口配置驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 ISO-STM32 开发板
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
	*/

#include "gps_config.h"
#include "usart.h"
#include "nmea/nmea.h"


/* DMA接收缓冲  */
uint8_t gps_rbuff[GPS_RBUFF_SIZE];

/* DMA传输结束标志 */
__IO uint8_t GPS_TransferEnd = 0, GPS_HalfTransferEnd = 0;

		double  lat;        /**< Latitude in NDEG - +/-[degree][min].[sec/60] */
    double  lon;        /**< Longitude in NDEG - +/-[degree][min].[sec/60] */


		nmeaINFO GPS_info;          //GPS解码后得到的信息 
		nmeaPARSER parser;      //解码时使用的数据结构  
    uint8_t new_parse=0;    //是否有新的解码数据标志
  
    nmeaTIME beiJingTime;    //北京时间 



/**
  * @brief  GPS_Interrupt_Config 配置GPS使用的DMA中断 
  * @param  None.
  * @retval None.
  */
static void GPS_Interrupt_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// DMA2 Channel Interrupt ENABLE
	NVIC_InitStructure.NVIC_IRQChannel = GPS_DMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}


/**
  * @brief  GPS_ProcessDMAIRQ GPS DMA中断服务函数
  * @param  None.
  * @retval None.
  */
void GPS_ProcessDMAIRQ(void)
{
  
  if(DMA_GetITStatus(GPS_DMA_IT_HT) )         /* DMA 半传输完成 */
  {
    GPS_HalfTransferEnd = 1;                //设置半传输完成标志位
    DMA_ClearFlag(GPS_DMA_FLAG_HT);
  }
  else if(DMA_GetITStatus(GPS_DMA_IT_TC))     /* DMA 传输完成 */
  {
    GPS_TransferEnd = 1;                    //设置传输完成标志位
    DMA_ClearFlag(GPS_DMA_FLAG_TC);

   }
}


/**
  * @brief  GPS_DMA_Config gps dma接收配置
  * @param  无
  * @retval 无
  */
static void GPS_DMA_Config(void)
{
		DMA_InitTypeDef DMA_InitStructure;
	
		/*开启DMA时钟*/
		RCC_AHBPeriphClockCmd(GPS_DMA_CLK, ENABLE);

		/*设置DMA源：串口数据寄存器地址*/
		DMA_InitStructure.DMA_PeripheralBaseAddr = GPS_DATA_ADDR;	   

		/*内存地址(要传输的变量的指针)*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)gps_rbuff;

		/*方向：从内存到外设*/		
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	

		/*传输大小DMA_BufferSize=SENDBUFF_SIZE*/	
		DMA_InitStructure.DMA_BufferSize = GPS_RBUFF_SIZE;

		/*外设地址不增*/	    
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 

		/*内存地址自增*/
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	

		/*外设数据单位*/	
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;

		/*内存数据单位 8bit*/
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	 

		/*DMA模式：不断循环*/
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	 

		/*优先级：中*/	
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;  

		/*禁止内存到内存的传输	*/
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/*配置DMA的通道*/		   
		DMA_Init(GPS_DMA_CHANNEL, &DMA_InitStructure); 	   
    
    GPS_Interrupt_Config();
		
    DMA_ITConfig(GPS_DMA_CHANNEL,DMA_IT_HT|DMA_IT_TC,ENABLE);  //配置DMA发送完成后产生中断

		/*使能DMA*/
		DMA_Cmd (GPS_DMA_CHANNEL,ENABLE);		
    
    /* 配置串口 向 DMA发出TX请求 */
		USART_DMACmd(GPS_USART, USART_DMAReq_Rx, ENABLE);

    
}

/**
  * @brief  GPS_Config gps 初始化
  * @param  无
  * @retval 无
  */
void GPS_Config(unsigned int baud)
{
  GPS_USART_INIT(baud);
  GPS_DMA_Config();  

    /* 设置用于输出调试信息的函数 */
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;

    /* 初始化GPS数据结构 */
    nmea_zero_INFO(&GPS_info);
    nmea_parser_init(&parser);
  
}

 

/**
  * @brief  trace 在解码时输出捕获的GPS语句
  * @param  str: 要输出的字符串，str_size:数据长度
  * @retval 无
  */
void trace(const char *str, int str_size)
{
  #ifdef __GPS_DEBUG    //在gps_config.h文件配置这个宏，是否输出调试信息
    uint16_t i;
    printf("\r\nTrace: ");
    for(i=0;i<str_size;i++)
      printf("%c",*(str+i));
  
    printf("\n");
  #endif
}

/**
  * @brief  error 在解码出错时输出提示消息
  * @param  str: 要输出的字符串，str_size:数据长度
  * @retval 无
  */
void error(const char *str, int str_size)
{
    #ifdef __GPS_DEBUG   //在gps_config.h文件配置这个宏，是否输出调试信息

    uint16_t i;
    printf("\r\nError: ");
    for(i=0;i<str_size;i++)
      printf("%c",*(str+i));
    printf("\n");
    #endif
}



/******************************************************************************************************** 
**     函数名称:            bit        IsLeapYear(uint8_t    iYear) 
**    功能描述:            判断闰年(仅针对于2000以后的年份) 
**    入口参数：            iYear    两位年数 
**    出口参数:            uint8_t        1:为闰年    0:为平年 
********************************************************************************************************/ 
static uint8_t IsLeapYear(uint8_t iYear) 
{ 
    uint16_t    Year; 
    Year    =    2000+iYear; 
    if((Year&3)==0) 
    { 
        return ((Year%400==0) || (Year%100!=0)); 
    } 
     return 0; 
} 

/******************************************************************************************************** 
**     函数名称:            void    GMTconvert(uint8_t *DT,uint8_t GMT,uint8_t AREA) 
**    功能描述:            格林尼治时间换算世界各时区时间 
**    入口参数：            *DT:    表示日期时间的数组 格式 YY,MM,DD,HH,MM,SS 
**                        GMT:    时区数 
**                        AREA:    1(+)东区 W0(-)西区 
********************************************************************************************************/ 
void    GMTconvert(nmeaTIME *SourceTime, nmeaTIME *ConvertTime, uint8_t GMT,uint8_t AREA) 
{ 
    uint32_t    YY,MM,DD,hh,mm,ss;        //年月日时分秒暂存变量 
     
    if(GMT==0)    return;                //如果处于0时区直接返回 
    if(GMT>12)    return;                //时区最大为12 超过则返回         

    YY    =    SourceTime->year;                //获取年 
    MM    =    SourceTime->mon;                 //获取月 
    DD    =    SourceTime->day;                 //获取日 
    hh    =    SourceTime->hour;                //获取时 
    mm    =    SourceTime->min;                 //获取分 
    ss    =    SourceTime->sec;                 //获取秒 

    if(AREA)                        //东(+)时区处理 
    { 
        if(hh+GMT<24)    hh    +=    GMT;//如果与格林尼治时间处于同一天则仅加小时即可 
        else                        //如果已经晚于格林尼治时间1天则进行日期处理 
        { 
            hh    =    hh+GMT-24;        //先得出时间 
            if(MM==1 || MM==3 || MM==5 || MM==7 || MM==8 || MM==10)    //大月份(12月单独处理) 
            { 
                if(DD<31)    DD++; 
                else 
                { 
                    DD    =    1; 
                    MM    ++; 
                } 
            } 
            else if(MM==4 || MM==6 || MM==9 || MM==11)                //小月份2月单独处理) 
            { 
                if(DD<30)    DD++; 
                else 
                { 
                    DD    =    1; 
                    MM    ++; 
                } 
            } 
            else if(MM==2)    //处理2月份 
            { 
                if((DD==29) || (DD==28 && IsLeapYear(YY)==0))        //本来是闰年且是2月29日 或者不是闰年且是2月28日 
                { 
                    DD    =    1; 
                    MM    ++; 
                } 
                else    DD++; 
            } 
            else if(MM==12)    //处理12月份 
            { 
                if(DD<31)    DD++; 
                else        //跨年最后一天 
                {               
                    DD    =    1; 
                    MM    =    1; 
                    YY    ++; 
                } 
            } 
        } 
    } 
    else 
    {     
        if(hh>=GMT)    hh    -=    GMT;    //如果与格林尼治时间处于同一天则仅减小时即可 
        else                        //如果已经早于格林尼治时间1天则进行日期处理 
        { 
            hh    =    hh+24-GMT;        //先得出时间 
            if(MM==2 || MM==4 || MM==6 || MM==8 || MM==9 || MM==11)    //上月是大月份(1月单独处理) 
            { 
                if(DD>1)    DD--; 
                else 
                { 
                    DD    =    31; 
                    MM    --; 
                } 
            } 
            else if(MM==5 || MM==7 || MM==10 || MM==12)                //上月是小月份2月单独处理) 
            { 
                if(DD>1)    DD--; 
                else 
                { 
                    DD    =    30; 
                    MM    --; 
                } 
            } 
            else if(MM==3)    //处理上个月是2月份 
            { 
                if((DD==1) && IsLeapYear(YY)==0)                    //不是闰年 
                { 
                    DD    =    28; 
                    MM    --; 
                } 
                else    DD--; 
            } 
            else if(MM==1)    //处理1月份 
            { 
                if(DD>1)    DD--; 
                else        //新年第一天 
                {               
                    DD    =    31; 
                    MM    =    12; 
                    YY    --; 
                } 
            } 
        } 
    }         

    ConvertTime->year   =    YY;                //更新年 
    ConvertTime->mon    =    MM;                //更新月 
    ConvertTime->day    =    DD;                //更新日 
    ConvertTime->hour   =    hh;                //更新时 
    ConvertTime->min    =    mm;                //更新分 
    ConvertTime->sec    =    ss;                //更新秒 
}  


/**
  * @brief  nmea_decode_test 解码GPS模块信息
  * @param  无
  * @retval 无
  */
void nmea_decode_test(void)
{

////    nmeaINFO GPS_info;          //GPS解码后得到的信息 
////    nmeaPARSER parser;      //解码时使用的数据结构  
////    uint8_t new_parse=0;    //是否有新的解码数据标志
////  
////    nmeaTIME beiJingTime;    //北京时间 

////    /* 设置用于输出调试信息的函数 */
////    nmea_property()->trace_func = &trace;
////    nmea_property()->error_func = &error;

////    /* 初始化GPS数据结构 */
////    nmea_zero_INFO(&GPS_info);
////    nmea_parser_init(&parser);

      if(GPS_HalfTransferEnd)     /* 接收到GPS_RBUFF_SIZE一半的数据 */
      {
        /* 进行nmea格式解码 */
        nmea_parse(&parser, (const char*)&gps_rbuff[0], HALF_GPS_RBUFF_SIZE, &GPS_info);
        
        GPS_HalfTransferEnd = 0;   //清空标志位
        new_parse = 1;             //设置解码消息标志 
      }
      else if(GPS_TransferEnd)    /* 接收到另一半数据 */
      {

        nmea_parse(&parser, (const char*)&gps_rbuff[HALF_GPS_RBUFF_SIZE], HALF_GPS_RBUFF_SIZE, &GPS_info);
       
        GPS_TransferEnd = 0;
        new_parse =1;
      }
      
      if(new_parse )                //有新的解码消息   
      {    
        /* 对解码后的时间进行转换，转换成北京时间 */
        GMTconvert(&GPS_info.utc,&beiJingTime,8,1);
        
//        /* 输出解码得到的信息 */
//				UsartPrintf(USART_DEBUG,"/************************输出解码得到的信息************************/");
//        UsartPrintf(USART_DEBUG,"\r\n时间%d,%d,%d,%d,%d,%d", beiJingTime.year+1900, beiJingTime.mon+1,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);
//        UsartPrintf(USART_DEBUG,"\r\n纬度：%f,经度%f",GPS_info.lat,GPS_info.lon);
//        UsartPrintf(USART_DEBUG,"\r\n正在使用的卫星：%d,可见卫星：%d",GPS_info.satinfo.inuse,GPS_info.satinfo.inview);
//        UsartPrintf(USART_DEBUG,"\r\n海拔高度：%f 米 ", GPS_info.elv);
//        UsartPrintf(USART_DEBUG,"\r\n速度：%f km/h ", GPS_info.speed);
//        UsartPrintf(USART_DEBUG,"\r\n航向：%f 度\r\n", GPS_info.direction);
				
				//GPS数据转换
				lat = (int)GPS_info.lat/100;
				lon = (int)GPS_info.lon/100;
        lat = lat + (GPS_info.lat - lat*100)/60;
				lon = lon + (GPS_info.lon - lon*100)/60;
        new_parse = 0;
      }
	

    /* 释放GPS数据结构 */
//     nmea_parser_destroy(&parser);

    
    //  return 0;
}





/*********************************************************end of file**************************************************/
