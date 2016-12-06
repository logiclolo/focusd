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
 * remotefocusd_process.c
 *
 * \brief
 * C source file of remote focus daemon
 * It includes main loop function and related procedures.
 * The main purpose is communicating with Boa and client.
 *
 * \date
 * 2010/12/21
 *
 * \author
 * James Zai-Chuan Ye
 *
 ***********************************************************************
 */


#include "remotefocusd.h"

extern const DWORD g_dwtabFocusingStartTable[];
extern DWORD g_dwtabFocusValueTable[];
extern DWORD g_dwFocusDefault;
extern BOOL g_bIrisFullyOpenStatus;

/* =========================================================================================== */
void RemoteFocusd_CheckLogFile(TRemoteFocusdInfo *pThis)
{
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);


	if ((access(REMOTEFOCUSD_FOCUS_LOG_FILE, F_OK) == 0)
			|| (access(REMOTEFOCUSD_ZOOM_LOG_FILE, F_OK) == 0))
	{
		REMOTEFOCUSD_DBGPRINT("Power loss unexpectedly! Need positioning.\n");

		if (pThis->iIsSupportFocus)
		{
			ptFocusMotorCtrlInfo->eFunctionType = eftPositioning;
			pThis->bFocusOperationEnable = TRUE;
			if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
						sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
			{
				REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
			}
		}

		if (pThis->iIsSupportZoom)
		{
			ptZoomMotorCtrlInfo->eFunctionType = eftPositioning;
			ptZoomMotorCtrlInfo->dwMotorPosition = REMOTEFOCUSD_ZOOM_VIRTUAL_START;
			pThis->bZoomOperationEnable = TRUE;
			if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
						sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
			{
				REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
			}
		}
	}


	return;
}

/* =========================================================================================== */
void RemoteFocusd_ReplyBadRequestMsg(SOCKET sckClient)
{
	char acReplyMsg[128];


	sprintf(acReplyMsg, "HTTP/1.1 400 Bad Request\r\n\r\n");
	write(sckClient, acReplyMsg, strlen(acReplyMsg));

	close(sckClient);
	return;
}

/* =========================================================================================== */
void RemoteFocusd_ReplyOKMessage(SOCKET sckClient)
{
	char acReplyMsg[1024];


	sprintf(acReplyMsg,
				"HTTP/1.1 200 OK\r\n"
				"Content-type: text/plain\r\n"
				"Cache-control: no-cache\r\n"
				"Pragma: no-cache\r\n"
				"Content-length: 2\r\n\r\n"
				"OK");

	write(sckClient, acReplyMsg, strlen(acReplyMsg));

	close(sckClient);
	return;
}

