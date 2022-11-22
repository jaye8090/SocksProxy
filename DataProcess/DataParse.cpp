#include "StdAfx.h"

#include "DataParse.h"
#include "DataPack.h"

CDataParse::CDataParse(void)
{
	m_pData = NULL;
	m_uiDataSize = 0;
	m_uiReadSize = 0;

	m_bParseSucc = true;
}

CDataParse::~CDataParse(void)
{
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::SetData(void * pData, unsigned int uiDataSize)
{
	m_pData = (unsigned char *)pData;
	m_uiDataSize = uiDataSize;
	m_uiReadSize = 0;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::BackData(void)
{
	m_uiReadSize = 0;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Pop(unsigned char * & pData, unsigned int uiDataSize)
{
	if (m_pData == NULL
		|| uiDataSize <= 0
		|| m_uiReadSize + uiDataSize > m_uiDataSize)
	{
		m_bParseSucc = false;
		return;
	}

	pData = m_pData + m_uiReadSize;
	m_uiReadSize += uiDataSize;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Pop(unsigned char & ucData)
{
	//��������
	unsigned char * pucData = NULL;
	Pop(pucData, sizeof(ucData));

	//ת������
	if (pucData == NULL)
	{
		m_bParseSucc = false;
		return;
	}
	ucData = *(unsigned char *)pucData;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Pop(unsigned short & usData)
{
	//��������
	unsigned char * pucData = NULL;
	Pop(pucData, sizeof(usData));

	//ת������
	if (pucData == NULL)
	{
		m_bParseSucc = false;
		return;
	}
	usData = *(unsigned short *)pucData;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Pop(unsigned int & uiData)
{
	//��������
	unsigned char * pucData = NULL;
	Pop(pucData, sizeof(uiData));

	//ת������
	if (pucData == NULL)
	{
		m_bParseSucc = false;
		return;
	}
	uiData = *(unsigned int *)pucData;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Pop(unsigned long & ulData)
{
	//��������
	unsigned int uiData = 0;
	Pop(uiData);
	ulData = uiData;
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Pop(string & strData)
{
	//�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	PopWithSize(pData, uiDataSize);

	//ת�����ַ���
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		m_bParseSucc = false;
		return;
	}
	strData.assign((char *)pData, uiDataSize);
}

/******************************************************************************
�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
******************************************************************************/
void CDataParse::PopWithSize(
	unsigned char * & pData, unsigned int & uiDataSize)
{
	//���򵯳�2�ֽڵ����ݴ�С
	unsigned short usDataSize = 0;
	PopReverse(usDataSize);
	uiDataSize = usDataSize;

	//��������
	Pop(pData, uiDataSize);
}

/******************************************************************************
�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
******************************************************************************/
void CDataParse::PopWithSize(CDataParse & parse)
{
	//�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	PopWithSize(pData, uiDataSize);

	//��������
	parse.SetData(pData, uiDataSize);
}

/******************************************************************************
�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
******************************************************************************/
void CDataParse::PopWithSize(CDataPack & data)
{
	//�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	PopWithSize(pData, uiDataSize);

	//ѹ������
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		m_bParseSucc = false;
		return;
	}
	data.Push(pData, uiDataSize);
}

/******************************************************************************
���򵯳�����
******************************************************************************/
void CDataParse::PopReverse(unsigned short & usData)
{
	//��������
	Pop(usData);
	//����
	usData = ntohs(usData);
}

/******************************************************************************
���򵯳�����
******************************************************************************/
void CDataParse::PopReverse(unsigned int & uiData)
{
	//��������
	Pop(uiData);
	//����
	uiData = ntohl(uiData);
}

/******************************************************************************
��������
******************************************************************************/
void CDataParse::Skip(unsigned int uiSkipSize)
{
	if (uiSkipSize <= 0
		|| m_uiReadSize + uiSkipSize > m_uiDataSize)
	{
		m_bParseSucc = false;
		return;
	}

	m_uiReadSize += uiSkipSize;
}

/******************************************************************************
�ȷ��򵯳�2�ֽڵ����ݴ�С���ٸ������ݴ�С��������
******************************************************************************/
void CDataParse::SkipWithSize(void)
{
	//���򵯳�2�ֽڵ����ݴ�С
	unsigned short usDataSize = 0;
	PopReverse(usDataSize);

	//��������
	Skip(usDataSize);
}

/******************************************************************************
�����Ƿ�ɹ�
******************************************************************************/
bool CDataParse::IsSucc(void)
{
	return m_bParseSucc;
}

/******************************************************************************
�����Ƿ����
******************************************************************************/
bool CDataParse::IsDone(void)
{
	return m_uiReadSize == m_uiDataSize;
}