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
 * remotefocusd_thread.c
 *
 * \brief
 * C source file of remote focus daemon
 * It includes thread functions and related procedures.
 * The main purpose is controlling the zoom and focus motors.
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



extern DWORD g_dwtabFocusValueTable[];
extern BOOL g_bIrisFullyOpenStatus;

#ifdef _TABLE_MAPPING
extern const DWORD g_dwtabFocusPanTable[];
#else
extern const DWORD g_dwtabFocusPositionTable[];
#endif

#ifdef _FOCUS_DEFAULT
extern DWORD g_dwFocusDefault;
#endif

#ifdef _FOCUSING_START
extern const DWORD g_dwtabFocusingStartTable[];
#endif

#define IRISOPEN        TRUE
#define IRISENABLE      FALSE

/* =========================================================================================== */
void RemoteFocusd_EnableFetch(TRemoteFocusdInfo *pThis, BOOL bOperationOn)
{
	TRemoteFocusd_ShareMemInfo* ptShareMemInfo = pThis->ptShareMemInfo;


	ptShareMemInfo->bFetchEnable = bOperationOn;
	return;
}

/* =========================================================================================== */
void RemoteFocusd_FetchFocusValue(TRemoteFocusdInfo *pThis)
{
	TRemoteFocusd_ShareMemInfo* ptShareMemInfo = pThis->ptShareMemInfo;


	pThis->dwFocusValue = ptShareMemInfo->dwFocusValue;
	return;
}

/* =========================================================================================== */
void RemoteFocusd_SetIris(TRemoteFocusdInfo *pThis, BOOL bIrisStatus)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_ShareMemInfo* ptShareMemInfo = pThis->ptShareMemInfo;
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);

	ptShareMemInfo->bFullyOpenedIris = bIrisStatus;
	ptShareMemInfo->bIrisEnable = TRUE;

	// Wait till the setting is done
	while (ptShareMemInfo->bFullyOpenedIris == TRUE && ptShareMemInfo->bIrisEnable == TRUE)
		usleep(REMOTEFOCUSD_OPEN_IRIS_TIME);

	return;
}

/* =========================================================================================== */
void RemoteFocusd_NotifyStartStopAutoFocus(TRemoteFocusdInfo *pThis, BOOL bStatus)
{
	if (!(pThis->iIsSupportFocus))	return;

	printf("RemoteFocusd_NotifyStartStopAutoFocus bStatus is %d\n", bStatus);
	TRemoteFocusd_ShareMemInfo* ptShareMemInfo = pThis->ptShareMemInfo;

	ptShareMemInfo->bIsStartAutoFocus = bStatus;
	ptShareMemInfo->bAutoFocusEnable = TRUE;

	return;
}

/* =========================================================================================== */
SCODE RemoteFocusd_SaveConfigFile(TRemoteFocusdInfo *pThis)
{
	FILE *pfConfig;


	sem_wait(&pThis->semFileMutex);

	if ((pfConfig = fopen(REMOTEFOCUSD_CONFIG_FILE, "w")) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("fopen failed!\n");
		return S_FAIL;
	}
	if (pThis->iIsSupportZoom)
	{
		fprintf(pfConfig, "zoom_motor=%d\n", pThis->dwZoomMotorPosition);
	}

	if (pThis->iIsSupportFocus)
	{
		fprintf(pfConfig, "focus_motor=%d\n", pThis->dwFocusMotorPosition);
	}
	fclose(pfConfig);

	sem_post(&pThis->semFileMutex);
	return S_OK;
}

#ifdef _FOCUS_DEFAULT
SCODE RemoteFocusd_SaveFocusDefaultConfig(TRemoteFocusdInfo *pThis)
{
	FILE *pfConfig;

	sem_wait(&pThis->semFileMutex);

	if ((pfConfig = fopen(REMOTEFOCUSD_FOCUS_DEFAULT_FILE, "w")) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("fopen failed!\n");
		return S_FAIL;
	}
		fprintf(pfConfig, "%d\n", g_dwFocusDefault);
	fclose(pfConfig);

	sem_post(&pThis->semFileMutex);
	return S_OK;
}
#endif

/* =========================================================================================== */
void RemoteFocusd_ZoomCreateLogFile(void)
{
	int fdLogFile;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;


	fdLogFile = open(REMOTEFOCUSD_ZOOM_LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, mode);
	
	if (fdLogFile < 0)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_ZoomCreateLogFile failed!\n");
		return;
	}
	
	close(fdLogFile);
	sync();

	return;
}

/* =========================================================================================== */
void RemoteFocusd_ZoomDeleteLogFile(void)
{

	if (remove(REMOTEFOCUSD_ZOOM_LOG_FILE) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_ZoomDeleteLogFile failed!\n");
		return;
	}
	sync();

	return;
}

/* =========================================================================================== */
void RemoteFocusd_FocusCreateLogFile(void)
{
	int fdLogFile;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;


	fdLogFile = open(REMOTEFOCUSD_FOCUS_LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, mode);
	
	if (fdLogFile < 0)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_ZoomCreateLogFile failed!\n");
		return;
	}
	
	close(fdLogFile);
	sync();

	return;
}

/* =========================================================================================== */
void RemoteFocusd_FocusDeleteLogFile(void)
{

	if (remove(REMOTEFOCUSD_FOCUS_LOG_FILE) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_ZoomDeleteLogFile failed!\n");
		return;
	}
	sync();

	return;
}

/* =========================================================================================== */
void RemoteFocusd_CalculatePanFocusPosition(TRemoteFocusdInfo *pThis, DWORD dwZoomPosition)
{
	if (!(pThis->iIsSupportFocus))	return;

#ifdef _TABLE_MAPPING
	// table mapping
	if (dwZoomPosition <= 0)
	{
		pThis->dwPanFocusPosition = g_dwtabFocusPanTable[0];
	}
	else if (dwZoomPosition > REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP)
	{
		pThis->dwPanFocusPosition = g_dwtabFocusPanTable[REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP - 1];
	}
	else
	{
		pThis->dwPanFocusPosition = g_dwtabFocusPanTable[dwZoomPosition - 1];
	}

#else
	DWORD dwInterval;
	DWORD dwTableIndex;
	DWORD dwIntervalIndex;
	DWORD dwIntervalLowerBound;

	dwTableIndex = dwZoomPosition / REMOTEFOCUSD_ZOOM_POSITION_INTERVAL;
	if (dwTableIndex < (REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE - 1))
	{// Search by interpolation
		dwIntervalLowerBound = g_dwtabFocusPositionTable[dwTableIndex];
		dwInterval = (DWORD)abs((int)g_dwtabFocusPositionTable[dwTableIndex] - (int)g_dwtabFocusPositionTable[dwTableIndex + 1]);

		dwIntervalIndex = dwZoomPosition % REMOTEFOCUSD_ZOOM_POSITION_INTERVAL;

		pThis->dwPanFocusPosition = dwIntervalLowerBound + ((DWORD)
												((((double)dwInterval)
													/ REMOTEFOCUSD_ZOOM_POSITION_INTERVAL) * ((double)dwIntervalIndex)));
	}
	else
	{
		pThis->dwPanFocusPosition = g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE - 1];
	}
