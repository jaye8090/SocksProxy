#pragma once

#include <string>
using namespace std;

#include "UserPtlDef.h"

class CDataPack;

/******************************************************************************
用户协议类
功能：
	实现SOCKS用户数据包的打包和解析
作者：
	佳也 2623168833 jaye8090@qq.com
时间：
	2016-1-8 到 2016-1-8
******************************************************************************/
class CUserPtl
{
public:
	CUserPtl(UserPtl::PFNPutTip pfnPutTip = NULL, void * pThis = NULL);

	~CUserPtl(void);

//基本接口
public:
	/**************************************************************************
	获取SOCKS版本
	参数：
		pData			数据
		uiDataSize		数据大小
	返回：
		成功返回SOCKS版本，失败返回0。
	**************************************************************************/
	unsigned char GetSocksVer(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	解析认证请求包
	参数：
		pData			数据
		uiDataSize		数据大小
	返回：
		成功返回true，失败返回false。
	**************************************************************************/
	bool ParseAuthRequest(void * pData, unsigned int uiDataSize);

	/**************************************************************************
	打包认证响应包
	参数：
		pack		输出值。打包。
	**************************************************************************/
	void PackAuthResponse(CDataPack & pack);

	/**************************************************************************
	解析SOCKS5连接请求包
	参数：
		ulDstIP			输出值。目标IP。
		ulDstPort		输出值。目标端口。
		pData			数据
		uiDataSize		数据大小
	返回：
		成功返回true，失败返回false。
	**************************************************************************/
	bool ParseSocks5ConnRequest(unsigned long & ulDstIP
		, unsigned short & usDstPort, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	解析SOCKS4连接请求包
	参数：
		ulDstIP			输出值。目标IP。
		ulDstPort		输出值。目标端口。
		pData			数据
		uiDataSize		数据大小
	返回：
		成功返回true，失败返回false。
	**************************************************************************/
	bool ParseSocks4ConnRequest(unsigned long & ulDstIP
		, unsigned short & usDstPort, void * pData, unsigned int uiDataSize);

	/**************************************************************************
	打包SOCKS5连接响应包
	参数：
		pack		输出值。打包。
	**************************************************************************/
	void PackSocks5ConnResponse(CDataPack & pack);

	/**************************************************************************
	打包SOCKS4连接响应包
	参数：
		pack		输出值。打包。
	**************************************************************************/
	void PackSocks4ConnResponse(CDataPack & pack);

//内部操作
private:
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
	UserPtl::PFNPutTip		m_pfnPutTip;		//输出提示回调函数
	void					* m_pThis;			//回调this指针
};