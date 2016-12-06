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
 * motorinfo_YC41AM.h
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
#define REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP				1
#define REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP				4	

#ifndef REMOTEFOCUSD_ZOOM_VIRTUAL_START
	#define REMOTEFOCUSD_ZOOM_VIRTUAL_START		0	
#endif

#ifndef REMOTEFOCUSD_ZOOM_VIRTUAL_END
	#define REMOTEFOCUSD_ZOOM_VIRTUAL_END		1300	// smaller than spec's 1352.
#endif

#ifndef REMOTEFOCUSD_FOCUS_VIRTUAL_START
	#define REMOTEFOCUSD_FOCUS_VIRTUAL_START	0
#endif

#ifndef REMOTEFOCUSD_FOCUS_VIRTUAL_END
	#define REMOTEFOCUSD_FOCUS_VIRTUAL_END		3300 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP	// smaller than sepc's 3318. (4 phases for one step, total is 825 steps)
#endif

#define REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP				1300	
#define REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP			3300 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP
#define REMOTEFOCUSD_ZOOM_MOTOR_TOTALSTEP				1300										//1352
#define REMOTEFOCUSD_FOCUS_MOTOR_TOTALSTEP				3300 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP		//3318
#define REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP			1400 										//greater than spec's 1352, for calibration
#define REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP		3420 										//greater than spec's 3318, for calibration
#define REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE			576 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP	//144
#define REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE			48 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP		//12
#define REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE			32 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP		//8
#define REMOTEFOCUSD_FINE_TUNE_SEARCH_STEP				4 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP      //1	
#define REMOTEFOCUSD_SMALL_SEARCH_STEP					8 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP		//2	
#define REMOTEFOCUSD_LARGE_SEARCH_STEP					24 / REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP		//6
#define REMOTEFOCUSD_ADJUST_STEP						54
#define REMOTEFOCUSD_ZOOM_ADJUST_STEP					40	
#define REMOTEFOCUSD_FOCUS_ADJUST_STEP					60/REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP

#define USLEEP_ONE_SEC 100000
#define REMOTEFOCUSD_FOCUS_ONESTEP_TIME					5172 					//us
#define REMOTEFOCUSD_ZOOM_ONESTEP_TIME					6666 					//us			
#define REMOTEFOCUSD_ONESTEP_SEARCH_TIME				6*USLEEP_ONE_SEC 		//us
#define REMOTEFOCUSD_OPEN_IRIS_TIME						5000000 				//us

#define REMOTEFOCUSD_FOCUS_POSITION_INTERVAL			116	
//#define REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE			30	
#define REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE			15	
#define REMOTEFOCUSD_FOCUS_VALUE_TABLE_SIZE				1740

//#define REMOTEFOCUSD_ZOOM_POSITION_INTERVAL             43          // 1300/30=43
#define REMOTEFOCUSD_ZOOM_POSITION_INTERVAL             86          // 1300/15=86

#define REMOTEFOCUSD_SELECT_TIMEOUT						3000000