#endif

	return;
}

/* =========================================================================================== */
#ifdef _FOCUSING_START
void RemoteFocusd_CalculateFocusingStart(TRemoteFocusdInfo *pThis, DWORD dwZoomPosition)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);

	DWORD dwInterval;
	DWORD dwTableIndex;
	DWORD dwIntervalIndex;
	DWORD dwIntervalLowerBound;


	dwTableIndex = dwZoomPosition / REMOTEFOCUSD_ZOOM_POSITION_INTERVAL;
	if (dwTableIndex < (REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE - 1))
	{// Search by interpolation
		dwIntervalLowerBound = g_dwtabFocusingStartTable[dwTableIndex];
		dwInterval = (DWORD)abs((int)g_dwtabFocusingStartTable[dwTableIndex] - (int)g_dwtabFocusingStartTable[dwTableIndex + 1]);

		dwIntervalIndex = dwZoomPosition % REMOTEFOCUSD_ZOOM_POSITION_INTERVAL;

		ptFocusMotorCtrlInfo->dwMotorStart = dwIntervalLowerBound + ((DWORD)
												((((double)dwInterval)
													/ REMOTEFOCUSD_ZOOM_POSITION_INTERVAL) * ((double)dwIntervalIndex)));
	}
	else
	{
		ptFocusMotorCtrlInfo->dwMotorStart = g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE - 1];
	}

	return;
}
#endif

/* =========================================================================================== */
void RemoteFocusd_AdjustFocus(TRemoteFocusdInfo *pThis, DWORD dwZoomPosition)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);

	RemoteFocusd_CalculatePanFocusPosition(pThis, dwZoomPosition);

#ifdef _FOCUSING_START
	// Added by Jiun, 2012/06/18
	RemoteFocusd_CalculateFocusingStart(pThis, dwZoomPosition);			
#endif

	ptFocusMotorCtrlInfo->dwMotorPosition = pThis->dwPanFocusPosition;
	ptFocusMotorCtrlInfo->eMotorDirection = emdDriect;
	ptFocusMotorCtrlInfo->eFunctionType = eftFocus;
	pThis->bFocusOperationEnable = TRUE;
	if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
					sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Waking up focus thread failed.\n");
	}

	return;
}// End of RemoteFocusd_AdjustFocus

/* =========================================================================================== */
void RemoteFocusd_PerformNextFunction(TRemoteFocusdInfo *pThis)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);


	if (pThis->eNextFuncType == eftAutoFocus)
	{
		REMOTEFOCUSD_DBGPRINT("Start auto focus...\n");
		ptFocusMotorCtrlInfo->bFullyOpenedIris = TRUE;
		ptFocusMotorCtrlInfo->eFunctionType = pThis->eNextFuncType;
	}
	else if (pThis->eNextFuncType == eftFocusScan)
	{
		REMOTEFOCUSD_DBGPRINT("Start focus scan...\n");
		ptFocusMotorCtrlInfo->bFullyOpenedIris = TRUE;
		ptFocusMotorCtrlInfo->eFunctionType = pThis->eNextFuncType;
	}

	pThis->bFocusOperationEnable = TRUE;
	if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
					sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Waking up focus motor thread failed.\n");
	}

	return;
}// End of RemoteFocusd_PerformNextFunction

/* =========================================================================================== */
void RemoteFocusd_ZoomStepHandler(TRemoteFocusdInfo *pThis, int fdMotorDevice, DWORD dwWalkStep, BOOL bIsForward)
{
	if (!(pThis->iIsSupportZoom))	return;

	int iRet;
	DWORD dwExtraVirtualSteps;
	DWORD dwRemainingSteps;


	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_CLR);
	REMOTEFOCUSD_DBGPRINT("Remaining zoom steps: %d\n", iRet);
	if (iRet > 0)
	{
		dwExtraVirtualSteps = dwWalkStep - (iRet / REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
		if (bIsForward)
		{
			pThis->dwZoomMotorPosition += dwExtraVirtualSteps;
		}
		else
		{
			pThis->dwZoomMotorPosition -= dwExtraVirtualSteps;
		}

		dwRemainingSteps = iRet % REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP;
		if (dwRemainingSteps != 0)
		{
#ifdef _BLOCKING_MODE
			iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK, dwRemainingSteps);
#else
			iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS, dwRemainingSteps);
			usleep(REMOTEFOCUSD_ZOOM_ONESTEP_TIME);
#endif
		}
	}
	
	return;
}