/* =========================================================================================== */
void RemoteFocusd_ReplyStatus(TRemoteFocusdInfo *pThis, SOCKET sckClient)
{
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
	TRemoteFocusd_ShareMemInfo*  ptShareMemInfo = pThis->ptShareMemInfo;
	char acParamSet[1024];
	char acReplyMsg[1024];

	if (pThis->iIsSupportFocus && pThis->iIsSupportZoom)
	{
#ifdef REPLYSTATUS_WITH_FOCUSVALUE
		RemoteFocusd_EnableFetch(pThis, TRUE);
		usleep(1000000);     // Waitting for share memory to parpers fetch FV.
		RemoteFocusd_FetchFocusValue(pThis);
		// 2013/08/05 add zoom_motor_start, zoom_motor_end and focus_motor_max to unify all remote focus API
		sprintf(acParamSet,
				"remote_focus_zoom_motor_max='%d'\r\n"
				"remote_focus_focus_motor_max='%d'\r\n"
				"remote_focus_zoom_motor_start='%d'\r\n"
				"remote_focus_zoom_motor_end='%d'\r\n"
				"remote_focus_focus_motor_start='%d'\r\n"
				"remote_focus_focus_motor_end='%d'\r\n"
				"remote_focus_zoom_motor='%d'\r\n"
				"remote_focus_focus_motor='%d'\r\n"
				"remote_focus_zoom_enable='%d'\r\n"
				"remote_focus_focus_enable='%d'\r\n"
				"remote_focus_iris_open='%d'\r\n"
				"focus_value='%d'\r\n",
				REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP,
				REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP, //focus motor max = End position - start postion in wide
				ptZoomMotorCtrlInfo->dwMotorStart,
				ptZoomMotorCtrlInfo->dwMotorEnd,
				ptFocusMotorCtrlInfo->dwMotorStart,
				ptFocusMotorCtrlInfo->dwMotorEnd,
				pThis->dwZoomMotorPosition,
				pThis->dwFocusMotorPosition,
				ptZoomMotorCtrlInfo->eFunctionType,
				ptFocusMotorCtrlInfo->eFunctionType,
				g_bIrisFullyOpenStatus,
				pThis->dwFocusValue
				);
		RemoteFocusd_EnableFetch(pThis, FALSE);
#else
		// 2013/08/05 add zoom_motor_start, zoom_motor_end and focus_motor_max to unify all remote focus API
		sprintf(acParamSet,
				"remote_focus_zoom_motor_max='%d'\r\n"
				"remote_focus_focus_motor_max='%d'\r\n"
				"remote_focus_zoom_motor_start='%d'\r\n"
				"remote_focus_zoom_motor_end='%d'\r\n"
				"remote_focus_focus_motor_start='%d'\r\n"
				"remote_focus_focus_motor_end='%d'\r\n"
				"remote_focus_zoom_motor='%d'\r\n"
				"remote_focus_focus_motor='%d'\r\n"
				"remote_focus_zoom_enable='%d'\r\n"
				"remote_focus_focus_enable='%d'\r\n"
				"remote_focus_iris_open='%d'\r\n"
				"focus_value='%d'\r\n",
				REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP,
				REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP, //focus motor max = End position - start postion in wide
				ptZoomMotorCtrlInfo->dwMotorStart,
				ptZoomMotorCtrlInfo->dwMotorEnd,
				ptFocusMotorCtrlInfo->dwMotorStart,
				ptFocusMotorCtrlInfo->dwMotorEnd,
				pThis->dwZoomMotorPosition,
				pThis->dwFocusMotorPosition,
				ptZoomMotorCtrlInfo->eFunctionType,
				ptFocusMotorCtrlInfo->eFunctionType,
				g_bIrisFullyOpenStatus
				);
#endif
	}
	else if(pThis->iIsSupportFocus)
	{
		sprintf(acParamSet,
				"remote_focus_focus_motor_max='%d'\r\n"
				"remote_focus_focus_motor_start='%d'\r\n"
				"remote_focus_focus_motor_end='%d'\r\n"
				"remote_focus_focus_motor='%d'\r\n"
				"remote_focus_focus_enable='%d'\r\n"
				"remote_focus_iris_open='%d'\r\n",
				REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP, //focus motor max = End position - start postion in wide
				ptFocusMotorCtrlInfo->dwMotorStart,
				ptFocusMotorCtrlInfo->dwMotorEnd,
				pThis->dwFocusMotorPosition,
				ptFocusMotorCtrlInfo->eFunctionType,
				g_bIrisFullyOpenStatus
			   );
	}
	else if(pThis->iIsSupportZoom)
	{
		sprintf(acParamSet,
				"remote_focus_zoom_motor_max='%d'\r\n"
				"remote_focus_zoom_motor_start='%d'\r\n"
				"remote_focus_zoom_motor_end='%d'\r\n"
				"remote_focus_zoom_motor='%d'\r\n"
				"remote_focus_zoom_enable='%d'\r\n"
				"remote_focus_iris_open='%d'\r\n",
				REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP,
				ptZoomMotorCtrlInfo->dwMotorStart,
				ptZoomMotorCtrlInfo->dwMotorEnd,
				pThis->dwZoomMotorPosition,
				ptZoomMotorCtrlInfo->eFunctionType,
				g_bIrisFullyOpenStatus
					);
	}

	sprintf(acReplyMsg,
				"HTTP/1.1 200 OK\r\n"
				"Content-type: text/plain\r\n"
				"Cache-control: no-cache\r\n"
				"Pragma: no-cache\r\n"
				"Content-length: %d\r\n\r\n"
				"%s",
				strlen(acParamSet),
				acParamSet);

	write(sckClient, acReplyMsg, strlen(acReplyMsg));

	close(sckClient);
	return;
}// End of RemoteFocusd_ReplyStatus

