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
获取SOCKS版本
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
		_PutTip("无效的SOCKS代理请求！", UserPtl::GET_SOCKS_VER_VER_INVALID);
		return 0;
	}

	return ucSocksVer;
}

/******************************************************************************
解析认证请求包
******************************************************************************/
bool CUserPtl::ParseAuthRequest(void * pData, unsigned int uiDataSize)
{
	//解析数据
	CDataParse parse;
	//设置数据
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserPtl::PARSE_AUTH_REQUEST_DATA_INVALID);
		return false;
	}
	parse.SetData(pData, uiDataSize);
	//版本
	parse.Skip(1);
	//方法数
	unsigned char ucMethodCnt = 0;
	parse.Pop(ucMethodCnt);
	//方法列表
	unsigned char * pMethodList = NULL;
	parse.Pop(pMethodList, ucMethodCnt);
	//解析是否成功
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_AUTH_REQUEST_PARSE_FAIL);
		return false;
	}

	//判断方法列表中是否支持无需认证
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
打包认证响应包
******************************************************************************/
void CUserPtl::PackAuthResponse(CDataPack & pack)
{
	//版本
	pack.Push("\x05", 1);
	//无需认证
	pack.Push("\x00", 1);
}

/******************************************************************************
解析SOCKS5连接请求包
******************************************************************************/
bool CUserPtl::ParseSocks5ConnRequest(unsigned long & ulDstIP
	, unsigned short & usDstPort, void * pData, unsigned int uiDataSize)
{
	//解析数据
	CDataParse parse;
	//设置数据
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_DATA_INVALID);
		return false;
	}
	parse.SetData(pData, uiDataSize);
	//版本
	parse.Skip(1);
	//命令
	unsigned char ucCmd = 0;
	parse.Pop(ucCmd);
	//解析是否成功
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_1);
		return false;
	}
	//只支持连接命令
	if (ucCmd != 0x01)
	{
		_PutTip("无效的SOCKS代理请求！"
			, UserPtl::PARSE_SOCKS5_CONN_REQUEST_CMD_INVALID);
		return false;
	}
	//保留
	parse.Skip(1);
	//地址类型
	unsigned char ucAddrType = 0;
	parse.Pop(ucAddrType);
	//解析是否成功
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_2);
		return false;
	}
	//只支持IPv4
	if (ucAddrType != 0x01)
	{
		_PutTip("无效的SOCKS代理请求！"
			, UserPtl::PARSE_SOCKS5_CONN_REQUEST_ADDR_INVALID);
		return false;
	}
	//目标IP
	parse.Pop(ulDstIP);
	//目标端口
	parse.PopReverse(usDstPort);
	//解析是否成功
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS5_CONN_REQUEST_PARSE_FAIL_3);
		return false;
	}

	return true;
}

/******************************************************************************
解析SOCKS4连接请求包
******************************************************************************/
bool CUserPtl::ParseSocks4ConnRequest(unsigned long & ulDstIP
	, unsigned short & usDstPort, void * pData, unsigned int uiDataSize)
{
	//解析数据
	CDataParse parse;
	//设置数据
	if (pData == NULL
		|| uiDataSize <= 0)
	{
		_PutTip("", UserPtl::PARSE_SOCKS4_CONN_REQUEST_DATA_INVALID);
		return false;
	}
	parse.SetData(pData, uiDataSize);
	//版本
	parse.Skip(1);
	//命令
	unsigned char ucCmd = 0;
	parse.Pop(ucCmd);
	//解析是否成功
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS4_CONN_REQUEST_PARSE_FAIL_1);
		return false;
	}
	//只支持连接命令
	if (ucCmd != 0x01)
	{
		_PutTip("无效的SOCKS代理请求！"
			, UserPtl::PARSE_SOCKS4_CONN_REQUEST_CMD_INVALID);
		return false;
	}
	//目标端口
	parse.PopReverse(usDstPort);
	//目标IP
	parse.Pop(ulDstIP);
	//解析是否成功
	if (!parse.IsSucc())
	{
		_PutTip("", UserPtl::PARSE_SOCKS4_CONN_REQUEST_PARSE_FAIL_2);
		return false;
	}

	return true;
}

/******************************************************************************
打包SOCKS5连接响应包
******************************************************************************/
void CUserPtl::PackSocks5ConnResponse(CDataPack & pack)
{
	//版本
	pack.Push("\x05", 1);
	//成功
	pack.Push("\x00", 1);
	//保留
	pack.Push("\x00", 1);
	//IPv4
	pack.Push("\x01", 1);
	//IP
	pack.Push("\x00\x00\x00\x00", 4);
	//端口
	pack.Push("\x00\x00", 2);
}

/******************************************************************************
打包SOCKS4连接响应包
******************************************************************************/
void CUserPtl::PackSocks4ConnResponse(CDataPack & pack)
{
	//版本
	pack.Push("\x00", 1);
	//成功
	pack.Push("\x5A", 1);
	//端口
	pack.Push("\x00\x00", 2);
	//IP
	pack.Push("\x00\x00\x00\x00", 4);
}

/******************************************************************************
输出提示
******************************************************************************/
void CUserPtl::_PutTip(string strTip, int iErr, string strErr)
{
	//输出提示回调
	if (m_pfnPutTip == NULL
		|| m_pThis == NULL)
	{
		return;
	}
	m_pfnPutTip(strTip, "UserPtl", iErr, strErr, m_pThis);
}