/* =========================================================================================== */
SCODE RemoteFocusd_MoveZoomMotor(TRemoteFocusdInfo *pThis, int fdMotorDevice, DWORD dwPosition)
{
	if (!(pThis->iIsSupportZoom))	return S_FAIL;

	int iRet, iAdjustStep;
	DWORD dwWalkStep, dwCtrlType;


	// Force to (REMOTEFOCUSD_ZOOM_VIRTUAL_START ~ REMOTEFOCUSD_ZOOM_VIRTUAL_END)
	// 65535 means underflow
	dwPosition = (dwPosition > 65535)? REMOTEFOCUSD_ZOOM_VIRTUAL_START : ((dwPosition > REMOTEFOCUSD_ZOOM_VIRTUAL_END)?
						REMOTEFOCUSD_ZOOM_VIRTUAL_END : dwPosition);

	//Move focus motor to the relative position
	RemoteFocusd_AdjustFocus(pThis, dwPosition);

	if (dwPosition >= pThis->dwZoomMotorPosition)
	{// Move forward
		ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
		dwWalkStep = dwPosition - pThis->dwZoomMotorPosition;

#ifdef _BLOCKING_MODE
		pThis->dwZoomMotorPosition += dwWalkStep;
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK,
						dwWalkStep * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
#else
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS,
						dwWalkStep * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);

		while ((dwWalkStep --) != 0)
		{
			usleep(REMOTEFOCUSD_ZOOM_ONESTEP_TIME);
			pThis->dwZoomMotorPosition ++;

			if (pThis->bZoomOperationEnable)
			{
				REMOTEFOCUSD_DBGPRINT("Operation interrupted!\n");
				RemoteFocusd_ZoomStepHandler(pThis, fdMotorDevice, dwWalkStep, TRUE);

				return S_FAIL;
			}
		}
#endif
	}
	else
	{// Move backward
		ioctl(fdMotorDevice, MOTOR_SET_BACKWARD);
		dwWalkStep = pThis->dwZoomMotorPosition - dwPosition;
#ifdef _BLOCKING_MODE
		pThis->dwZoomMotorPosition -= dwWalkStep;
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK,
						dwWalkStep * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
#else
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS,
						dwWalkStep * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
		while ((dwWalkStep --) != 0)
		{
			usleep(REMOTEFOCUSD_ZOOM_ONESTEP_TIME);
			pThis->dwZoomMotorPosition --;

			if (pThis->bZoomOperationEnable)
			{
				REMOTEFOCUSD_DBGPRINT("Operation interrupted!\n");
				RemoteFocusd_ZoomStepHandler(pThis, fdMotorDevice, dwWalkStep, FALSE);

				return S_FAIL;
			}
		}
#endif
		//* Note: 
		//*   Because of the motor's feature,
		//*   motor should move some more steps and
		//*   then move back

#ifdef _DIFFERENT_ZOOM_FOCUS_ADJUST_STEP
		iAdjustStep = REMOTEFOCUSD_ZOOM_ADJUST_STEP;
#else
		iAdjustStep = REMOTEFOCUSD_ADJUST_STEP;
#endif

#ifndef _BLOCKING_MODE
		dwCtrlType = MOTOR_WALK_STEPS;
#else
		dwCtrlType = MOTOR_WALK_STEPS_BLOCK;
#endif
		if (dwPosition >= iAdjustStep)
		{
			iRet = ioctl(fdMotorDevice, dwCtrlType, iAdjustStep * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
#ifndef _BLOCKING_MODE
			usleep(iAdjustStep * REMOTEFOCUSD_ZOOM_ONESTEP_TIME);
#endif
			ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
			iRet = ioctl(fdMotorDevice, dwCtrlType, iAdjustStep * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);

#ifndef _BLOCKING_MODE
			usleep(iAdjustStep * REMOTEFOCUSD_ZOOM_ONESTEP_TIME);
#endif
		}
	}

	return S_OK;
}// End of RemoteFocusd_MoveZoomMotor

/* =========================================================================================== */
void RemoteFocusd_OperateZoomMotor(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportZoom))	return;

	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);


	if (ptZoomMotorCtrlInfo->eMotorDirection == emdDriect)
	{
		if (RemoteFocusd_MoveZoomMotor(pThis, fdMotorDevice, ptZoomMotorCtrlInfo->dwMotorPosition) != S_OK)
		{
			return;
		}
	}
	else
	{// Fine-tune
		// Force to (0 ~ 5)
		ptZoomMotorCtrlInfo->dwMotorSteps = (ptZoomMotorCtrlInfo->dwMotorSteps < 0)?
			0 : ((ptZoomMotorCtrlInfo->dwMotorSteps > 30)? 30 : ptZoomMotorCtrlInfo->dwMotorSteps);

		if (ptZoomMotorCtrlInfo->eMotorDirection == emdForward)
		{
			if (RemoteFocusd_MoveZoomMotor(pThis, fdMotorDevice,
						pThis->dwZoomMotorPosition + ptZoomMotorCtrlInfo->dwMotorSteps) != S_OK)
			{
				return;
			}
		}
		else if (ptZoomMotorCtrlInfo->eMotorDirection == emdBackward)
		{
			if (RemoteFocusd_MoveZoomMotor(pThis, fdMotorDevice,
						pThis->dwZoomMotorPosition - ptZoomMotorCtrlInfo->dwMotorSteps) != S_OK)
			{
				return;
			}
		}
	}

	if (pThis->eNextFuncType != eftNoService)
	{
		RemoteFocusd_PerformNextFunction(pThis);
		pThis->eNextFuncType = eftNoService;
	}


	ptZoomMotorCtrlInfo->eFunctionType = eftNoService;
	return;
}// End of RemoteFocusd_OperateZoomMotor

/* =========================================================================================== */
void RemoteFocusd_FocusStepHandler(TRemoteFocusdInfo *pThis, int fdMotorDevice, DWORD dwWalkStep, BOOL bIsForward)
{
	if (!(pThis->iIsSupportFocus))	return;

	int iRet;
	DWORD dwExtraVirtualSteps;
	DWORD dwRemainingSteps;


	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_CLR);
	REMOTEFOCUSD_DBGPRINT("Remaining focus steps: %d\n", iRet);
	if (iRet > 0)
	{
		dwExtraVirtualSteps = dwWalkStep - (iRet / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);
		if (bIsForward)
		{
			pThis->dwFocusMotorPosition += dwExtraVirtualSteps;
		}
		else
		{
			pThis->dwFocusMotorPosition -= dwExtraVirtualSteps;
		}

		dwRemainingSteps = iRet % REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP;
		if (dwRemainingSteps != 0)
		{
#ifdef _BLOCKING_MODE
			iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK, dwRemainingSteps);
#else
			iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS, dwRemainingSteps);
			usleep(REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
#endif
		}
	}

	return;
}

/* =========================================================================================== */
SCODE RemoteFocusd_MoveFocusMotor(TRemoteFocusdInfo *pThis, int fdMotorDevice, DWORD dwPosition)
{
	if (!(pThis->iIsSupportFocus))	return S_FAIL;

	int iRet, iAdjustStep;
	DWORD dwWalkStep, dwCtrlType;


	// Force to (REMOTEFOCUSD_FOCUS_VIRTUAL_START ~ REMOTEFOCUSD_FOCUS_VIRTUAL_END)
	// 65535 means underflow
	dwPosition = (dwPosition > 65535)? REMOTEFOCUSD_FOCUS_VIRTUAL_START : ((dwPosition > REMOTEFOCUSD_FOCUS_VIRTUAL_END)?
						REMOTEFOCUSD_FOCUS_VIRTUAL_END : dwPosition);


	if (dwPosition >= pThis->dwFocusMotorPosition)
	{// Move forward
		ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
		dwWalkStep = dwPosition - pThis->dwFocusMotorPosition;


#ifdef _BLOCKING_MODE
		pThis->dwFocusMotorPosition += dwWalkStep;
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK,
						dwWalkStep * REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);
#else
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS,
						dwWalkStep * REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);

		while ((dwWalkStep --) != 0)
		{
			usleep(REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
			pThis->dwFocusMotorPosition ++;

			if (pThis->bFocusOperationEnable)
			{
				REMOTEFOCUSD_DBGPRINT("Operation interrupted!\n");
				RemoteFocusd_FocusStepHandler(pThis, fdMotorDevice, dwWalkStep, TRUE);

				return S_FAIL;
			}
		}
#endif
	}
	else
	{// Move backward
		ioctl(fdMotorDevice, MOTOR_SET_BACKWARD);
		dwWalkStep = pThis->dwFocusMotorPosition - dwPosition;

		fprintf(stderr,"MoveFocusMotor(backward), dwWalkStep is %d\n", dwWalkStep);
#ifdef _BLOCKING_MODE
		pThis->dwFocusMotorPosition -= dwWalkStep;
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK,
						dwWalkStep * REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);