/* =========================================================================================== */
void RemoteFocusd_ReplyParam(TRemoteFocusdInfo *pThis, SOCKET sckClient, DWORD dwPosition)
{
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	char acParamSet[1024];
	char acReplyMsg[1024];


	// Force to (0 ~ REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP)
	dwPosition = (dwPosition < 0)? 0 : ((dwPosition >= REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP)? 
						(REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP - 1) : dwPosition);

	sprintf(acParamSet,
				"remote_focus_focus_value='%d'\r\n"
				"remote_focus_focus_enable='%d'\r\n",
				g_dwtabFocusValueTable[dwPosition],
				ptFocusMotorCtrlInfo->eFunctionType);

	sprintf(acReplyMsg,
				"HTTP/1.1 200 OK\r\n"
				"Content-type: text/plain\r\n"
				"Cache-control: no-cache\r\n"
				"Pragma: no-cache\r\n"
				"Content-length: %d\r\n\r\n"
				"%s",
				strlen(acParamSet),
				acParamSet);

	write(sckClient, acReplyMsg, strlen(acReplyMsg));

	close(sckClient);
	return;
}// End of RemoteFocusd_ReplyParam
/* =========================================================================================== */
#ifdef _FOCUS_DEFAULT
void RemoteFocusd_ReplyFocusDefault(TRemoteFocusdInfo *pThis, SOCKET sckClient)
{
	char acParamSet[1024];
	char acReplyMsg[1024];

	sprintf(acParamSet,"remote_focus_focus_default='%d'\r\n",g_dwFocusDefault);

	sprintf(acReplyMsg,
				"HTTP/1.1 200 OK\r\n"
				"Content-type: text/plain\r\n"
				"Cache-control: no-cache\r\n"
				"Pragma: no-cache\r\n"
				"Content-length: %d\r\n\r\n"
				"%s",
				strlen(acParamSet),
				acParamSet);

	write(sckClient, acReplyMsg, strlen(acReplyMsg));

	close(sckClient);
	return;
}// End of RemoteFocusd_ReplyFocusDefault
#endif

/* =========================================================================================== */
void RemoteFocusd_ProcessPostCommand(char* pcCommand)
{
	int iIndex;


	if (pcCommand == NULL)
	{
		return;
	}

	for (iIndex = 0; iIndex < strlen(pcCommand); iIndex ++)
	{
		if (pcCommand[iIndex] == '+')
		{
			pcCommand[iIndex] = ' ';
		}
	}
	
	return;
}

/* =========================================================================================== */
void RemoteFocusd_InformMotorThread(TRemoteFocusdInfo *pThis, TRemoteFocusd_ClientInfo* ptClientInfo)
{
	TRemoteFocusd_MotorCtrlInfo* ptMotorCtrlInfoOpt = &(ptClientInfo->tMotorCtrlInfoOpt);
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);


	switch (ptMotorCtrlInfoOpt->eFunctionType)
	{
		case eftFocus:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->dwMotorPosition = ptMotorCtrlInfoOpt->dwMotorPosition;
				ptFocusMotorCtrlInfo->dwMotorSteps = ptMotorCtrlInfoOpt->dwMotorSteps;
				ptFocusMotorCtrlInfo->eMotorDirection = ptMotorCtrlInfoOpt->eMotorDirection;
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus thread failed.\n");
				}
			}
			break;
		case eftZoom:
			if (pThis->iIsSupportZoom)
			{
				ptZoomMotorCtrlInfo->dwMotorPosition = ptMotorCtrlInfoOpt->dwMotorPosition;
				ptZoomMotorCtrlInfo->dwMotorSteps = ptMotorCtrlInfoOpt->dwMotorSteps;
				ptZoomMotorCtrlInfo->eMotorDirection = ptMotorCtrlInfoOpt->eMotorDirection;
				ptZoomMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bZoomOperationEnable = TRUE;
				if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
							sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
				}
			}
			break;
		case eftAutoFocus:
			if (pThis->iIsSupportFocus)
			{
				// Stop zoom motor first
				ptZoomMotorCtrlInfo->eFunctionType = eftStop;
				pThis->bZoomOperationEnable = TRUE;
				if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
							sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
				}
				if (g_bIrisFullyOpenStatus == TRUE)
				{
					ptFocusMotorCtrlInfo->bFullyOpenedIris = TRUE;
				}
				else
				{
					ptFocusMotorCtrlInfo->bFullyOpenedIris = ptMotorCtrlInfoOpt->bFullyOpenedIris;
				}
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
			}
			break;
		case eftFocusScan:
			if (pThis->iIsSupportFocus)
			{
				// Stop zoom motor first
				ptZoomMotorCtrlInfo->eFunctionType = eftStop;
				pThis->bZoomOperationEnable = TRUE;
				if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
							sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
				}

				if (g_bIrisFullyOpenStatus == TRUE)
				{
					ptFocusMotorCtrlInfo->bFullyOpenedIris = TRUE;
				}
				else
				{
					ptFocusMotorCtrlInfo->bFullyOpenedIris = ptMotorCtrlInfoOpt->bFullyOpenedIris;
				}
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
			}
			break;
		case eftPositioning:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
			}

			if (pThis->iIsSupportZoom)
			{
				ptZoomMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				ptZoomMotorCtrlInfo->dwMotorPosition = REMOTEFOCUSD_ZOOM_VIRTUAL_START;
				pThis->bZoomOperationEnable = TRUE;
				if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
							sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
				}
			}

			break;
		case eftStop:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
			}

			if (pThis->iIsSupportZoom)
			{
				ptZoomMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bZoomOperationEnable = TRUE;
				if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
							sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
				}
			}

			break;
		case eftIrisOpen:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
				ptFocusMotorCtrlInfo->bFullyOpenedIris = TRUE;
				g_bIrisFullyOpenStatus = TRUE;
			}
			break;
		case eftIrisEnable:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
				ptFocusMotorCtrlInfo->bFullyOpenedIris = FALSE;
				g_bIrisFullyOpenStatus =FALSE;
			}
			break;
		case eftResetFocus:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
			}
			break;
		case eftSetFocusDefault:
			if (pThis->iIsSupportFocus)
			{
				ptFocusMotorCtrlInfo->eFunctionType = ptMotorCtrlInfoOpt->eFunctionType;
				ptFocusMotorCtrlInfo->dwMotorFocusDefault = ptMotorCtrlInfoOpt->dwMotorFocusDefault;
				pThis->bFocusOperationEnable = TRUE;
				if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
							sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
				}
			}
			break;

		default:
			// Do nothing
			break;
	}// End of switch (ptMotorCtrlInfoOpt->eFunctionType)

	return;
}// End of RemoteFocusd_InformMotorThread

