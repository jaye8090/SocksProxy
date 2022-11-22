#include "StdAfx.h"

#include "DataPack.h"

CDataPack::CDataPack(DataPack::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;

	m_pDataBuf = NULL;
	m_uiBufSize = 0;
	m_uiDataSize = 0;
}

CDataPack::~CDataPack(void)
{
	Clear();
}

/******************************************************************************
压入数据
******************************************************************************/
void CDataPack::Push(void * pPushData, unsigned int uiPushSize)
{
	//申请数据缓冲区
	_AllocDataBuf(uiPushSize);
	//拷贝数据
	_CopyData(pPushData, uiPushSize);
}

/******************************************************************************
压入数据
******************************************************************************/
void CDataPack::Push(CDataPack & data)
{
	//获取数据
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	data.GetData(pData, uiDataSize);

	//压入数据
	Push(pData, uiDataSize);
}

/******************************************************************************
先反向压入2字节的数据大小，再压入数据
******************************************************************************/
void CDataPack::PushWithSize(void * pPushData, unsigned int uiPushSize)
{
	//反向压入2字节的数据大小
	PushReverse((unsigned short)uiPushSize);
	//压入数据
	Push(pPushData, uiPushSize);
}

/******************************************************************************
先反向压入2字节的数据大小，再压入数据
******************************************************************************/
void CDataPack::PushWithSize(CDataPack & data)
{
	//获取数据
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	data.GetData(pData, uiDataSize);

	//先反向压入2字节的数据大小，再压入数据
	PushWithSize(pData, uiDataSize);
}

/******************************************************************************
反向压入数据
******************************************************************************/
void CDataPack::PushReverse(unsigned short usData)
{
	//反向
	usData = htons(usData);
	//压入数据
	Push(&usData, sizeof(usData));
}

/******************************************************************************
反向压入数据
******************************************************************************/
void CDataPack::PushReverse(unsigned int uiData)
{
	//反向
	uiData = htonl(uiData);
	//压入数据
	Push(&uiData, sizeof(uiData));
}

/******************************************************************************
获取数据
******************************************************************************/
void CDataPack::GetData(unsigned char * & pData, unsigned int & uiDataSize)
{
	pData = m_pDataBuf;
	uiDataSize = m_uiDataSize;
}

/******************************************************************************
数据是否为空
******************************************************************************/
bool CDataPack::IsEmpty(void)
{
	return m_uiDataSize == 0;
}

/******************************************************************************
清空数据
******************************************************************************/
void CDataPack::Clear(void)
{
	if (m_pDataBuf != NULL)
	{
		free(m_pDataBuf);
		m_pDataBuf = NULL;
	}
	m_uiBufSize = 0;
	m_uiDataSize = 0;
}

/******************************************************************************
申请数据缓冲区
******************************************************************************/
void CDataPack::_AllocDataBuf(unsigned int uiAllocSize)
{
	//缓冲区不够，则申请
	if (m_uiBufSize < m_uiDataSize + uiAllocSize)
	{
		//最少申请的大小
		if (uiAllocSize < DataPack::ALLOC_BUF_SIZE)
		{
			uiAllocSize = DataPack::ALLOC_BUF_SIZE;
		}

		//申请内存
		unsigned char * pDataBuf = (unsigned char *)
			realloc(m_pDataBuf, m_uiBufSize + uiAllocSize);
		if (pDataBuf == NULL)
		{
			_PutTip("", DataPack::ALLOC_DATA_BUF_ALLOC_FAIL);
			return;
		}

		m_pDataBuf = pDataBuf;
		m_uiBufSize += uiAllocSize;
	}
}

/******************************************************************************
拷贝数据
******************************************************************************/
void CDataPack::_CopyData(void * pPushData, unsigned int uiPushSize)
{
	if (m_pDataBuf == NULL)
	{
		_PutTip("", DataPack::COPY_DATA_DATA_BUF_NULL);
		return;
	}
	if (m_uiDataSize < 0)
	{
		_PutTip("", DataPack::COPY_DATA_DATA_SIZE_INVALID);
		return;
	}
	if (pPushData == NULL
		|| uiPushSize <= 0)
	{
		_PutTip("", DataPack::COPY_DATA_PUSH_DATA_INVALID);
		return;
	}
	if (m_uiDataSize + uiPushSize > m_uiBufSize)
	{
		_PutTip("", DataPack::COPY_DATA_BUF_SIZE_SMALL);
		return;
	}

	memcpy(m_pDataBuf + m_uiDataSize, pPushData, uiPushSize);
	m_uiDataSize += uiPushSize;
}

/******************************************************************************
输出提示
******************************************************************************/
void CDataPack::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "DataPack", iErr, strErr, m_pThis);
}