#else
		iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS,
						dwWalkStep * REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);
		while ((dwWalkStep --) != 0)
		{
			usleep(REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
			pThis->dwFocusMotorPosition --;

			if (pThis->bFocusOperationEnable)
			{
				REMOTEFOCUSD_DBGPRINT("Operation interrupted!\n");
				RemoteFocusd_FocusStepHandler(pThis, fdMotorDevice, dwWalkStep, FALSE);

				return S_FAIL;
			}
		}
#endif
		//* Note: 
		//*   Because of the motor's feature,
		//*   motor should move some more steps and
		//*   then move back
#ifdef _DIFFERENT_ZOOM_FOCUS_ADJUST_STEP
		iAdjustStep = REMOTEFOCUSD_FOCUS_ADJUST_STEP;
#else
		iAdjustStep = REMOTEFOCUSD_ADJUST_STEP;
#endif

#ifndef _BLOCKING_MODE
		dwCtrlType = MOTOR_WALK_STEPS;
#else
		dwCtrlType = MOTOR_WALK_STEPS_BLOCK;
#endif
		if (dwPosition >= iAdjustStep)
		{
			iRet = ioctl(fdMotorDevice, dwCtrlType, iAdjustStep * REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);
#ifndef _BLOCKING_MODE
			usleep(iAdjustStep * REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
#endif

			ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
			iRet = ioctl(fdMotorDevice, dwCtrlType, iAdjustStep * REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP);
#ifndef _BLOCKING_MODE
			usleep(iAdjustStep * REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
#endif
		}
	}

	return S_OK;
}// End of RemoteFocusd_MoveFocusMotor

/* =========================================================================================== */
void RemoteFocusd_OperateFocusMotor(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);


	if (ptFocusMotorCtrlInfo->eMotorDirection == emdDriect)
	{
		if (RemoteFocusd_MoveFocusMotor(pThis, fdMotorDevice, ptFocusMotorCtrlInfo->dwMotorPosition) != S_OK)
		{
			return;
		}
	}
	else
	{// Fine-tune
		// Force to (0 ~ 5)
		ptFocusMotorCtrlInfo->dwMotorSteps = (ptFocusMotorCtrlInfo->dwMotorSteps < 0)?
			0 : ((ptFocusMotorCtrlInfo->dwMotorSteps > 30)? 30 : ptFocusMotorCtrlInfo->dwMotorSteps);
	
		if (ptFocusMotorCtrlInfo->eMotorDirection == emdForward)
		{
			if (RemoteFocusd_MoveFocusMotor(pThis, fdMotorDevice,
						pThis->dwFocusMotorPosition + ptFocusMotorCtrlInfo->dwMotorSteps) != S_OK)
			{
				return;
			}
		}
		else if (ptFocusMotorCtrlInfo->eMotorDirection == emdBackward)
		{
			if (RemoteFocusd_MoveFocusMotor(pThis, fdMotorDevice,
						pThis->dwFocusMotorPosition - ptFocusMotorCtrlInfo->dwMotorSteps) != S_OK)
			{
				return;
			}
		}
	}

	ptFocusMotorCtrlInfo->eFunctionType = eftNoService;
	return;
}// End of RemoteFocusd_OperateFocusMotor

/* =========================================================================================== */
SCODE RemoteFocusd_StepByStepSearch(TRemoteFocusdInfo *pThis, int fdMotorDevice,
										DWORD dwStart, DWORD dwSearchRange, DWORD dwStride)
{
	if (!(pThis->iIsSupportFocus))	return S_FAIL;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	int iRet;
	DWORD dwPosition;
	DWORD dwBestPosition;
	DWORD dwBestFocus;
	DWORD dwWalkStep;
	DWORD dwCtrlType;



	// Move to start point
	if (RemoteFocusd_MoveFocusMotor(pThis, fdMotorDevice, dwStart) != S_OK)
	{
		RemoteFocusd_EnableFetch(pThis, FALSE);
		return S_FAIL;
	}


	RemoteFocusd_EnableFetch(pThis, TRUE);
	usleep(1000000);     // Waitting for share memory to parpers fetch FV.
	RemoteFocusd_FetchFocusValue(pThis);
	g_dwtabFocusValueTable[0] = pThis->dwFocusValue;
	dwBestFocus = pThis->dwFocusValue;
	dwBestPosition = 0;
	REMOTEFOCUSD_DBGPRINT("Initial Focus Value: %d.\n", pThis->dwFocusValue);

	// Start Searching
	ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
	for (dwPosition = dwStride ; dwPosition <= dwSearchRange ; dwPosition += dwStride)
	{
		if (pThis->bFocusOperationEnable)
		{
			REMOTEFOCUSD_DBGPRINT("Operation interrupted!\n");
			RemoteFocusd_EnableFetch(pThis, FALSE);
			return S_FAIL;
		}
#ifdef _BLOCKING_MODE
		dwCtrlType = MOTOR_WALK_STEPS_BLOCK;
#else
		dwCtrlType = MOTOR_WALK_STEPS;
#endif
		iRet = ioctl(fdMotorDevice, dwCtrlType, REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP * dwStride);

		//Whether blocking mode or not, need to wait for the image becomes stable.
		usleep(REMOTEFOCUSD_ONESTEP_SEARCH_TIME);

		pThis->dwFocusMotorPosition += dwStride;

		RemoteFocusd_FetchFocusValue(pThis);
		REMOTEFOCUSD_DBGPRINT("Step: %d, Focus Value: %d.\n", dwPosition, pThis->dwFocusValue);

		if (ptFocusMotorCtrlInfo->eFunctionType == eftFocusScan)
		{
			g_dwtabFocusValueTable[dwPosition] = pThis->dwFocusValue;
		}

		if (pThis->dwFocusValue > dwBestFocus)
		{
			dwBestFocus = pThis->dwFocusValue;
			dwBestPosition = dwPosition;
		}
	}

	// Move to the position with the best focus
	dwWalkStep = dwPosition - dwStride - dwBestPosition;
	if (RemoteFocusd_MoveFocusMotor(pThis, fdMotorDevice,
				pThis->dwFocusMotorPosition - dwWalkStep) != S_OK)
	{
		RemoteFocusd_EnableFetch(pThis, FALSE);
		return S_FAIL;
	}

	REMOTEFOCUSD_DBGPRINT("Best focus: %d, Motor position: %d.\n", dwBestFocus, dwBestPosition);
	RemoteFocusd_EnableFetch(pThis, FALSE);

	return S_OK;
}// End of RemoteFocusd_StepByStepSearch