/* =========================================================================================== */
SCODE RemoteFocusd_InterpretClientCommand(TRemoteFocusd_MotorCtrlInfo* ptMotorCtrlInfoOpt
											, char* szName, char* szVal)
{

	if (strncmp(szName, "function", 8) == 0)
	{
		if (strncmp(szVal, "zoom", 4) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftZoom;
		}
		else if (strncmp(szVal, "focus", 5) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftFocus;
		}
		else if (strncmp(szVal, "auto", 4) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftAutoFocus;
		}
		else if (strncmp(szVal, "scan", 4) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftFocusScan;
		}
		else if (strncmp(szVal, "positioning", 11) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftPositioning;
		}
		else if (strncmp(szVal, "getstatus", 9) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftGetStatus;
		}
		else if (strncmp(szVal, "getparam", 8) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftGetParam;
		}
		else if (strncmp(szVal, "stop", 4) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftStop;
		}
		else if (strncmp(szVal, "irisopen", 8) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftIrisOpen;
		}
		else if (strncmp(szVal, "irisenable", 10) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftIrisEnable;
		}
#ifdef _FOCUS_DEFAULT
		else if (strncmp(szVal, "resetfocus", 10) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftResetFocus;
		}
		else if (strncmp(szVal, "setfocusdefault", 15) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftSetFocusDefault;
		}
		else if (strncmp(szVal, "getfocusdefault", 15) == 0)
		{
			ptMotorCtrlInfoOpt->eFunctionType = eftGetFocusDefault;
		}
#endif
		else
		{
			REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
			return S_FAIL;
		}
	}
	else if (strncmp(szName, "direction", 9) == 0)
	{
		if (strncmp(szVal, "forward", 7) == 0)
		{
			ptMotorCtrlInfoOpt->eMotorDirection = emdForward;
		}
		else if (strncmp(szVal, "backward", 8) == 0)
		{
			ptMotorCtrlInfoOpt->eMotorDirection = emdBackward;
		}
		else if (strncmp(szVal, "direct", 6) == 0)
		{
			ptMotorCtrlInfoOpt->eMotorDirection = emdDriect;
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
			return S_FAIL;
		}
	}
	else if (strncmp(szName, "steps", 5) == 0)
	{
		if (strlen(szVal) != 0)
		{
			ptMotorCtrlInfoOpt->dwMotorSteps = atoi(szVal);
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
			return S_FAIL;
		}
	}
	else if (strncmp(szName, "position", 8) == 0)
	{
		if (strlen(szVal) != 0)
		{
			ptMotorCtrlInfoOpt->dwMotorPosition = atoi(szVal);
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
			return S_FAIL;
		}
	}
#ifdef _FOCUS_DEFAULT
	else if (strncmp(szName, "defaultposition", 15) ==0)
	{
		if (strlen(szVal) != 0)
		{
			ptMotorCtrlInfoOpt->dwMotorFocusDefault = atoi(szVal);
			if (ptMotorCtrlInfoOpt->dwMotorFocusDefault >= REMOTEFOCUSD_FOCUS_VIRTUAL_END)
			{
				ptMotorCtrlInfoOpt->dwMotorFocusDefault = REMOTEFOCUSD_FOCUS_VIRTUAL_END;
			}
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
			return S_FAIL;
		}
	}
#endif
	else if (strncmp(szName, "iris", 4) == 0)
	{
		ptMotorCtrlInfoOpt->bFullyOpenedIris = TRUE;
	}
	else
	{
		REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
		return S_FAIL;
	}

	return S_OK;
}// End of RemoteFocusd_InterpretClientCommand

