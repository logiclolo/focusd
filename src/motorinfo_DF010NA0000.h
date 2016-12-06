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
 * motorinfo_DF010NA0000.h
 *
 * \brief
 * C header file of motor information
 *
 * \date
 * 2012/06/11
 *
 * \author
 * Jiun Cheng Cheng
 *
 ***********************************************************************
 */
/* =========================== Header Files =========================== */

#ifndef REMOTEFOCUSD_ZOOM_VIRTUAL_START
	#define REMOTEFOCUSD_ZOOM_VIRTUAL_START		0
#endif

#ifndef REMOTEFOCUSD_ZOOM_VIRTUAL_END
	#define REMOTEFOCUSD_ZOOM_VIRTUAL_END		3750
#endif

#ifndef REMOTEFOCUSD_FOCUS_VIRTUAL_START
	#define REMOTEFOCUSD_FOCUS_VIRTUAL_START	0
#endif

#ifndef REMOTEFOCUSD_FOCUS_VIRTUAL_END
  #ifdef _IP8362
    #define REMOTEFOCUSD_FOCUS_VIRTUAL_END		5850//5700
  #endif
  #if (defined _FD8362 || _FD8362E || _FD8363)
    #define REMOTEFOCUSD_FOCUS_VIRTUAL_END	  6000//5700
  #endif
#endif

#define USLEEP_ONE_SEC 100000
#ifdef _IP8362
  #define REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP				1 // 3750
#endif
#if (defined _FD8362 || _FD8362E || _FD8363)
  #define REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP				1 //17 // 2550
#endif
#define REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP				1
#define REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP		    REMOTEFOCUSD_ZOOM_VIRTUAL_END
#define REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP		REMOTEFOCUSD_FOCUS_VIRTUAL_END  // 廠商標準 5850 ±  150
#define REMOTEFOCUSD_ZOOM_MOTOR_TOTALSTEP	        REMOTEFOCUSD_ZOOM_VIRTUAL_END// 廠商標準 :  3850 ±  150
#define REMOTEFOCUSD_FOCUS_MOTOR_TOTALSTEP			REMOTEFOCUSD_FOCUS_VIRTUAL_END
#define REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP			4434 		//greater than spec's 4334
#define REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP		6100 		//greater than spec's 6100
#define REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE			1920
#define REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE		160
#define REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE			80
#define REMOTEFOCUSD_FINE_TUNE_SEARCH_STEP				1
#define REMOTEFOCUSD_SMALL_SEARCH_STEP					60
#define REMOTEFOCUSD_LARGE_SEARCH_STEP					80
#define REMOTEFOCUSD_ADJUST_STEP						100
#define REMOTEFOCUSD_COMPENSATION_SEARCH_RANGE       600
#define REMOTEFOCUSD_COMPENSATION_SEARCH_STEP				 60
#define REMOTEFOCUSD_RFS_INIT_RANGE					128 //  should more than REMOTEFOCUSD_COMPENSATION_SEARCH_STEP*2
#define REMOTEFOCUSD_RFS_INIT_STEP          16  // REMOTEFOCUSD_RFS_INIT_RANGE / 8
#define REMOTEFOCUSD_RFS_RANGE_RATIO				2
#define REMOTEFOCUSD_RFS_STEP_RATIO					8
#define REMOTEFOCUSD_RFS_COUNTER_LIMIT      16
#define REMOTEFOCUSD_FOCUS_SCAN_INIT_STEP   100
#define REMOTEFOCUSD_FOCUS_SCAN_TERMINATION_COND   1


#define REMOTEFOCUSD_FOCUS_ONESTEP_TIME					750 		//us
#define REMOTEFOCUSD_ZOOM_ONESTEP_TIME					1000  		//us
#define REMOTEFOCUSD_ONESTEP_SEARCH_TIME				USLEEP_ONE_SEC*2 		//us
#define REMOTEFOCUSD_OPEN_IRIS_TIME						  USLEEP_ONE_SEC*4/10		//us

#define REMOTEFOCUSD_FOCUS_POSITION_INTERVAL			400 // 6000/15=400
#define REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE			15
#define REMOTEFOCUSD_FOCUS_POSITION_START_TABLE_SIZE	16
#define REMOTEFOCUSD_FOCUS_VALUE_TABLE_SIZE				REMOTEFOCUSD_FOCUS_VIRTUAL_END

#define REMOTEFOCUSD_ZOOM_POSITION_INTERVAL             250          // 3750/15=250

#define REMOTEFOCUSD_SELECT_TIMEOUT						3000000
