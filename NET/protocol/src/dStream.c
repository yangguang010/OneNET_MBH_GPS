/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	dStream.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-02-28
	*
	*	版本： 		V1.0
	*
	*	说明： 		cJson格式数据流通用封装
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//协议封装文件
#include "dStream.h"

//C库
#include <string.h>
#include <stdio.h>





//==========================================================
//	函数名称：	DSTREAM_toString
//
//	函数功能：	将数值转为字符串
//
//	入口参数：	StreamArray：数据流
//				buf：转换后的缓存
//				pos：数据流中的哪个数据
//				bufLen：缓存长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void DSTREAM_toString(DATA_STREAM *streamArray, char *buf, unsigned short pos, unsigned short bufLen)
{
	
	memset(buf, 0, bufLen);

	switch((unsigned char)streamArray[pos].dataType)
	{
		case TYPE_BOOL:
			snprintf(buf, bufLen, "%d", *(_Bool *)streamArray[pos].data);
		break;
		
		case TYPE_CHAR:
			snprintf(buf, bufLen, "%d", *(signed char *)streamArray[pos].data);
		break;
		
		case TYPE_UCHAR:
			snprintf(buf, bufLen, "%d", *(unsigned char *)streamArray[pos].data);
		break;
		
		case TYPE_SHORT:
			snprintf(buf, bufLen, "%d", *(signed short *)streamArray[pos].data);
		break;
		
		case TYPE_USHORT:
			snprintf(buf, bufLen, "%d", *(unsigned short *)streamArray[pos].data);
		break;
		
		case TYPE_INT:
			snprintf(buf, bufLen, "%d", *(signed int *)streamArray[pos].data);
		break;
		
		case TYPE_UINT:
			snprintf(buf, bufLen, "%d", *(unsigned int *)streamArray[pos].data);
		break;
		
		case TYPE_LONG:
			snprintf(buf, bufLen, "%ld", *(signed long *)streamArray[pos].data);
		break;
		
		case TYPE_ULONG:
			snprintf(buf, bufLen, "%ld", *(unsigned long *)streamArray[pos].data);
		break;
			
		case TYPE_FLOAT:
			snprintf(buf, bufLen, "%f", *(float *)streamArray[pos].data);
		break;
		
		case TYPE_DOUBLE:
			snprintf(buf, bufLen, "%f", *(double *)streamArray[pos].data);
		break;
		
		case TYPE_GPS:
			snprintf(buf, bufLen, "{\"lon\":%s,\"lat\":%s}", (char *)streamArray[pos].data, (char *)(streamArray[pos].data) + 16);
		break;
		
		case TYPE_STRING:
			snprintf(buf, bufLen, "\"%s\"", (char *)streamArray[pos].data);
		break;
	}

}

