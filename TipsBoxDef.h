#pragma once

namespace TipsBox
{

/******************************************************************************
�����붨��
******************************************************************************/
typedef enum ErrorCode
{
	INIT_TIPS_BOX_NULL = 1,		//��ʾ��ָ��Ϊ��
	INIT_PARENT_WND_NULL,		//�����ָ��Ϊ��

	PUT_TIP_TIPS_BOX_NULL,		//��ʾ��ָ��Ϊ��

	LOCK_INTERFACE_LOCK_FAIL,			//����ʧ��
	LOCK_INTERFACE_LOCK_EXCEPTION,		//�����쳣

	UNLOCK_INTERFACE_UNLOCK_FAIL,		//����ʧ��

	LOCK_SCROLL_LOCK_FAIL,			//����ʧ��
	LOCK_SCROLL_LOCK_EXCEPTION,		//�����쳣

	UNLOCK_SCROLL_UNLOCK_FAIL,		//����ʧ��

	IS_NEED_SAVE_TIPS_BOX_NULL,		//��ʾ��ָ��Ϊ��

	SAVE_TIPS_TIPS_BOX_NULL,			//��ʾ��ָ��Ϊ��
	SAVE_TIPS_SOFTWARE_PATH_EMPTY,		//�����ָ��Ϊ��

	SAVE_TO_FILE_OPEN_FAIL,						//�ļ���ʧ��
	SAVE_TO_FILE_SET_LOCALE_FAIL,				//��������ʧ��
	SAVE_TO_FILE_WRITE_STRING_EXCEPTION,		//д���ļ��쳣

	CREATE_TIPS_LOG_PATH_SOFTWARE_PATH_EMPTY,		//�����ָ��Ϊ��
	CREATE_TIPS_LOG_PATH_CREATE_PATH_FAIL,			//����Ŀ¼ʧ��
}EErrorCode;

/******************************************************************************
��������
******************************************************************************/
typedef enum Const
{
	SAVE_TIPS_SIZE = 10 * 1024 * 1024,		//������ʾ��Ϣ�Ĵ�С��10M��
}EConst;

}