/* =========================================================================================== */
SCODE RemoteFocusd_ParseClientCommand(TRemoteFocusdInfo *pThis, char* pcCommand
											, TRemoteFocusd_ClientInfo *ptClientInfo)
{
	TRemoteFocusd_MotorCtrlInfo* ptMotorCtrlInfoOpt = &(ptClientInfo->tMotorCtrlInfoOpt);
	BOOL bStop = FALSE;
	char szName[64], szVal[64];
	char *pcQuery, *pcNext, *pcTmp;
	int iNameLen;


	if ((pcCommand == NULL) || (pcCommand[0] == '\0'))
	{
		REMOTEFOCUSD_DBGPRINT("Empty query string!\n");
		return S_FAIL;
	}
	else
	{
		pcQuery = pcCommand;

		while (!bStop)
		{
			pcNext = strchr(pcQuery, '&');
			if (pcNext == NULL)
			{
				bStop = TRUE;
				pcNext = pcQuery + strlen(pcQuery);
			}

			pcTmp = strchr(pcQuery, '=');
			if (pcTmp != NULL)
			{
				// Parsing name
				iNameLen = pcTmp - pcQuery;
				memcpy(szName, pcQuery, iNameLen);
				szName[iNameLen] = '\0';
				pcTmp++;

				// Parsing value
				iNameLen = pcNext - pcTmp;
				memcpy(szVal, pcTmp, iNameLen);
				szVal[iNameLen] = '\0';
			}
			else
			{
				// Parsing name only
				strncpy(szName, pcQuery, strlen(pcQuery) + 1);
				szVal[0] = '\0';
			}


			if (RemoteFocusd_InterpretClientCommand
						(ptMotorCtrlInfoOpt, szName, szVal) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InterpretClientCommand failed.\n");
				return S_FAIL;
			}

			pcQuery = pcNext + 1;
		}
	}

	if (ptMotorCtrlInfoOpt->eFunctionType == eftNoService)
	{
		REMOTEFOCUSD_DBGPRINT("Function isn't specified.\n");
		return S_FAIL;
	}

	// Debug information
	REMOTEFOCUSD_DBGPRINT("Command string: %s\n", pcCommand);

	return S_OK;
}// End of RemoteFocusd_ParseClientCommand

/* =========================================================================================== */
SCODE RemoteFocusd_FdCgiCommandHandler(TRemoteFocusdInfo *pThis)
{
	char* pcCommand = NULL;
	fdcgi* ptFdCgi = NULL;
	TRemoteFocusd_ClientInfo tClientInfo;
	TRemoteFocusd_MotorCtrlInfo* ptMotorCtrlInfoOpt = NULL;


	ptFdCgi = pThis->ptFdCgi;
	memset(&tClientInfo, 0, sizeof(TRemoteFocusd_ClientInfo));

	if (FDCGI_read(ptFdCgi, &pcCommand,
						&(tClientInfo.sckClient)) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("Reinitialize FDCgi...\n");
		FD_CLR(pThis->iCGIFd, &(pThis->fds_ReadSet));
		FDCGI_close(pThis->ptFdCgi);
		if (RemoteFocusd_InitFdCgi(pThis) != S_OK)
		{
			REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitFdCgi failed.\n");
			return S_FAIL;
		}

		return S_OK;
	}

	if (strcmp(FDCGI_GetMethod(ptFdCgi), "POST") == 0)
	{
		RemoteFocusd_ProcessPostCommand(pcCommand);
	}

	if (RemoteFocusd_ParseClientCommand
				(pThis, pcCommand, &tClientInfo) != S_OK)
	{//Parsing error
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_ParseClientCommand failed.\n");
		RemoteFocusd_ReplyBadRequestMsg(tClientInfo.sckClient);
		return S_FAIL;
	}

	// Function execution
	ptMotorCtrlInfoOpt = &(tClientInfo.tMotorCtrlInfoOpt);
	if (ptMotorCtrlInfoOpt->eFunctionType == eftGetStatus)
	{
		RemoteFocusd_ReplyStatus(pThis, tClientInfo.sckClient);
	}
	else if (ptMotorCtrlInfoOpt->eFunctionType == eftGetParam)
	{
		RemoteFocusd_ReplyParam(pThis, tClientInfo.sckClient,
									ptMotorCtrlInfoOpt->dwMotorPosition);
	}
#ifdef _FOCUS_DEFAULT
	else if (ptMotorCtrlInfoOpt->eFunctionType == eftGetFocusDefault)
	{
		RemoteFocusd_ReplyFocusDefault(pThis, tClientInfo.sckClient);
	}
#endif
	else
	{
		RemoteFocusd_InformMotorThread(pThis, &tClientInfo);
		RemoteFocusd_ReplyOKMessage(tClientInfo.sckClient);
	}

	return S_OK;
}// End of RemoteFocusd_FdCgiCommandHandler

