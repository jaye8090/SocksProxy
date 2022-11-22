#include "StdAfx.h"

#include "UserPtl.h"
#include "../DataProcess/DataPack.h"
#include "../DataProcess/DataParse.h"

CUserPtl::CUserPtl(UserPtl::PFNPutTip pfnPutTip, void * pThis)
{
	m_pfnPutTip = pfnPutTip;
	m_pThis = pThis;
}

CUserPtl::~CUserPtl(void)
{
}

/******************************************************************************
��ȡSOCKS�汾
******************************************************************************/
unsigned char CUserPtl::GetSocksVer(void * pData, unsigned int uiDataSize)
{
	if (pData == NULL
		|| uiDataSize < sizeof(unsigned char))
	{
		_PutTip("", UserPtl::GET_SOCKS_VER_DATA_INVALID);
		return 0;
	}
	unsigned char * pucData = (unsigned char *)pData;
	unsigned char ucSocksVer = *(unsigned char *)pucData;

	if (ucSocksVer != 0x04
		&& ucSocksVer != 0x05)
	{
		_PutTip("��Ч��SOCKS��������", UserPtl::GET_SOCKS_VER_VER_INVALID);
		return 0;
	}

	return ucSocksVer;
}

/******************************************************************************
������֤�����
******************************************************************************/
bool CUserPtl::ParseAuthRequest(void * pData, unsigned int uiDataSize)
{
	//��������
	CDataParse parse;
	//��������
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserPtl::PARSE_AUTH_REQUEST_DATA_INVALID);
		return false;
	}
	parse.SetData(pData, uiDataSize);
	//�汾
	parse.Skip(1);
	//������
	unsigned char ucMethodCnt = 0;
	parse.Pop(ucMethodCnt);
	//�����б�
	unsigned char * pMethodList = NULL;
	parse.Pop(pMethodList, ucMethodCnt);
	//�����Ƿ�ɹ�
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_AUTH_REQUEST_PARSE_FAIL);
		return false;
	}

	//�жϷ����б����Ƿ�֧��������֤
	bool bNoAuth = false;
	if (pMethodList == NULL)
	{
		_PutTip("", UserPtl::PARSE_AUTH_REQUEST_METHOD_LIST_NULL);
		return false;
	}
	for (int i = 0; i < ucMethodCnt; i++)
	{
		if (pMethodList[i] == 0x00)
		{
			bNoAuth = true;
			break;
		}
	}
	if (!bNoAuth)
	{
		_PutTip("", UserPtl::PARSE_AUTH_REQUEST_AUTH_METHOD_INVALID);
		return false;
	}

	return true;
}

/******************************************************************************
�����֤��Ӧ��
******************************************************************************/
void CUserPtl::PackAuthResponse(CDataPack & pack)
{
	//�汾
	pack.Push("\x05", 1);
	//������֤
	pack.Push("\x00", 1);
}

/******************************************************************************
����SOCKS5���������
******************************************************************************/
bool CUserPtl::ParseSocks5ConnRequest(unsigned long & ulDstIP
	, unsigned short & usDstPort, void * pData, unsigned int uiDataSize)
{
	//��������
	CDataParse parse;
	//��������
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_DATA_INVALID);
		return false;
	}
	parse.SetData(pData, uiDataSize);
	//�汾
	parse.Skip(1);
	//����
	unsigned char ucCmd = 0;
	parse.Pop(ucCmd);
	//�����Ƿ�ɹ�
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_1);
		return false;
	}
	//ֻ֧����������
	if (ucCmd != 0x01)
	{
		_PutTip("��Ч��SOCKS��������"
			, UserPtl::PARSE_SOCKS5_CONN_REQUEST_CMD_INVALID);
		return false;
	}
	//����
	parse.Skip(1);
	//��ַ����
	unsigned char ucAddrType = 0;
	parse.Pop(ucAddrType);
	//�����Ƿ�ɹ�
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_2);
		return false;
	}
	//ֻ֧��IPv4
	if (ucAddrType != 0x01)
	{
		_PutTip("��Ч��SOCKS��������"
			, UserPtl::PARSE_SOCKS5_CONN_REQUEST_ADDR_INVALID);
		return false;
	}
	//Ŀ��IP
	parse.Pop(ulDstIP);
	//Ŀ��˿�
	parse.PopReverse(usDstPort);
	//�����Ƿ�ɹ�
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_3);
		return false;
	}

	return true;
}

/******************************************************************************
����SOCKS4���������
******************************************************************************/
bool CUserPtl::ParseSocks4ConnRequest(unsigned long & ulDstIP
	, unsigned short & usDstPort, void * pData, unsigned int uiDataSize)
{
	//��������
	CDataParse parse;
	//��������
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserPtl::PARSE_SOCKS4_CONN_REQUEST_DATA_INVALID);
		return false;
	}
	parse.SetData(pData, uiDataSize);
	//�汾
	parse.Skip(1);
	//����
	unsigned char ucCmd = 0;
	parse.Pop(ucCmd);
	//�����Ƿ�ɹ�
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS4_CONN_REQUEST_PARSE_FAIL_1);
		return false;
	}
	//ֻ֧����������
	if (ucCmd != 0x01)
	{
		_PutTip("��Ч��SOCKS��������"
			, UserPtl::PARSE_SOCKS4_CONN_REQUEST_CMD_INVALID);
		return false;
	}
	//Ŀ��˿�
	parse.PopReverse(usDstPort);
	//Ŀ��IP
	parse.Pop(ulDstIP);
	//�����Ƿ�ɹ�
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS4_CONN_REQUEST_PARSE_FAIL_2);
		return false;
	}

	return true;
}

/******************************************************************************
���SOCKS5������Ӧ��
******************************************************************************/
void CUserPtl::PackSocks5ConnResponse(CDataPack & pack)
{
	//�汾
	pack.Push("\x05", 1);
	//�ɹ�
	pack.Push("\x00", 1);
	//����
	pack.Push("\x00", 1);
	//IPv4
	pack.Push("\x01", 1);
	//IP
	pack.Push("\x00\x00\x00\x00", 4);
	//�˿�
	pack.Push("\x00\x00", 2);
}

/******************************************************************************
���SOCKS4������Ӧ��
******************************************************************************/
void CUserPtl::PackSocks4ConnResponse(CDataPack & pack)
{
	//�汾
	pack.Push("\x00", 1);
	//�ɹ�
	pack.Push("\x5A", 1);
	//�˿�
	pack.Push("\x00\x00", 2);
	//IP
	pack.Push("\x00\x00\x00\x00", 4);
}

/******************************************************************************
�����ʾ
******************************************************************************/
void CUserPtl::_PutTip(string strTip, int iErr, string strErr)
{
	//�����ʾ�ص�
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "UserPtl", iErr, strErr, m_pThis);
}