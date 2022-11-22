#include "StdAfx.h"

#include "TimeCnt.h"

CTimeCnt::CTimeCnt(void)
{
}

CTimeCnt::~CTimeCnt(void)
{
}

/******************************************************************************
��ʼ
******************************************************************************/
void CTimeCnt::Begin(void)
{
	//��¼��ʼʱ��
	m_iBegin = GetTickCount();
}

/******************************************************************************
����
******************************************************************************/
unsigned int CTimeCnt::End(void)
{
	//��ȡ����ʱ��
	__int64 iEnd = GetTickCount();
	//ʱ�����
	if (iEnd < m_iBegin)
	{
		m_iBegin -= ULONG_MAX;
		m_iBegin--;
	}

	//�������ĵ�ʱ��
	return unsigned int(iEnd - m_iBegin);
}