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
设置数据
******************************************************************************/
void CDataParse::SetData(void * pData, unsigned int uiDataSize)
{
	m_pData = (unsigned char *)pData;
	m_uiDataSize = uiDataSize;
	m_uiReadSize = 0;
}

/******************************************************************************
回退数据
******************************************************************************/
void CDataParse::BackData(void)
{
	m_uiReadSize = 0;
}

/******************************************************************************
弹出数据
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
弹出数据
******************************************************************************/
void CDataParse::Pop(unsigned char & ucData)
{
	//弹出数据
	unsigned char * pucData = NULL;
	Pop(pucData, sizeof(ucData));

	//转换数据
	if (pucData == NULL)
	{
		m_bParseSucc = false;
		return;
	}
	ucData = *(unsigned char *)pucData;
}

/******************************************************************************
弹出数据
******************************************************************************/
void CDataParse::Pop(unsigned short & usData)
{
	//弹出数据
	unsigned char * pucData = NULL;
	Pop(pucData, sizeof(usData));

	//转换数据
	if (pucData == NULL)
	{
		m_bParseSucc = false;
		return;
	}
	usData = *(unsigned short *)pucData;
}

/******************************************************************************
弹出数据
******************************************************************************/
void CDataParse::Pop(unsigned int & uiData)
{
	//弹出数据
	unsigned char * pucData = NULL;
	Pop(pucData, sizeof(uiData));

	//转换数据
	if (pucData == NULL)
	{
		m_bParseSucc = false;
		return;
	}
	uiData = *(unsigned int *)pucData;
}

/******************************************************************************
弹出数据
******************************************************************************/
void CDataParse::Pop(unsigned long & ulData)
{
	//弹出数据
	unsigned int uiData = 0;
	Pop(uiData);
	ulData = uiData;
}

/******************************************************************************
弹出数据
******************************************************************************/
void CDataParse::Pop(string & strData)
{
	//先反向弹出2字节的数据大小，再根据数据大小弹出数据
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	PopWithSize(pData, uiDataSize);

	//转换成字符串
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		m_bParseSucc = false;
		return;
	}
	strData.assign((char *)pData, uiDataSize);
}

/******************************************************************************
先反向弹出2字节的数据大小，再根据数据大小弹出数据
******************************************************************************/
void CDataParse::PopWithSize(
	unsigned char * & pData, unsigned int & uiDataSize)
{
	//反向弹出2字节的数据大小
	unsigned short usDataSize = 0;
	PopReverse(usDataSize);
	uiDataSize = usDataSize;

	//弹出数据
	Pop(pData, uiDataSize);
}

/******************************************************************************
先反向弹出2字节的数据大小，再根据数据大小弹出数据
******************************************************************************/
void CDataParse::PopWithSize(CDataParse & parse)
{
	//先反向弹出2字节的数据大小，再根据数据大小弹出数据
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	PopWithSize(pData, uiDataSize);

	//设置数据
	parse.SetData(pData, uiDataSize);
}

/******************************************************************************
先反向弹出2字节的数据大小，再根据数据大小弹出数据
******************************************************************************/
void CDataParse::PopWithSize(CDataPack & data)
{
	//先反向弹出2字节的数据大小，再根据数据大小弹出数据
	unsigned char * pData = NULL;
	unsigned int uiDataSize = 0;
	PopWithSize(pData, uiDataSize);

	//压入数据
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		m_bParseSucc = false;
		return;
	}
	data.Push(pData, uiDataSize);
}

/******************************************************************************
反向弹出数据
******************************************************************************/
void CDataParse::PopReverse(unsigned short & usData)
{
	//弹出数据
	Pop(usData);
	//反向
	usData = ntohs(usData);
}

/******************************************************************************
反向弹出数据
******************************************************************************/
void CDataParse::PopReverse(unsigned int & uiData)
{
	//弹出数据
	Pop(uiData);
	//反向
	uiData = ntohl(uiData);
}

/******************************************************************************
跳过数据
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
先反向弹出2字节的数据大小，再根据数据大小跳过数据
******************************************************************************/
void CDataParse::SkipWithSize(void)
{
	//反向弹出2字节的数据大小
	unsigned short usDataSize = 0;
	PopReverse(usDataSize);

	//跳过数据
	Skip(usDataSize);
}

/******************************************************************************
解析是否成功
******************************************************************************/
bool CDataParse::IsSucc(void)
{
	return m_bParseSucc;
}

/******************************************************************************
解析是否完成
******************************************************************************/
bool CDataParse::IsDone(void)
{
	return m_uiReadSize == m_uiDataSize;
}