/* =========================================================================================== */
SCODE RemoteFocusd_HandleEventMessage(TRemoteFocusdInfo *pThis, int iMessageNum, TMessageInfo *ptMsgInfo)
{
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
#ifdef _TWO_FUNCTION_AF_BUTTON // 2011/07/26 add by Gary for FD8162/FD8362-VRNT
    DWORD dwAFButtonPressTimeForFocusScan = REMOTEFOCUSD_DEFAULT_AF_BUTTON_PRESSTIME_FOR_FOCUS_SCAN;
    DWORD dwAFButtonPressTimeForAutoFocus = REMOTEFOCUSD_DEFAULT_AF_BUTTON_PRESSTIME_FOR_AUTO_FOCUS;
#ifdef _AF_BUTTON_PRESS_TIME_FOR_FOCUS_SCAN
    dwAFButtonPressTimeForFocusScan = _AF_BUTTON_PRESS_TIME_FOR_FOCUS_SCAN;
#endif // _AF_BUTTON_PRESS_TIME_FOR_FOCUS_SCAN
#ifdef _AF_BUTTON_PRESS_TIME_FOR_AUTO_FOCUS
    dwAFButtonPressTimeForAutoFocus = _AF_BUTTON_PRESS_TIME_FOR_AUTO_FOCUS;
#endif // _AF_BUTTON_PRESS_TIME_FOR_AUTO_FOCUS

#endif // _TWO_FUNCTION_AF_BUTTON
	struct timeval tvButtonTimeValue;
	DWORD          dwButtonReleaseTime;

	switch (ptMsgInfo[0].iType)
	{
		case emtAutoFocus:
			if (ptMsgInfo[0].aiValue[0] == emvTrigger)
			{
				gettimeofday(&tvButtonTimeValue, NULL);
				pThis->dwButtonPressTime = (DWORD)((tvButtonTimeValue.tv_usec / 1000) + (tvButtonTimeValue.tv_sec * 1000));
			}
			else if(ptMsgInfo[0].aiValue[0] == emvNormal)
			{
				gettimeofday(&tvButtonTimeValue, NULL);
				dwButtonReleaseTime = (DWORD)((tvButtonTimeValue.tv_usec / 1000) + (tvButtonTimeValue.tv_sec * 1000));

#ifdef _TWO_FUNCTION_AF_BUTTON // 2011/07/26 add by Gary for FD8162/FD8362-VRNT
                // Here we assume AF button press time for focus scan is larger than for auto focus.
				if ((pThis->iIsSupportFocus) && ((dwButtonReleaseTime - pThis->dwButtonPressTime) > dwAFButtonPressTimeForFocusScan * 1000))
				{
                    REMOTEFOCUSD_DBGPRINT("Press AF button over %ld seconds. Perform focus scan...\n", (unsigned long)dwAFButtonPressTimeForFocusScan);
                    ptFocusMotorCtrlInfo->eFunctionType = eftFocusScan;
					pThis->bFocusOperationEnable = TRUE;
					if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
									sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
					{
						REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
					}
				}
                else if ((pThis->iIsSupportFocus) && ((dwButtonReleaseTime - pThis->dwButtonPressTime) > dwAFButtonPressTimeForAutoFocus * 1000))
                {
                    REMOTEFOCUSD_DBGPRINT("Press AF button over %ld seconds. Perform auto focus...\n", (unsigned long)dwAFButtonPressTimeForAutoFocus);
                    ptFocusMotorCtrlInfo->eFunctionType = eftAutoFocus;
					pThis->bFocusOperationEnable = TRUE;
                    if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
                                    sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
                    {
                        REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
                    }
                }
#else
				if ((pThis->iIsSupportZoom) && ((dwButtonReleaseTime - pThis->dwButtonPressTime) > 2000))
				{
					REMOTEFOCUSD_DBGPRINT("Move zoom motor to wide side...\n");

					pThis->eNextFuncType = eftFocusScan;
					ptZoomMotorCtrlInfo->eFunctionType = eftZoom;
					ptZoomMotorCtrlInfo->dwMotorPosition = 0;
					ptZoomMotorCtrlInfo->eMotorDirection = emdDriect;
					pThis->bZoomOperationEnable = TRUE;
					if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
									sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
					{
						REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
					}
				}
#endif  // _TWO_FUNCTION_AF_BUTTON
			}
			else if(ptMsgInfo[0].aiValue[0] == emvFalling)		// Added by Jiun, 2012/06/15
			{													// Positioning motor after restore
				if (pThis->iIsSupportFocus)
				{
					ptFocusMotorCtrlInfo->eFunctionType = eftPositioning;
					pThis->bFocusOperationEnable = TRUE;
					if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
								sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
					{
						REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
					}
				}

				if (pThis->iIsSupportZoom)
				{
					ptZoomMotorCtrlInfo->eFunctionType = eftPositioning;
					ptZoomMotorCtrlInfo->dwMotorPosition = REMOTEFOCUSD_ZOOM_VIRTUAL_START;
					pThis->bZoomOperationEnable = TRUE;
					if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
								sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
					{
						REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
					}
				}
			}
			else
			{
				REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
				return S_FAIL;
			}

			break;

		case emtZoomIn:
			if (pThis->iIsSupportZoom)
			{
				if (ptMsgInfo[0].aiValue[0] == emvTrigger)
				{
					REMOTEFOCUSD_DBGPRINT("Zoom in...\n");
					gettimeofday(&tvButtonTimeValue, NULL);
					pThis->dwButtonPressTime = (DWORD)((tvButtonTimeValue.tv_usec / 1000) + (tvButtonTimeValue.tv_sec * 1000));
					if (ptZoomMotorCtrlInfo->bKeepZooming == FALSE)
					{
						ptZoomMotorCtrlInfo->bKeepZooming= TRUE;
						ptZoomMotorCtrlInfo->eFunctionType = eftZoom;
						ptZoomMotorCtrlInfo->dwMotorSteps = 1;//(REMOTEFOCUSD_ZOOM_MOTOR_TOTALSTEP/10);
						ptZoomMotorCtrlInfo->eMotorDirection = emdForward;
						pThis->bZoomOperationEnable = TRUE;
						if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
									sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
						{
							REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
						}
					}
				}
				else if(ptMsgInfo[0].aiValue[0] == emvNormal)
				{
					REMOTEFOCUSD_DBGPRINT("Zoom in stop!\n");
					ptZoomMotorCtrlInfo->bKeepZooming= FALSE;
				}
			}
			break;

		case emtZoomOut:
			if (pThis->iIsSupportZoom)
			{
				if (ptMsgInfo[0].aiValue[0] == emvTrigger)
				{
					REMOTEFOCUSD_DBGPRINT("Zoom out...\n");
					gettimeofday(&tvButtonTimeValue, NULL);
					pThis->dwButtonPressTime = (DWORD)((tvButtonTimeValue.tv_usec / 1000) + (tvButtonTimeValue.tv_sec * 1000));
					if (ptZoomMotorCtrlInfo->bKeepZooming == FALSE)
					{
						ptZoomMotorCtrlInfo->bKeepZooming= TRUE;
						ptZoomMotorCtrlInfo->eFunctionType = eftZoom;
						ptZoomMotorCtrlInfo->dwMotorSteps = 1;//(REMOTEFOCUSD_ZOOM_MOTOR_TOTALSTEP/10);
						ptZoomMotorCtrlInfo->eMotorDirection = emdBackward;
						pThis->bZoomOperationEnable = TRUE;
						if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
									sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
						{
							REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
						}
					}
				}
				else if(ptMsgInfo[0].aiValue[0] == emvNormal)
				{
					REMOTEFOCUSD_DBGPRINT("Zoom out stop!\n");
					ptZoomMotorCtrlInfo->bKeepZooming= FALSE;
				}
			}
			break;

		default:
			REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
			return S_FAIL;
			break;
	}

	return S_OK;
}// End of RemoteFocusd_HandleEventMessage