/* =========================================================================================== */
void RemoteFocusd_PerformAutoFocus(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	DWORD dwWalkStep;
	DWORD dwSearchStart = ptFocusMotorCtrlInfo->dwMotorStart;
	DWORD dwValidRange = ptFocusMotorCtrlInfo->dwMotorEnd - ptFocusMotorCtrlInfo->dwMotorStart;
	BOOL bNeedCloseIris = FALSE;
	fprintf(stderr,"=======RemoteFocusd_PerformAutoFocus()=======\n");
	fprintf(stderr,"ptFocusMotorCtrlInfo->dwMotorEnd is %d, ptFocusMotorCtrlInfo->dwMotorStart is %d\n",ptFocusMotorCtrlInfo->dwMotorEnd,ptFocusMotorCtrlInfo->dwMotorStart);
	fprintf(stderr,"pThis->dwFocusMotorPosition is %d\n",pThis->dwFocusMotorPosition);
	fprintf(stderr,"pThis->dwPanFocusPosition is %d\n",pThis->dwPanFocusPosition);

	if(ptFocusMotorCtrlInfo->bFullyOpenedIris)
	{
		RemoteFocusd_SetIris(pThis, IRISOPEN);
		bNeedCloseIris =TRUE;
	}

	// nofity venc that auto focus is starting
	RemoteFocusd_NotifyStartStopAutoFocus(pThis, TRUE);

	if (dwValidRange > REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE)
	{
		dwWalkStep = REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE >> 1;
		if (pThis->dwPanFocusPosition < ((REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE >> 1) + ptFocusMotorCtrlInfo->dwMotorStart))
		{	//First Check boundary of near end
			fprintf(stderr,"1.First Check boundary of near end\n");
			dwWalkStep = pThis->dwPanFocusPosition - ptFocusMotorCtrlInfo->dwMotorStart;

			dwSearchStart = pThis->dwPanFocusPosition - dwWalkStep;
			dwValidRange = REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE;
		}
		else if ((ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwPanFocusPosition)
					< (REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE >> 1))
		{	//First Check boundary of far end 
			fprintf(stderr,"2.First Check boundary of far end\n");
			dwWalkStep = REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE
							- (ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwPanFocusPosition);

			dwSearchStart = pThis->dwPanFocusPosition - dwWalkStep;
			dwValidRange = REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE;
		}
		else
		{
			fprintf(stderr,"33333333333333333\n");
			dwSearchStart = pThis->dwPanFocusPosition - dwWalkStep;
			dwValidRange = REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE;
		}
	}

	REMOTEFOCUSD_DBGPRINT("Start first-level searching...\n");
	if (RemoteFocusd_StepByStepSearch(pThis, fdMotorDevice, dwSearchStart, dwValidRange, REMOTEFOCUSD_LARGE_SEARCH_STEP) != S_OK)
	{
		//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
		if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
		{
			RemoteFocusd_SetIris(pThis, IRISENABLE);
		}
		RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);
		return;
	}


	dwWalkStep = REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE >> 1;
	if (pThis->dwFocusMotorPosition < ((REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE >> 1) + ptFocusMotorCtrlInfo->dwMotorStart))
	{	//Second Check boundary of near end
		dwWalkStep = pThis->dwFocusMotorPosition - ptFocusMotorCtrlInfo->dwMotorStart;

	}
	else if ((ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwFocusMotorPosition)
					< (REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE >> 1))
	{	//Second Check boundary of far end
		dwWalkStep = REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE
						- (ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwFocusMotorPosition);
	}

	REMOTEFOCUSD_DBGPRINT("Start second-level searching...\n");
	if (RemoteFocusd_StepByStepSearch(pThis, fdMotorDevice, pThis->dwFocusMotorPosition - dwWalkStep, REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE, REMOTEFOCUSD_SMALL_SEARCH_STEP) != S_OK)
	{
		//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
		if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
		{
			RemoteFocusd_SetIris(pThis, IRISENABLE);
		}
		RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);
		return;
	}

#ifdef _SUPPORT_FINE_TUNING	
	dwWalkStep = REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE >> 1;
	if (pThis->dwFocusMotorPosition < ((REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE >> 1) + ptFocusMotorCtrlInfo->dwMotorStart))
	{	//Third Check boundary of near end
		dwWalkStep = pThis->dwFocusMotorPosition - ptFocusMotorCtrlInfo->dwMotorStart;

	}
	else if ((ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwFocusMotorPosition)
					< (REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE >> 1))
	{	//Third Check boundary of far end
		dwWalkStep = REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE
						- (ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwFocusMotorPosition);
	}

	REMOTEFOCUSD_DBGPRINT("Start Third-level searching...\n");
	if (RemoteFocusd_StepByStepSearch(pThis, fdMotorDevice, pThis->dwFocusMotorPosition - dwWalkStep, REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE, REMOTEFOCUSD_FINE_TUNE_SEARCH_STEP) != S_OK)
	{
		//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
		if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
		{
			RemoteFocusd_SetIris(pThis, IRISENABLE);
		}
		RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);
		return;
	}
#endif

	ptFocusMotorCtrlInfo->eFunctionType = eftNoService;
	//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
	if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
	{
		RemoteFocusd_SetIris(pThis, IRISENABLE);
	}

	// nofity venc that auto focus is stopping 
	RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);

	return;
}// End of RemoteFocusd_PerformAutoFocus

