#include "StdAfx.h"

#include "TimeCnt.h"

CTimeCnt::CTimeCnt(void)
{
}

CTimeCnt::~CTimeCnt(void)
{
}

/******************************************************************************
开始
******************************************************************************/
void CTimeCnt::Begin(void)
{
	//记录开始时间
	m_iBegin = GetTickCount();
}

/******************************************************************************
结束
******************************************************************************/
unsigned int CTimeCnt::End(void)
{
	//获取结束时间
	__int64 iEnd = GetTickCount();
	//时间溢出
	if (iEnd < m_iBegin)
	{
		m_iBegin -= ULONG_MAX;
		m_iBegin--;
	}

	//返回消耗的时间
	return unsigned int(iEnd - m_iBegin);
}