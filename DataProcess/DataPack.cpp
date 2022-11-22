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
ѹ������
******************************************************************************/
void CDataPack::Push(void * pPushData, unsigned int uiPushSize)
{
	//�������ݻ�����
	_AllocDataBuf(uiPushSize);
	//��������
	_CopyData(pPushData, uiPushSize);
}

/******************************************************************************
ѹ������
******************************************************************************/
void CDataPack::Push(CDataPack & data)
{
	//��ȡ����
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	data.GetData(pData, uiDataSize);

	//ѹ������
	Push(pData, uiDataSize);
}

/******************************************************************************
�ȷ���ѹ��2�ֽڵ����ݴ�С����ѹ������
******************************************************************************/
void CDataPack::PushWithSize(void * pPushData, unsigned int uiPushSize)
{
	//����ѹ��2�ֽڵ����ݴ�С
	PushReverse((unsigned short)uiPushSize);
	//ѹ������
	Push(pPushData, uiPushSize);
}

/******************************************************************************
�ȷ���ѹ��2�ֽڵ����ݴ�С����ѹ������
******************************************************************************/
void CDataPack::PushWithSize(CDataPack & data)
{
	//��ȡ����
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	data.GetData(pData, uiDataSize);

	//�ȷ���ѹ��2�ֽڵ����ݴ�С����ѹ������
	PushWithSize(pData, uiDataSize);
}

/******************************************************************************
����ѹ������
******************************************************************************/
void CDataPack::PushReverse(unsigned short usData)
{
	//����
	usData = htons(usData);
	//ѹ������
	Push(&usData, sizeof(usData));
}

/******************************************************************************
����ѹ������
******************************************************************************/
void CDataPack::PushReverse(unsigned int uiData)
{
	//����
	uiData = htonl(uiData);
	//ѹ������
	Push(&uiData, sizeof(uiData));
}

/******************************************************************************
��ȡ����
******************************************************************************/
void CDataPack::GetData(unsigned char * & pData, unsigned int & uiDataSize)
{
	pData = m_pDataBuf;
	uiDataSize = m_uiDataSize;
}

/******************************************************************************
�����Ƿ�Ϊ��
******************************************************************************/
bool CDataPack::IsEmpty(void)
{
	return m_uiDataSize == 0;
}

/******************************************************************************
�������
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
�������ݻ�����
******************************************************************************/
void CDataPack::_AllocDataBuf(unsigned int uiAllocSize)
{
	//������������������
	if (m_uiBufSize < m_uiDataSize + uiAllocSize)
	{
		//��������Ĵ�С
		if (uiAllocSize < DataPack::ALLOC_BUF_SIZE)
		{
			uiAllocSize = DataPack::ALLOC_BUF_SIZE;
		}

		//�����ڴ�
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
��������
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
�����ʾ
******************************************************************************/
void CDataPack::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "DataPack", iErr, strErr, m_pThis);
}