/* =========================================================================================== */
void RemoteFocusd_PerformFocusScan(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
	BOOL bNeedCloseIris = FALSE;

	if(ptFocusMotorCtrlInfo->bFullyOpenedIris)
	{
		RemoteFocusd_SetIris(pThis, IRISOPEN);
		bNeedCloseIris = TRUE;
	}

	// nofity venc that focus scan is starting 
	RemoteFocusd_NotifyStartStopAutoFocus(pThis, TRUE);

	// Reset focus value table
	memset(g_dwtabFocusValueTable, 0, sizeof(DWORD) * REMOTEFOCUSD_FOCUS_VALUE_TABLE_SIZE);

	// Added by Jiun, 2012/6/14
	// Motor needs to positioning before full scan  
	DWORD dwBackupZoomPosition = pThis->dwZoomMotorPosition;
	DWORD dwWalkStep;
	DWORD dwSearchStart;
	DWORD dwValidRange;
	DWORD dwCtrlType;
	DWORD dwFocusMotorPos;
	
	if ((pThis->iIsSupportZoom))
	{
		//Zoom motor positioning
		ptZoomMotorCtrlInfo->eFunctionType = eftPositioning;
		ptZoomMotorCtrlInfo->dwMotorPosition = dwBackupZoomPosition;
		pThis->bZoomOperationEnable = TRUE;
		if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
					sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
		{
			REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
		}
	}

	//Focus motor positioning
#ifdef _FOCUS_CALIBRATION_REVERS
	ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
	dwFocusMotorPos = REMOTEFOCUSD_FOCUS_VIRTUAL_END;
#else
	ioctl(fdMotorDevice, MOTOR_SET_BACKWARD);
	dwFocusMotorPos = 0;
#endif

#ifdef _BLOCKING_MODE
	dwCtrlType = MOTOR_WALK_STEPS_BLOCK;
#else
	dwCtrlType = MOTOR_WALK_STEPS;
#endif

	ioctl(fdMotorDevice, dwCtrlType, REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);

	// we must make sure zoom positioning is done, and then do the step by step search
	if (REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP > REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP)
	{
		usleep((REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP - REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP) * REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
	}

#ifdef _BLOCKING_MODE
	pThis->dwFocusMotorPosition = dwFocusMotorPos;
#else
	dwWalkStep = REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP;
	while ((dwWalkStep --) != 0)
	{
		usleep(REMOTEFOCUSD_FOCUS_ONESTEP_TIME);
		if (pThis->dwFocusMotorPosition != 0)
			pThis->dwFocusMotorPosition --;

		if (pThis->bFocusOperationEnable)
		{
			REMOTEFOCUSD_DBGPRINT("Operation interrupted!\n");

			return S_FAIL;
		}
	}
#endif
	// Perform full scan
	REMOTEFOCUSD_DBGPRINT("Start searching...\n");
	dwSearchStart = ptFocusMotorCtrlInfo->dwMotorStart;
	dwValidRange = ptFocusMotorCtrlInfo->dwMotorEnd - ptFocusMotorCtrlInfo->dwMotorStart;
	if (RemoteFocusd_StepByStepSearch(pThis, fdMotorDevice, dwSearchStart, dwValidRange, REMOTEFOCUSD_LARGE_SEARCH_STEP) != S_OK)
	{
		//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
		if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
		{
			RemoteFocusd_SetIris(pThis, IRISENABLE);
		}
		RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);
		return;
	}

#ifdef _SUPPORT_FINE_TUNING	
	// Fine tune
	dwWalkStep = REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE >> 1;
	if (pThis->dwFocusMotorPosition < ((REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE >> 1) + ptFocusMotorCtrlInfo->dwMotorStart))
	{	//Third Check boundary of near end
		dwWalkStep = pThis->dwFocusMotorPosition - ptFocusMotorCtrlInfo->dwMotorStart;

	}
	else if ((ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwFocusMotorPosition)
					< (REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE >> 1))
	{	//Third Check boundary of far end
		dwWalkStep = REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE
						- (ptFocusMotorCtrlInfo->dwMotorEnd - pThis->dwFocusMotorPosition);
	}

	REMOTEFOCUSD_DBGPRINT("Start Fine tune searching...\n");
	if (RemoteFocusd_StepByStepSearch(pThis, fdMotorDevice, pThis->dwFocusMotorPosition - dwWalkStep, REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE, REMOTEFOCUSD_FINE_TUNE_SEARCH_STEP) != S_OK)
	{
		//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
		if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
		{
			RemoteFocusd_SetIris(pThis, IRISENABLE);
		}
		RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);
		return;
	}
#endif
	
	ptFocusMotorCtrlInfo->eFunctionType = eftNoService;

	//If IRIS is not set to irisopen (is irisenable), the iris status will leave full open iris after remotefocus finishing.
	if(g_bIrisFullyOpenStatus == FALSE && bNeedCloseIris == TRUE)
	{
			RemoteFocusd_SetIris(pThis, IRISENABLE);
	}

	// nofity venc that focus scan is stopping 
	RemoteFocusd_NotifyStartStopAutoFocus(pThis, FALSE);

	return;
}// End of RemoteFocusd_PerformFocusScan

void RemoteFocusd_PositioningAutoFocus(TRemoteFocusdInfo *pThis)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	DWORD dwZoomPosition = ptZoomMotorCtrlInfo->dwMotorPosition;

	if (pThis->iIsSupportZoom)
	{
#ifdef _FOCUSING_START
		// Added by Jiun, 2012/06/18
		// Focus
		RemoteFocusd_CalculateFocusingStart(pThis, dwZoomPosition);
#endif
	}
	ptFocusMotorCtrlInfo->eFunctionType = eftAutoFocus;
	pThis->bFocusOperationEnable = TRUE;
	if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
					sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Waking up focus thread failed.\n");
	}

	return;
}// End of RemoteFocusd_PositioningAutoFocus

/* =========================================================================================== */
void RemoteFocusd_FocusMotorPositioning(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	fprintf(stderr,"======RemoteFocusd_FocusMotorPositioning======\n");
	if (!(pThis->iIsSupportFocus))	return;

	int iRet;
	DWORD dwFocusPos;

#ifdef _FOCUS_CALIBRATION_REVERS
	ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
	dwFocusPos = REMOTEFOCUSD_FOCUS_VIRTUAL_END;
#else
	ioctl(fdMotorDevice, MOTOR_SET_BACKWARD);
	dwFocusPos = 0;
#endif

#ifdef _BLOCKING_MODE
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK, REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);
#else
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS, REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);
	usleep((REMOTEFOCUSD_FOCUS_ONESTEP_TIME / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP) * REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);
#endif
	pThis->dwFocusMotorPosition = dwFocusPos;

	/*****************
	added by Logic

	When positioning(calibration), RemoteFocusd_ZoomMotorPositioning() in zoom thread does this part too
	but focus thread get "eftPositioning" message early than Zoom thread
	so we need to set zoom position=0 here to do auto focus
	*****************/
	pThis->dwZoomMotorPosition = REMOTEFOCUSD_ZOOM_VIRTUAL_START;//normally REMOTEFOCUSD_ZOOM_VIRTUAL_START=0
	RemoteFocusd_CalculatePanFocusPosition(pThis, pThis->dwZoomMotorPosition);

	RemoteFocusd_PositioningAutoFocus(pThis);

	return;
}


/* =========================================================================================== */
#ifdef _FOCUS_DEFAULT
void RemoteFocusd_FocusMotorReset(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportFocus))	return;

	int iRet;
	DWORD dwFocusPos;
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);

	//Move focus motor back to focusdefault position
