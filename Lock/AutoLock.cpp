#include "StdAfx.h"

#include "AutoLock.h"

CAutoLock::CAutoLock(CSyncObject * pLock
	, AutoLock::PFNPutTip pfnPutTip, void * pThis)
{
	m_pLock = pLock;
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;

	if (m_pLock == NULL)
	{
		_PutTip("", AutoLock::CONSTRUCT_LOCK_NULL);
		return;
	}
	try
	{
		if (!m_pLock->Lock())
		{
			_PutTip("", AutoLock::CONSTRUCT_LOCK_FAIL);
			return;
		}
	}
	catch (...)
	{
		_PutTip("", AutoLock::CONSTRUCT_LOCK_EXCEPTION);
		return;
	}
}

CAutoLock::~CAutoLock(void)
{
	if (m_pLock == NULL)
	{
		_PutTip("", AutoLock::DESTRUCT_LOCK_NULL);
		return;
	}
	if (!m_pLock->Unlock())
	{
		_PutTip("", AutoLock::DESTRUCT_UNLOCK_FAIL);
		return;
	}
}

/******************************************************************************
输出提示
******************************************************************************/
void CAutoLock::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "AutoLock", iErr, strErr, m_pThis);
}