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
 * remotefocusd.c
 *
 * \brief
 * C source file of remote focus daemon
 * It includes main function and related procedures.
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

BOOL g_bTerminated = FALSE;
BOOL g_bRunThread = TRUE;
BOOL g_bIrisFullyOpenStatus = TRUE;

//! For module dependency check
volatile static const char rcsid_remotefocusd[]
		= "$Id: "REMOTEFOCUSD_VERSION_STRING", remotefocusd, "REMOTEFOCUSD_UPDATETIME" $";

DWORD g_dwtabFocusValueTable[REMOTEFOCUSD_FOCUS_VALUE_TABLE_SIZE];

#ifdef _TABLE_MAPPING
DWORD g_dwtabFocusPanTable[REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP];
#endif			 

#ifdef _FOCUS_DEFAULT
DWORD g_dwFocusDefault;
#endif

/* ==================================================================== */
void RemoteFocusd_Usage()
{
	printf("Remote Focus daemon\n"
		   "Usage:\n"
		   "remotefocusd [-d][-v][-h]\n"
		   "Options:\n"
		   "-d              Daemon mode\n"
		   "-v              Show current version\n"
		   "-h              This help\n");

	return;
}

/* ==================================================================== */
void RemoteFocusd_ShowVersion()
{
	printf("RemoteFocusd Version: %s\n", REMOTEFOCUSD_VERSION_STRING);
	printf("Last Modify Date:     %s \n", REMOTEFOCUSD_UPDATETIME);
	printf("Copyright (c) 2000-2010 Vivotek Inc. All rights reserved. \n");
	
	return;
}

/* ==================================================================== */
void RemoteFocusd_Config(TRemoteFocusdInfo *pThis, int iArgc, char** pszArgv)
{
	BOOL bIsDaemon = FALSE;
	int iOpt;


	while ((iOpt = getopt(iArgc, pszArgv, "dvh")) != -1)
	{
		switch (iOpt)
		{
			case 'd':
				bIsDaemon = TRUE;
				break;
			case 'v':
				RemoteFocusd_ShowVersion();
				exit(0);
				break;
			case 'h':
			default:
				RemoteFocusd_Usage();
				exit(0);
		}
	}

	if (bIsDaemon)
	{
		REMOTEFOCUSD_DBGPRINT("Entering daemon mode...\n");
		daemon(0,0);
	}

	return;
}

/* ==================================================================== */
void RemoteFocusd_SignalHandler(int iSigNo)
{
	if ( (iSigNo == SIGTERM) || (iSigNo == SIGINT) )
	{
		REMOTEFOCUSD_DBGPRINT("Receive TERM/INT signal!\n");
		g_bTerminated = TRUE;
	}
	else if (iSigNo == SIGPIPE)
	{
		REMOTEFOCUSD_DBGPRINT("Receive SIGPIPE!\n");
	}
	
	return;
}

/* ==================================================================== */
void RemoteFocusd_InitSignal()
{
	struct sigaction tSigAct;

	// For program termination signal
	tSigAct.sa_handler = RemoteFocusd_SignalHandler;
	sigemptyset(&tSigAct.sa_mask);
	tSigAct.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &tSigAct, NULL);
	sigaction(SIGINT, &tSigAct, NULL);
	sigaction(SIGPIPE, &tSigAct, NULL);

	return;
}

/* ==================================================================== */
int main(int iArgc, char** pszArgv)
{
	TRemoteFocusdInfo tRemoteFocusdInfo;
	TRemoteFocusdInfo *pThis = &tRemoteFocusdInfo;


	memset(pThis, 0, sizeof(TRemoteFocusdInfo));

	RemoteFocusd_Config(pThis, iArgc, pszArgv);
	RemoteFocusd_InitSignal();

	if (RemoteFocusd_Init(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_Init failed.\n");
		goto END;
	}

	//Main loop
	RemoteFocusd_InfoProcess(pThis);

END:
	if (RemoteFocusd_Release(pThis) != S_OK)
	{
		REMOTEFOCUSD_DBGPRINT("RemoteFocusd_Release failed.\n");
		return 1;
	}

	return 0;
}
