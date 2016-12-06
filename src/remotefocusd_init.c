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
 * remotefocusd_init.c
 *
 * \brief
 * C source file of remote focus daemon
 * It includes initialization functions.
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

#ifdef _FOCUS_DEFAULT
extern DWORD g_dwFocusDefault;

/* ==================================================================== */

SCODE RemoteFocusd_LoadResetDefault()
{
	FILE *pfFocusDefault;
	char acParam[4];
	int iTableIndex = 0;

	if ((pfFocusDefault = fopen(REMOTEFOCUSD_FOCUS_DEFAULT_FILE, "r")) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("fopen failed!\n");
		return S_FAIL;
	}


	fscanf(pfFocusDefault, "%s", acParam);
	g_dwFocusDefault = atoi(acParam);


	fclose(pfFocusDefault);
	return S_OK;
} // End of RemoteFocusd_LoadConfigFile
#endif

#ifdef _TABLE_MAPPING
extern DWORD g_dwtabFocusPanTable[];

/* ==================================================================== */

SCODE RemoteFocusd_LoadMappingTable()
{
	FILE *pfMappingTable;
	char acParam[4];
	int iTableIndex = 0;

	if ((pfMappingTable = fopen(REMOTEFOCUSD_FOCUS_TABLE_MAPPING_FILE, "r")) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("fopen failed!\n");
		return S_FAIL;
	}

	while (fscanf(pfMappingTable, "%s", acParam) != EOF)
	{
		if (iTableIndex >= REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP)		break;	
		g_dwtabFocusPanTable[iTableIndex] = atoi(acParam);
		iTableIndex++;
	}

	fclose(pfMappingTable);
	return S_OK;
} // End of RemoteFocusd_LoadConfigFile
#else

#if defined(_FD8173) || defined(_FD8373) || defined(_IB8373)
DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE];
extern DWORD g_dome_dwtabFocusPositionTable[];
extern DWORD g_bullet_dwtabFocusPositionTable[];
#endif

#endif