/* =========================================================================================== */
SCODE RemoteFocusd_ParseFIFOCmd(TRemoteFocusdInfo *pThis, char *pcBuffer, int iBuffLen, int iType)
{
	TMessageInfo atMsgInfo[REMOTEFOCUSD_MESSAGE_NUMBER];
	TMessageTime tMsgTime;
	int iNumber;
	int iRet;


	iNumber = REMOTEFOCUSD_MESSAGE_NUMBER;
	switch (iType)
	{
		case mtEvent:
			iRet = Message_Parse_Event(pThis->hXMLParser, pcBuffer, iBuffLen, &iNumber, atMsgInfo, &tMsgTime);
			if (iRet != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("Message_Parse_Event failed.\n");
				return S_FAIL;
			}

			if (RemoteFocusd_HandleEventMessage(pThis, iNumber, atMsgInfo) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_HandleEventMessage failed.\n");
				return S_FAIL;
			}

			break;
		default:
			// Do nothing
			break;
	}

	return S_OK;
}// End of RemoteFocusd_ParseFIFOCmd

/* =========================================================================================== */
SCODE RemoteFocusd_CheckMessage(int iMsgType, int iMsgLength)
{

	if ((iMsgType != mtEvent) || (iMsgLength <= 0))
	{
		REMOTEFOCUSD_DBGPRINT("Unknown command.\n");
		return S_FAIL;
	}

	return S_OK;
}