#ifdef _FOCUS_CALIBRATION_REVERS
	ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
	dwFocusPos = REMOTEFOCUSD_FOCUS_VIRTUAL_END;
#else
	ioctl(fdMotorDevice, MOTOR_SET_BACKWARD);
	dwFocusPos = 0;
#endif

#ifdef _BLOCKING_MODE
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK, REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);
#else
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS, REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);
	usleep((REMOTEFOCUSD_FOCUS_ONESTEP_TIME / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP) * REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP);
#endif
	pThis->dwFocusMotorPosition = dwFocusPos;

	//Move focus motor back to focusdefault position
	ptFocusMotorCtrlInfo->dwMotorPosition = g_dwFocusDefault ;
	ptFocusMotorCtrlInfo->eMotorDirection = emdDriect;
	ptFocusMotorCtrlInfo->eFunctionType = eftFocus;
	pThis->bFocusOperationEnable = TRUE;
	if (send(pThis->fdFocusMotorWakeup[0], &(pThis->bFocusOperationEnable),
				sizeof(pThis->bFocusOperationEnable), MSG_DONTWAIT) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Waking up focus thread failed.\n");
	}

	return;
}
#endif

/* =========================================================================================== */
void RemoteFocusd_ZoomMotorPositioning(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	fprintf(stderr,"======RemoteFocusd_ZoomMotorPositioning=======\n");

	if (!(pThis->iIsSupportZoom))	return;

	int iRet;
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);


	ioctl(fdMotorDevice, MOTOR_SET_BACKWARD);
#ifdef _BLOCKING_MODE
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK, REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP);
#else
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS, REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP);
	usleep((REMOTEFOCUSD_ZOOM_ONESTEP_TIME / REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP) * REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP);
#endif
	
    // Added by Jiun, 2012/6/14 
	// Move Zoom motor to assign point
	ioctl(fdMotorDevice, MOTOR_SET_FORWARD);
#ifdef _BLOCKING_MODE
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS_BLOCK,
				ptZoomMotorCtrlInfo->dwMotorPosition * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
#else
	iRet = ioctl(fdMotorDevice, MOTOR_WALK_STEPS,
				ptZoomMotorCtrlInfo->dwMotorPosition * REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP);
	usleep(ptZoomMotorCtrlInfo->dwMotorPosition * REMOTEFOCUSD_ZOOM_ONESTEP_TIME);
#endif
	pThis->dwZoomMotorPosition = ptZoomMotorCtrlInfo->dwMotorPosition;


	ptZoomMotorCtrlInfo->eFunctionType = eftNoService;
	return;
}