//==========================================================
//	函数名称：	 DSTREAM_GetDataStream_Body
//
//	函数功能：	获取数据流格式消息体
//
//	入口参数：	type：格式类型
//				buffer：缓存
//				maxLen：最大缓存长度
//
//	返回参数：	Body的长度，0-失败
//
//	说明：		
//==========================================================
short DSTREAM_GetDataStream_Body(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, char *buffer, short maxLen)
{
	
	short count = 0, numBytes = 0;								//count-循环计数。numBytes-记录数据装载长度
	char stream_buf[96];
	char data_buf[48];
	short cBytes = 0;
	
	maxLen -= 1;												//预留结束符位置

	switch(type)
	{
		case FORMAT_TYPE1:
		
			if(numBytes + 16 < maxLen)
			{
				memcpy(buffer, "{\"datastreams\":[", 16);
				numBytes += 16;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag)						//如果使能发送标志位
				{
					memset(stream_buf, 0, sizeof(stream_buf));

					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]},", streamArray[count].name, data_buf);
					
					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes)
					{
						//UsartPrintf(USART_DEBUG, "dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(buffer + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
			
			buffer[numBytes--] = '\0';							//将最后的','替换为结束符
			
			if(numBytes + 2 <= maxLen)
			{
				memcpy(buffer + numBytes, "]}", 2);
				numBytes += 2;
			}
			else
				return 0;
		
		break;
		
		case FORMAT_TYPE3:
			
			if(numBytes + 1 < maxLen)
			{
				memcpy(buffer, "{", 1);
				numBytes++;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) 						//如果使能发送标志位
				{
					memset(stream_buf, 0, sizeof(stream_buf));

					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":%s,", streamArray[count].name, data_buf);

					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes)
					{
						//UsartPrintf(USART_DEBUG, "dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(buffer + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
			
			buffer[numBytes--] = '\0';							//将最后的','替换为结束符

			memcpy(buffer + numBytes, "}", 1);
			numBytes += 1;
		
		break;
		
		case FORMAT_TYPE4:
			
			if(numBytes + 1 < maxLen)
			{
				memcpy(buffer, "{", 1);
				numBytes++;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) 						//如果使能发送标志位
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":{\"2016-08-10T12:31:17\":%s},", streamArray[count].name, data_buf);
					
					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes)
					{
						//UsartPrintf(USART_DEBUG, "dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(buffer + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
			
			buffer[numBytes--] = '\0';							//将最后的','替换为结束符

			memcpy(buffer + numBytes, "}", 1);
			numBytes += 1;
		
		break;
		
		case FORMAT_TYPE5:
		
			if(numBytes + 2 < maxLen)
			{
				memcpy(buffer, ",;", 2);
				numBytes += 2;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag && streamArray[count].dataType != TYPE_GPS)	//如果使能发送标志位 格式5不支持GPS
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "%s,%s;", streamArray[count].name, data_buf);
					
					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes - 2)
					{
						//UsartPrintf(USART_DEBUG, "dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(buffer + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
		
		break;
		
		default:
		break;
	}
	
	//UsartPrintf(USART_DEBUG, "Body Len: %d\r\n", numBytes);
	return numBytes;

}

//==========================================================
//	函数名称：	 DSTREAM_GetDataStream_Body_Measure
//
//	函数功能：	测量数据流格式消息体长度
//
//	入口参数：	type：格式类型
//
//	返回参数：	Body的长度
//
//	说明：		
//==========================================================
short DSTREAM_GetDataStream_Body_Measure(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{

	short count = 0, numBytes = 0;						//count-循环计数。numBytes-记录数据装载长度
	char stream_buf[96];
	char data_buf[32];

	switch(type)
	{
		case FORMAT_TYPE1:
			
			numBytes += 16;
			
			for(; count < streamArrayCnt; count++)
			{
				memset(stream_buf, 0, sizeof(stream_buf));

				DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
				snprintf(stream_buf, sizeof(stream_buf), "{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]},", streamArray[count].name, data_buf);
				
				numBytes += strlen(stream_buf);
			}
			
			numBytes += 2;
		
		break;
		
		case FORMAT_TYPE3:
			
			numBytes++;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) //如果使能发送标志位
				{
					memset(stream_buf, 0, sizeof(stream_buf));

					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":%s,", streamArray[count].name, data_buf);

					numBytes += strlen(stream_buf);
				}
			}
			
			numBytes += 1;
		
		break;
		
		case FORMAT_TYPE4:
			
			numBytes++;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) //如果使能发送标志位
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":{\"2016-08-10T12:31:17\":%s},", streamArray[count].name, data_buf);
					
					numBytes += strlen(stream_buf);
				}
			}
			
			numBytes += 1;
		
		break;
		
		case FORMAT_TYPE5:
			
			numBytes += 2;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) //如果使能发送标志位
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "%s,%s;", streamArray[count].name, data_buf);
					
					numBytes += strlen(stream_buf);
				}
			}
		
		break;
		
		default:
		break;
	}
	
	return numBytes;

}