/* =========================================================================================== */
SCODE RemoteFocusd_FIFOCommandHandler(TRemoteFocusdInfo *pThis)
{
	char szCmd[1024];
	DWORD dwLength;
	int iType, iOffset, iMsgLength;
	int iRet;

	
	memset(szCmd, 0, sizeof(szCmd));

	// Read message header for message type and length
	iRet = read(pThis->iFIFOFd, szCmd, REMOTEFOCUSD_MESSAGE_HEADER_LENGTH);
	if (iRet < REMOTEFOCUSD_MESSAGE_HEADER_LENGTH)
	{
		REMOTEFOCUSD_DBGPRINT("Failed to read FIFO.\n");
		return S_FAIL_READ;
	}
	iType = *szCmd;
	iMsgLength = *(szCmd + 1);

	if (RemoteFocusd_CheckMessage(iType, iMsgLength) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_CheckMessage failed.\n");
		return S_FAIL;
	}

	if (Message_GetTypeLength(szCmd, REMOTEFOCUSD_MESSAGE_HEADER_LENGTH,
										&iType, &iOffset, &dwLength) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("Message_GetTypeLength failed.\n");
		return S_FAIL;
	}

	iRet = read(pThis->iFIFOFd, szCmd + REMOTEFOCUSD_MESSAGE_HEADER_LENGTH,
					dwLength - (REMOTEFOCUSD_MESSAGE_HEADER_LENGTH - iOffset));
	if (iRet < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Failed to read FIFO.\n");
		return S_FAIL_READ;
	}
	REMOTEFOCUSD_DBGPRINT("Receive FIFO command: %s\n", szCmd + iOffset);

	if (RemoteFocusd_ParseFIFOCmd(pThis, szCmd + iOffset, dwLength, iType) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_ParseFIFOCmd failed.\n");
		return S_FAIL;
	}

	return S_OK;
}// End of RemoteFocusd_FIFOCommandHandler

/* =========================================================================================== */
SCODE RemoteFocusd_SelectHandler(TRemoteFocusdInfo *pThis, fd_set fds_Read)
{

	if (FD_ISSET(pThis->iCGIFd, &fds_Read))
	{
		if (RemoteFocusd_FdCgiCommandHandler(pThis) != S_OK)
		{
			REMOTEFOCUSD_DBGPRINT("RemoteFocusd_FdCgiCommandHandler failed.\n");
			return S_FAIL;
		}
	}
	else if (FD_ISSET(pThis->iFIFOFd, &fds_Read))
	{
		if (RemoteFocusd_FIFOCommandHandler(pThis) == S_FAIL_READ)
		{
			REMOTEFOCUSD_DBGPRINT("RemoteFocusd_FIFOCommandHandler failed.\n");
			REMOTEFOCUSD_DBGPRINT("Reinitialize FIFO...\n");
			FD_CLR(pThis->iFIFOFd, &pThis->fds_ReadSet);
			close(pThis->iFIFOFd);
			if (RemoteFocusd_InitFIFO(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitFIFO failed.\n");
			}

			return S_FAIL;
		}
	}

	return S_OK;
}// End of RemoteFocusd_SelectHandler

/* =========================================================================================== */
SCODE RemoteFocusd_InfoProcess(TRemoteFocusdInfo *pThis)
{
	fd_set fds_Read;
	struct timeval tvTimeOut;
	int iSelect;

	extern BOOL g_bTerminated;
	extern BOOL g_bRunThread;


	// Check if power loss unexpectedly
	RemoteFocusd_CheckLogFile(pThis);

	while (!g_bTerminated)
	{
		if (!g_bRunThread)
		{// Motor thread ends unexpectedly
			REMOTEFOCUSD_DBGPRINT("Motor thread ends unexpectedly!\n");
			return S_FAIL;
		}

		// For select function
		fds_Read = pThis->fds_ReadSet;
		tvTimeOut.tv_sec = REMOTEFOCUSD_SELECT_TIMEOUT / 1000000;
		tvTimeOut.tv_usec = REMOTEFOCUSD_SELECT_TIMEOUT % 1000000;

		iSelect = select(pThis->iMaxFd + 1, &fds_Read, NULL, NULL, &tvTimeOut);
		if (iSelect < 0)
		{// Select Error
			if (errno != EINTR)
			{
				REMOTEFOCUSD_DBGPRINT("select failed.\n");
			}
		}
		else if (iSelect > 0)
		{
			if (RemoteFocusd_SelectHandler(pThis, fds_Read) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SelectHandler failed.\n");
			}
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("select timeout\n");
		}
	}

	// Notify motor thread
	g_bRunThread = FALSE;

	return S_OK;
}// End of RemoteFocusd_InfoProcess
