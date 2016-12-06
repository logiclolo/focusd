/*
 ***********************************************************************
 * $Header$
 *
 *  Copyright (c) 2000-2010 Vivotek Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VIVOTEK INC.                                                    |
 *  +-----------------------------------------------------------------+
 *
 * $History$
 *
 ***********************************************************************
 */

/*!
 ***********************************************************************
 * Copyright 2000-2010 Vivotek, Inc. All rights reserved.
 *
 * \file
 * remotefocusd.h
 *
 * \brief
 * C header file of remote focus daemon
 *
 * \date
 * 2010/12/21
 *
 * \author
 * James Zai-Chuan Ye
 *
 ***********************************************************************
 */
#ifndef __REMOTEFOCUSD_H__
#define __REMOTEFOCUSD_H__

/* =========================== Header Files =========================== */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "common.h"
#include "osisolate.h"
#include "fdcgi.h"
#include "message.h"
#include "msgcommon.h"
#include "xmlsparser.h"

#include "focusmotor.h"

#ifdef _YC41AM
	#include "motorinfo_YC41AM.h"
#elif defined(_DF010NA0000)
	#include "motorinfo_DF010NA0000.h"
#elif defined(_TT02812PB)
	#include "motorinfo_TT02812PB.h"
#elif defined(_MSVF33X0313IR)
	#include "motorinfo_MSVF33X0313IR.h"
#elif defined(_ASP3Z0312LMRP)
	#include "motorinfo_ASP3Z0312LMRP.h"
#elif defined(_ABFMOTOR)
	#include "motorinfo_ABFMOTOR.h"
#endif
/* ========================= Macro Definition ========================= */
#define REMOTEFOCUSD_VERSION_STRING				"1.0.1.0"
#define REMOTEFOCUSD_UPDATETIME					"2013/06/12"
#define REMOTEFOCUSD_MODULENAME					"remotefocusd"