/* =========================================================================================== */
void RemoteFocusd_FocusMissionDispatcher(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportFocus))	return;

	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);


	switch (ptFocusMotorCtrlInfo->eFunctionType)
	{
		case eftFocus:
			RemoteFocusd_FocusCreateLogFile();

			RemoteFocusd_OperateFocusMotor(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_FocusDeleteLogFile();

			break;
		case eftAutoFocus:
			RemoteFocusd_FocusCreateLogFile();

			RemoteFocusd_PerformAutoFocus(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_FocusDeleteLogFile();

			break;
		case eftFocusScan:
			RemoteFocusd_FocusCreateLogFile();

			RemoteFocusd_PerformFocusScan(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_FocusDeleteLogFile();

			break;
		case eftPositioning:
			RemoteFocusd_FocusCreateLogFile();

			RemoteFocusd_FocusMotorPositioning(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_FocusDeleteLogFile();

			break;
		case eftStop:
			ptFocusMotorCtrlInfo->eFunctionType = eftNoService;
			break;
		case eftIrisOpen:
			RemoteFocusd_SetIris(pThis, IRISOPEN);
			ptFocusMotorCtrlInfo->eFunctionType = eftNoService;
			break;
		case eftIrisEnable:
			RemoteFocusd_SetIris(pThis, IRISENABLE);
			ptFocusMotorCtrlInfo->eFunctionType = eftNoService;
			break;
#ifdef _FOCUS_DEFAULT
		case eftResetFocus:
			RemoteFocusd_FocusCreateLogFile();

			RemoteFocusd_FocusMotorReset(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_FocusDeleteLogFile();

			break;
		case eftSetFocusDefault:
			g_dwFocusDefault = ptFocusMotorCtrlInfo->dwMotorFocusDefault;
			printf("g_dwFocusDefault =%d\n",g_dwFocusDefault);
			RemoteFocusd_SaveFocusDefaultConfig(pThis);
			ptFocusMotorCtrlInfo->eFunctionType = eftNoService;
			break;
#endif
		default:
			// Do nothing
			break;
	}// End of switch (ptFocusMotorCtrlInfo->eFunctionType)
	
	return;
}// End of RemoteFocusd_FocusMissionDispatcher

/* ============================================================================================= */
DWORD THREADAPI RemoteFocusd_FocusMotorThread(DWORD dwInstance)
{
	TRemoteFocusdInfo *pThis = (TRemoteFocusdInfo*) dwInstance;
	TRemoteFocusd_MotorCtrlInfo* ptFocusMotorCtrlInfo = &(pThis->tFocusMotorCtrlInfo);
	
	if (!(pThis->iIsSupportFocus))	return 0;

#ifndef _FOCUSING_START
	ptFocusMotorCtrlInfo->dwMotorStart = REMOTEFOCUSD_FOCUS_VIRTUAL_START;
#endif

	ptFocusMotorCtrlInfo->dwMotorEnd = REMOTEFOCUSD_FOCUS_VIRTUAL_END;
	int fdFocusMotorDevice;
	int iSelect;
	fd_set fdReadSet;
	struct timeval tvTimeOut;
	BOOL bReadBuf;

	extern BOOL g_bRunThread;


	fdFocusMotorDevice = open(REMOTEFOCUSD_FOCUS_MOTOR_DEVICE_FILE, O_WRONLY);
	if (fdFocusMotorDevice < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Failed to open device\n");
		g_bRunThread = FALSE;
		return -1;
	}

	// Turn off motors for thermal issue
#ifndef _CURRENT_CTRL
	ioctl(fdFocusMotorDevice, MOTOR_TURN_OFF);
#endif

	while (g_bRunThread)
	{
		// For select function
		FD_ZERO(&fdReadSet);
		FD_SET(pThis->fdFocusMotorWakeup[1], &fdReadSet);
		tvTimeOut.tv_sec = REMOTEFOCUSD_SELECT_TIMEOUT / 1000000;
		tvTimeOut.tv_usec = REMOTEFOCUSD_SELECT_TIMEOUT % 1000000;

		iSelect = select(pThis->fdFocusMotorWakeup[1] + 1, &fdReadSet, NULL, NULL, &tvTimeOut);
		if (iSelect < 0)
		{// Select Error
			if (errno != EINTR)
			{
				REMOTEFOCUSD_DBGPRINT("select failed.\n");
			}
		}
		else if (iSelect > 0)
		{
			read(pThis->fdFocusMotorWakeup[1], &bReadBuf, sizeof(bReadBuf));

			if (pThis->bFocusOperationEnable)
			{
				pThis->bFocusOperationEnable = FALSE;
				RemoteFocusd_FocusMissionDispatcher(pThis, fdFocusMotorDevice);

				// Turn off motors for thermal issue
#ifndef _CURRENT_CTRL
				ioctl(fdFocusMotorDevice, MOTOR_TURN_OFF);
#endif
			}
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("select timeout\n");
		}
	}// End of while (g_bRunThread)

	close(fdFocusMotorDevice);
	g_bRunThread = FALSE;
	return 0;
}// End of RemoteFocusd_FocusMotorThread

/* =========================================================================================== */
void RemoteFocusd_ZoomMissionDispatcher(TRemoteFocusdInfo *pThis, int fdMotorDevice)
{
	if (!(pThis->iIsSupportZoom))	return;
	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
	struct timeval tvButtonTimeValue;
	DWORD dwButtonReleaseTime;

	switch (ptZoomMotorCtrlInfo->eFunctionType)
	{
		case eftZoom:
			RemoteFocusd_ZoomCreateLogFile();

			RemoteFocusd_OperateZoomMotor(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_ZoomDeleteLogFile();

			//check stop
			if (pThis->tZoomMotorCtrlInfo.bKeepZooming)
			{
				//eftZooming
				gettimeofday(&tvButtonTimeValue, NULL);
				dwButtonReleaseTime = (DWORD)((tvButtonTimeValue.tv_usec / 1000) + (tvButtonTimeValue.tv_sec * 1000));
				if ((dwButtonReleaseTime - pThis->dwButtonPressTime) > 1000)
				{
					ptZoomMotorCtrlInfo->dwMotorSteps = (REMOTEFOCUSD_ZOOM_MOTOR_TOTALSTEP/10);
				}
				else
				{
					ptZoomMotorCtrlInfo->dwMotorSteps = 1;
				}
				ptZoomMotorCtrlInfo->eFunctionType = eftZoom;
				pThis->bZoomOperationEnable = TRUE;
				if (send(pThis->fdZoomMotorWakeup[0], &(pThis->bZoomOperationEnable),
							sizeof(pThis->bZoomOperationEnable), MSG_DONTWAIT) < 0)
				{
					REMOTEFOCUSD_DBGPRINT("Waking up zoom motor thread failed.\n");
				}
			}

			break;

		case eftPositioning:
			RemoteFocusd_ZoomCreateLogFile();

			RemoteFocusd_ZoomMotorPositioning(pThis, fdMotorDevice);
			if (RemoteFocusd_SaveConfigFile(pThis) != S_OK)
			{
				REMOTEFOCUSD_DBGPRINT("RemoteFocusd_SaveConfigFile failed.\n");
			}

			RemoteFocusd_ZoomDeleteLogFile();

			break;
		case eftStop:
			ptZoomMotorCtrlInfo->eFunctionType = eftNoService;

			break;
		default:
			// Do nothing
			break;
	}

	return;
}// End of RemoteFocusd_ZoomMissionDispatcher

/* ============================================================================================= */
DWORD THREADAPI RemoteFocusd_ZoomMotorThread(DWORD dwInstance)
{
	TRemoteFocusdInfo *pThis = (TRemoteFocusdInfo*) dwInstance;
	if (!(pThis->iIsSupportZoom))	return 0;

	TRemoteFocusd_MotorCtrlInfo* ptZoomMotorCtrlInfo = &(pThis->tZoomMotorCtrlInfo);
	ptZoomMotorCtrlInfo->dwMotorStart = REMOTEFOCUSD_ZOOM_VIRTUAL_START;
	ptZoomMotorCtrlInfo->dwMotorEnd = REMOTEFOCUSD_ZOOM_VIRTUAL_END;

	int fdZoomMotorDevice;

	int iSelect;
	fd_set fdReadSet;
	struct timeval tvTimeOut;
	BOOL bReadBuf;

	extern BOOL g_bRunThread;


	fdZoomMotorDevice = open(REMOTEFOCUSD_ZOOM_MOTOR_DEVICE_FILE, O_WRONLY);	
	if (fdZoomMotorDevice < 0)
	{
		REMOTEFOCUSD_DBGPRINT("Failed to open device\n");
		g_bRunThread = FALSE;
		return -1;
	}

	// Turn off motors for thermal issue
#ifndef _CURRENT_CTRL
	ioctl(fdZoomMotorDevice, MOTOR_TURN_OFF);
#endif

	RemoteFocusd_CalculatePanFocusPosition(pThis, pThis->dwZoomMotorPosition);
	
#ifdef _FOCUSING_START
	RemoteFocusd_CalculateFocusingStart(pThis,  pThis->dwZoomMotorPosition);					// Added by Jiun, 2012/06/18
#endif

	while (g_bRunThread)
	{
		// For select function
		FD_ZERO(&fdReadSet);
		FD_SET(pThis->fdZoomMotorWakeup[1], &fdReadSet);
		tvTimeOut.tv_sec = REMOTEFOCUSD_SELECT_TIMEOUT / 1000000;
		tvTimeOut.tv_usec = REMOTEFOCUSD_SELECT_TIMEOUT % 1000000;

		iSelect = select(pThis->fdZoomMotorWakeup[1] + 1, &fdReadSet, NULL, NULL, &tvTimeOut);
		if (iSelect < 0)
		{// Select Error
			if (errno != EINTR)
			{
				REMOTEFOCUSD_DBGPRINT("select failed.\n");
			}
		}
		else if (iSelect > 0)
		{
			read(pThis->fdZoomMotorWakeup[1], &bReadBuf, sizeof(bReadBuf));

			if (pThis->bZoomOperationEnable)
			{
				pThis->bZoomOperationEnable = FALSE;
				RemoteFocusd_ZoomMissionDispatcher(pThis, fdZoomMotorDevice);

				// Turn off motors for thermal issue
#ifndef _CURRENT_CTRL
				ioctl(fdZoomMotorDevice, MOTOR_TURN_OFF);
#endif
			}
		}
		else
		{
			REMOTEFOCUSD_DBGPRINT("select timeout\n");
		}
	}// End of while (g_bRunThread)

	close(fdZoomMotorDevice);
	g_bRunThread = FALSE;
	return 0;
}// End of RemoteFocusd_ZoomMotorThread
