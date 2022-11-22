#pragma once

#include <string>
using namespace std;

#include "DataPackDef.h"

/******************************************************************************
数据打包类
功能：
	实现数据的打包操作
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2013-11-22 到 2016-1-4
******************************************************************************/
class CDataPack
{
public:
	CDataPack(DataPack::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CDataPack(void);

//基本接口
public:
	/**************************************************************************
	压入数据
	参数：
		pPushData		压入数据
		uiPushSize		压入大小
	**************************************************************************/
	void Push(void * pPushData, unsigned int uiPushSize);

	/**************************************************************************
	压入数据
	参数：
		data		数据
	**************************************************************************/
	void Push(CDataPack & data);

	/**************************************************************************
	先反向压入2字节的数据大小，再压入数据
	参数：
		pPushData		压入数据
		uiPushSize		压入大小
	**************************************************************************/
	void PushWithSize(void * pPushData, unsigned int uiPushSize);

	/**************************************************************************
	先反向压入2字节的数据大小，再压入数据
	参数：
		data		数据
	**************************************************************************/
	void PushWithSize(CDataPack & data);

	/**************************************************************************
	反向压入数据
	参数：
		usData		数据
	**************************************************************************/
	void PushReverse(unsigned short usData);

	/**************************************************************************
	反向压入数据
	参数：
		uiData		数据
	**************************************************************************/
	void PushReverse(unsigned int uiData);

	/**************************************************************************
	获取数据
	参数：
		pData			输出值。数据指针。
						调用Clear、对象析构后，此内存将被释放。

		uiDataSize		输出值。数据大小。
	**************************************************************************/
	void GetData(unsigned char * & pData, unsigned int & uiDataSize);

	/**************************************************************************
	数据是否为空
	返回：
		数据为空返回true，否则返回false。
	**************************************************************************/
	bool IsEmpty(void);

	/**************************************************************************
	清空数据
	**************************************************************************/
	void Clear(void);

//内部操作
private:
	/**************************************************************************
	申请数据缓冲区
	参数：
		uiAllocSize		申请大小
	**************************************************************************/
	void _AllocDataBuf(unsigned int uiAllocSize);

	/**************************************************************************
	拷贝数据
	参数：
		pPushData		压入数据
		uiPushSize		压入大小
	**************************************************************************/
	void _CopyData(void * pPushData, unsigned int uiPushSize);

	/**************************************************************************
	输出提示
	参数：
		strTip		提示
		iErr		错误码
		strErr		错误串
	**************************************************************************/
	void _PutTip(string strTip, int iErr = 0, string strErr = "");

//内部数据
private:
	DataPack::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void					* m_pThis;			//回调this指针

	unsigned char		* m_pDataBuf;		//数据缓冲区指针
	unsigned int		m_uiBufSize;		//缓冲区大小
	unsigned int		m_uiDataSize;		//数据大小
};