/* ==================================================================== */
SCODE RemoteFocusd_LoadCapabilityFile(TRemoteFocusdInfo *ptInfo)
{
    SCODE   sReturn;
    char    acParseString[2][64];
    int 	iCapability;
	snprintf(acParseString[0], 64, "/root/capability/remotefocus");

	// notice : capability bitmap
	// 								(000) 0 : no support remotefocus
	//								(001) 1 : support zoom / focus
	//								(010) 2 : support zoom
	//								(100) 4 : support focus


    TCfgParseMap acCfgParseMap[] = {
	    {acParseString[0], citInteger|csmGetbyVal, sizeof(int), &iCapability, NULL},
        {NULL, 0, 0, NULL, NULL}
    };

    sReturn = XMLSParser_ReadAll(REMOTEFOCUSD_CAPABILITY_FILE, acCfgParseMap);
    if (sReturn != S_OK)
    {
		REMOTEFOCUSD_DBGPRINT("XML parser failed, index = %d.\n", sReturn);
        return S_FAIL;
    }


	switch (iCapability)
	{
		case esupNoSupport:
			ptInfo->iIsSupportZoom = 0;
			ptInfo->iIsSupportFocus = 0;
			break;

		case esupZoomFocus:
			ptInfo->iIsSupportZoom = 1;
			ptInfo->iIsSupportFocus = 1;
			break;

		case esupZoom:
			ptInfo->iIsSupportZoom = 1;
			ptInfo->iIsSupportFocus = 0;
			break;

		case esupFocus:
			ptInfo->iIsSupportZoom = 0;
			ptInfo->iIsSupportFocus = 1;
			break;
		default:
			ptInfo->iIsSupportZoom = 0;
			ptInfo->iIsSupportFocus = 0;
			break;
	}

    printf("******RemoteFocusd Daemon Configuration******\n");
    printf("IsSupportZoom = %d\n", ptInfo->iIsSupportZoom);
    printf("IsSupportFocus = %d\n", ptInfo->iIsSupportFocus);
    printf("******RemoteFocusd Daemon Configuration******\n");

    return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_LoadConfigFile(TRemoteFocusdInfo *pThis)
{
	FILE *pfConfig;
	char acParam[128];
	char *acValue;


	if ((pfConfig = fopen(REMOTEFOCUSD_CONFIG_FILE, "r")) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("fopen failed!\n");
		return S_FAIL;
	}

	while (fscanf(pfConfig, "%s", acParam) != EOF)
	{
		acValue = strchr(acParam, '=');
		*(acValue ++) = '\0';
		
		if ((pThis->iIsSupportZoom) && strncmp(acParam, "zoom_motor", 10) == 0)
		{
			pThis->dwZoomMotorPosition = atoi(acValue);
		}
		else if ((pThis->iIsSupportFocus) && strncmp(acParam, "focus_motor", 11) == 0)
		{
			pThis->dwFocusMotorPosition = atoi(acValue);
		}
	}

	fclose(pfConfig);
	return S_OK;
} // End of RemoteFocusd_LoadConfigFile

/* ==================================================================== */
#if defined(_FD8173) || defined(_FD8373) || defined(_IB8373)
SCODE RemoteFocusd_CheckProductType(TRemoteFocusdInfo *pThis)
{
	FILE *pfConfig;
	char acParam[128];
	char *acValue;


	if ((pfConfig = fopen(REMOTEFOCUSD_ETC_FWVER, "r")) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("fopen failed!\n");
		return S_FAIL;
	}

	while (fscanf(pfConfig, "%s", acParam) != EOF)
	{
		acValue = strchr(acParam, '-');
		*(acValue ++) = '\0';
		
		if (strncmp(acParam, "FD8173", 6) == 0 || strncmp(acParam, "FD8373", 6) == 0)
		{
			memcpy(g_dwtabFocusPositionTable, g_dome_dwtabFocusPositionTable, sizeof(g_dwtabFocusPositionTable));
		}
		else if (strncmp(acParam, "IB8373", 6) == 0)
		{
			memcpy(g_dwtabFocusPositionTable, g_bullet_dwtabFocusPositionTable, sizeof(g_dwtabFocusPositionTable));
		}
	}

	fclose(pfConfig);
	return S_OK;
} // End of RemoteFocusd_CheckProductType

/* ==================================================================== */
#endif

void RemoteFocusd_GenPidFile()
{
	FILE *pFile;
	char actemp[8];
	
	if ((pFile = fopen(REMOTEFOCUSD_PID_FILE, "w")) != NULL)
	{
		snprintf(actemp, 8, "%d", getpid());
		fputs(actemp, pFile);
		fclose(pFile);
	}
	
	return;
}

/* ==================================================================== */
void RemoteFocusd_UpdateMaxFd(TRemoteFocusdInfo *pThis, SOCKET sckInput)
{
	if (sckInput > pThis->iMaxFd)
	{
		pThis->iMaxFd = sckInput;
	}
	
	return;
}

/* ==================================================================== */
SCODE RemoteFocusd_InitFdCgi(TRemoteFocusdInfo *pThis)
{
	int iSocketFd;


	if (FDCGI_init(&(pThis->ptFdCgi), REMOTEFOCUSD_FDCGI_SOCKET_PATH) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("FDCGI_init failed.\n");
		return S_FAIL;
	}

	fdcgi *ptFdCgi = pThis->ptFdCgi;

	iSocketFd = FDCGI_GetSocketFd(ptFdCgi);
	pThis->iCGIFd = iSocketFd;
	FD_SET(iSocketFd, &pThis->fds_ReadSet);
	RemoteFocusd_UpdateMaxFd(pThis, iSocketFd);

	return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_InitFIFO(TRemoteFocusdInfo *pThis)
{
	int iFIFOFd;


	if (access(REMOTEFOCUSD_FIFO_PATH, F_OK) == 0)
	{
		remove(REMOTEFOCUSD_FIFO_PATH);
	}

	if (mkfifo(REMOTEFOCUSD_FIFO_PATH, 0777) != 0)
	{
		REMOTEFOCUSD_DBGPRINT("Failed to create FIFO.\n");
		return S_FAIL;
	}

	iFIFOFd = open(REMOTEFOCUSD_FIFO_PATH, O_RDWR);
	if (iFIFOFd == -1)
	{
		REMOTEFOCUSD_DBGPRINT("Failed to open FIFO.\n");
		return S_FAIL;
	}

	pThis->iFIFOFd = iFIFOFd;
	FD_SET(iFIFOFd, &pThis->fds_ReadSet);
	RemoteFocusd_UpdateMaxFd(pThis, iFIFOFd);

	return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_InitShareMem(TRemoteFocusdInfo *pThis)
{
	key_t key;


	// Create shared memory
	key = ftok(REMOTEFOCUSD_SHMEM_FILEPATH, 0);
	if ((pThis->iShMemId = shmget(key, REMOTEFOCUSD_SHMEM_SEGSIZE, IPC_CREAT | 0666)) < 0)
	{
		printf("key is %x, errno is \"%s\"\n", key, strerror(errno));
		REMOTEFOCUSD_DBGPRINT("shmget failed.\n");
		return S_FAIL;
	}
	
	// Attach shared memory
	if ((pThis->ptShareMemInfo =
			(TRemoteFocusd_ShareMemInfo*)shmat(pThis->iShMemId, 0, 0)) == (void*)(-1))
	{
		REMOTEFOCUSD_DBGPRINT("shmat failed.\n");
		return S_FAIL;
	}
	
	return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_InitXMLParser(TRemoteFocusdInfo *pThis)
{
	HANDLE hXMLParser;


	if ((hXMLParser = XML_ParserCreate(NULL)) == NULL)
	{
		REMOTEFOCUSD_DBGPRINT("XML_ParserCreate failed.\n");
		return S_FAIL;
	}
	pThis->hXMLParser = hXMLParser;

	return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_InitFocusMotorThread(TRemoteFocusdInfo *pThis)
{
	TOSThreadInitOptions tOSInitThreadOpt;
	memset(&tOSInitThreadOpt, 0, sizeof(TOSThreadInitOptions));

	// Let kernel manage the stack size
	tOSInitThreadOpt.dwStackSize = 0;
	tOSInitThreadOpt.dwInstance  = (DWORD)(pThis);
	tOSInitThreadOpt.dwPriority  = REMOTEFOCUSD_THREAD_PRIORITY;
	tOSInitThreadOpt.pThreadProc = RemoteFocusd_FocusMotorThread;
	tOSInitThreadOpt.dwFlags = T_TSLICE;

	if (OSThread_Initial(&pThis->hFocusMotorThread, &tOSInitThreadOpt) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("OSThread_Initial failed.\n");
		return S_FAIL;
	}
	if (OSThread_Start(pThis->hFocusMotorThread) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("OSThread_Start failed.\n");
		return S_FAIL;
	}

	return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_InitZoomMotorThread(TRemoteFocusdInfo *pThis)
{
	TOSThreadInitOptions tOSInitThreadOpt;
	memset(&tOSInitThreadOpt, 0, sizeof(TOSThreadInitOptions));

	// Let kernel manage the stack size
	tOSInitThreadOpt.dwStackSize = 0;
	tOSInitThreadOpt.dwInstance  = (DWORD)(pThis);
	tOSInitThreadOpt.dwPriority  = REMOTEFOCUSD_THREAD_PRIORITY;
	tOSInitThreadOpt.pThreadProc = RemoteFocusd_ZoomMotorThread;
	tOSInitThreadOpt.dwFlags = T_TSLICE;

	if (OSThread_Initial(&pThis->hZoomMotorThread, &tOSInitThreadOpt) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("OSThread_Initial failed.\n");
		return S_FAIL;
	}
	if (OSThread_Start(pThis->hZoomMotorThread) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("OSThread_Start failed.\n");
		return S_FAIL;
	}

	return S_OK;
}

/* ==================================================================== */
SCODE RemoteFocusd_Init(TRemoteFocusdInfo *pThis)
{

	pThis->bFocusOperationEnable = FALSE;
	pThis->bZoomOperationEnable = FALSE;
	pThis->eNextFuncType = eftNoService;

	if (RemoteFocusd_LoadCapabilityFile(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_LoadCapabilityFile failed.\n");
		return S_FAIL;
	}

	if (RemoteFocusd_LoadConfigFile(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_LoadConfigFile failed.\n");
		return S_FAIL;
	}

#if defined(_FD8173) || defined(_FD8373) || defined(_IB8373)
	if (RemoteFocusd_CheckProductType(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_CheckProductType failed.\n");
		return S_FAIL;
	}
#endif

	if (RemoteFocusd_InitFdCgi(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitFdCgi failed.\n");
		return S_FAIL;
	}

	if (RemoteFocusd_InitFIFO(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitFIFO failed.\n");
		return S_FAIL;
	}

	if (RemoteFocusd_InitShareMem(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitShareMem failed.\n");
		return S_FAIL;
	}

	if (RemoteFocusd_InitXMLParser(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitXMLParser failed.\n");
		return S_FAIL;
	}

	if ((pThis->iIsSupportFocus) && (RemoteFocusd_InitFocusMotorThread(pThis) != S_OK))
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitFocusMotorThread failed.\n");
		return S_FAIL;
	}

	if ((pThis->iIsSupportZoom) && (RemoteFocusd_InitZoomMotorThread(pThis) != S_OK))
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_InitZoomMotorThread failed.\n");
		return S_FAIL;
	}


	// Initialize semaphore
	sem_init(&pThis->semFileMutex, 0, 1);

	// Log my pid
	RemoteFocusd_GenPidFile();
	pThis->dwMyPid = getpid();
	
	// Create socket pair
	socketpair(AF_UNIX, SOCK_STREAM, 0, pThis->fdFocusMotorWakeup);
	socketpair(AF_UNIX, SOCK_STREAM, 0, pThis->fdZoomMotorWakeup);

#ifdef _TABLE_MAPPING
	// Load Pan Focus table
	if (pThis->iIsSupportFocus)
	{
	RemoteFocusd_LoadMappingTable();
	}

#endif

#ifdef _FOCUS_DEFAULT
	if (pThis->iIsSupportFocus)
	{
		RemoteFocusd_LoadResetDefault();
	}
#endif

	return S_OK;
}// End of RemoteFocusd_Init

/* ==================================================================== */
SCODE RemoteFocusd_Release(TRemoteFocusdInfo *pThis)
{

	REMOTEFOCUSD_DBGPRINT("Release resources...");

	// Detach shared memory
	if ((shmdt((void*)(pThis->ptShareMemInfo))) < 0)
	{
		REMOTEFOCUSD_DBGPRINT("shmdt failed.\n");
		return S_FAIL;
	}

	if ((pThis->iIsSupportFocus) && (OSThread_Release(&(pThis->hFocusMotorThread)) != S_OK))
	{
		REMOTEFOCUSD_DBGPRINT("OSThread_Release failed.\n");
		return S_FAIL;
	}
	
	if ((pThis->iIsSupportZoom) && (OSThread_Release(&(pThis->hZoomMotorThread)) != S_OK))
	{
		REMOTEFOCUSD_DBGPRINT("OSThread_Release failed.\n");
		return S_FAIL;
	}


	XML_ParserFree(pThis->hXMLParser);
	sem_destroy(&(pThis->semFileMutex));
	FDCGI_close(pThis->ptFdCgi);
	close(pThis->iFIFOFd);
	unlink(REMOTEFOCUSD_PID_FILE);

	printf("OK\n");
	return S_OK;
}