#define REMOTEFOCUSD_DBGPRINT(fmt, args...) \
	{\
		printf("["REMOTEFOCUSD_MODULENAME"] : " fmt, ##args);\
	}

#define REMOTEFOCUSD_FDCGI_SOCKET_PATH			"/tmp/RemoteFocus"
#define REMOTEFOCUSD_FIFO_PATH					"/tmp/RemoteFocusFIFO"
#define REMOTEFOCUSD_PID_FILE					"/var/run/remotefocusd.pid"
#define REMOTEFOCUSD_CAPABILITY_FILE			"/etc/conf.d/config_capability.xml"
#define REMOTEFOCUSD_CONFIG_FILE				"/etc/conf.d/remotefocusd/remotefocusd.conf"
#define REMOTEFOCUSD_FOCUS_LOG_FILE				"/etc/conf.d/remotefocusd/focusworking.log"
#define REMOTEFOCUSD_ZOOM_LOG_FILE				"/etc/conf.d/remotefocusd/zoomworking.log"
#define REMOTEFOCUSD_FOCUS_TABLE_MAPPING_FILE	"/etc/conf.d/remotefocusd/pan_focus.tab"
#define REMOTEFOCUSD_FOCUS_DEFAULT_FILE			"/etc/conf.d/remotefocusd/focus_default"
#define REMOTEFOCUSD_FOCUS_MOTOR_DEVICE_FILE	"/dev/focusmotor"
#define REMOTEFOCUSD_ZOOM_MOTOR_DEVICE_FILE		"/dev/zoommotor"

#define REMOTEFOCUSD_THREAD_PRIORITY			120

#define REMOTEFOCUSD_MESSAGE_HEADER_LENGTH		4
#define REMOTEFOCUSD_MESSAGE_NUMBER 			10

#define REMOTEFOCUSD_SHMEM_SEGSIZE				32
#define REMOTEFOCUSD_SHMEM_FILEPATH				"/usr/sbin/remotefocusd"
#define REMOTEFOCUSD_ETC_FWVER                  "/etc/firmware-version"

#define REMOTEFOCUSD_RESP_200_OK				"HTTP/1.1 200 OK\r\n\r\n"
#define REMOTEFOCUSD_RESP_BAD_REQUEST			"HTTP/1.1 400 Bad Request\r\n\r\n"
#define REMOTEFOCUSD_RESP_SERVER_ERROR			"HTTP/1.1 500 Internal Server Error\r\n\r\n"
#define REMOTEFOCUSD_RESP_SERVICE_UNAVAILABLE	"HTTP/1.1 503 Service Unavailable\r\n\r\n"

// Add by Gary on 2011/07/26 for FD8162/FD8362-VRNT
#define REMOTEFOCUSD_DEFAULT_AF_BUTTON_PRESSTIME_FOR_FOCUS_SCAN     5 //second
#define REMOTEFOCUSD_DEFAULT_AF_BUTTON_PRESSTIME_FOR_AUTO_FOCUS     2 //second

/* =============== Structure and Enumeration Definition =============== */
typedef enum _ERemoteFocusd_Capability
{
	esupNoSupport = 0, 		// no support remotefocus
	esupZoomFocus = 1,		// support zoom / focus
	esupZoom = 2,			// support zoom
	esupFocus = 4,			// support focus
} ERemoteFocusd_Capability;

typedef enum _ERemoteFocusd_FunctionType
{
	eftNoService = 0,
	eftZoom,
	eftFocus,
	eftAutoFocus,
	eftFocusScan,
	eftPositioning,
	eftGetStatus,
	eftGetParam,
	eftStop,
	eftZooming,
	eftIrisOpen,
	eftIrisEnable,
	eftResetFocus,
	eftGetFocusDefault,
	eftSetFocusDefault
} ERemoteFocusd_FunctionType;

typedef enum _ERemoteFocusd_MotorDirection
{
	emdForward = 0,
	emdBackward,
	emdDriect,
} ERemoteFocusd_MotorDirection;

typedef struct _TRemoteFocusd_MotorCtrlInfo
{
	ERemoteFocusd_FunctionType eFunctionType;
	ERemoteFocusd_MotorDirection eMotorDirection;

	DWORD dwMotorSteps;
	DWORD dwMotorPosition;
	DWORD dwMotorFocusDefault;
	// Addded by Jiun on 2012/06/15 for motor's focal lengths record
	DWORD dwMotorStart;
	DWORD dwMotorEnd;

	BOOL bFullyOpenedIris;
	BOOL bKeepZooming;

} TRemoteFocusd_MotorCtrlInfo;

typedef struct _TRemoteFocusd_ClientInfo
{
	SOCKET sckClient;

	TRemoteFocusd_MotorCtrlInfo tMotorCtrlInfoOpt;
} TRemoteFocusd_ClientInfo;

typedef struct _TRemoteFocusd_ShareMemInfo
{
#ifdef __FV_EDGE
	DWORD dwFocusMode;					//	0 : isp		1 : edge
#endif
	BOOL bFetchEnable;
	DWORD dwFocusValue;

	BOOL bIrisEnable;
	BOOL bFullyOpenedIris;
	BOOL bAutoFocusEnable;
	BOOL bIsStartAutoFocus;
} TRemoteFocusd_ShareMemInfo;

typedef struct _TRemoteFocusdInfo
{
	DWORD dwMyPid;

	// For communication loop (main loop)
	HANDLE hXMLParser;
	fdcgi *ptFdCgi;
	int iMaxFd;
	int iCGIFd;
	int iFIFOFd;
	fd_set fds_ReadSet;
	DWORD dwButtonPressTime;

	// For focus motor thread
	DWORD dwFocusValue;
	HANDLE hFocusMotorThread;
	BOOL bFocusOperationEnable;
	DWORD dwFocusMotorPosition;
	DWORD dwPanFocusPosition;
	TRemoteFocusd_MotorCtrlInfo tFocusMotorCtrlInfo;

	// For zoom motor thread
	HANDLE hZoomMotorThread;
	BOOL bZoomOperationEnable;
	ERemoteFocusd_FunctionType eNextFuncType;
	DWORD dwZoomMotorPosition;
	TRemoteFocusd_MotorCtrlInfo tZoomMotorCtrlInfo;

	//For accessing memory shared with venc
	TRemoteFocusd_ShareMemInfo* ptShareMemInfo;
	int iShMemId;

	// Semaphore for both thread accessing config. file
	sem_t semFileMutex;

	// Socket pairs for communication between motor threads and info process
	int fdFocusMotorWakeup[2];
	int fdZoomMotorWakeup[2];

	// remotefocus capability
	int iIsSupportZoom;
	int iIsSupportFocus;
} TRemoteFocusdInfo;

/* ======================= Function Declaration ======================= */
SCODE RemoteFocusd_Init(TRemoteFocusdInfo *pThis);
SCODE RemoteFocusd_Release(TRemoteFocusdInfo *pThis);

SCODE RemoteFocusd_InitFdCgi(TRemoteFocusdInfo *pThis);
SCODE RemoteFocusd_InitFIFO(TRemoteFocusdInfo *pThis);

SCODE RemoteFocusd_InfoProcess(TRemoteFocusdInfo *pThis);

DWORD THREADAPI RemoteFocusd_FocusMotorThread(DWORD dwInstance);
DWORD THREADAPI RemoteFocusd_ZoomMotorThread(DWORD dwInstance);

#endif //__REMOTEFOCUSD_H__
