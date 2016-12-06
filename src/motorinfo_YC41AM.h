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

#ifndef REMOTEFOCUSD_ZOOM_VIRTUAL_START
	#define REMOTEFOCUSD_ZOOM_VIRTUAL_START		0
#endif

#ifndef REMOTEFOCUSD_ZOOM_VIRTUAL_END
	#define REMOTEFOCUSD_ZOOM_VIRTUAL_END		700		
#endif

#ifndef REMOTEFOCUSD_FOCUS_VIRTUAL_START
	#define REMOTEFOCUSD_FOCUS_VIRTUAL_START	0
#endif

#ifndef REMOTEFOCUSD_FOCUS_VIRTUAL_END
	#define REMOTEFOCUSD_FOCUS_VIRTUAL_END		1740 		
#endif

#define REMOTEFOCUSD_ZOOM_VIRTUAL_ONESTEP				1
#define REMOTEFOCUSD_FOCUS_VIRTUAL_ONESTEP				1
#define REMOTEFOCUSD_ZOOM_VIRTUAL_TOTALSTEP				700
#define REMOTEFOCUSD_FOCUS_VIRTUAL_TOTALSTEP			1740
#define REMOTEFOCUSD_ZOOM_MOTOR_TOTALSTEP				700  		//766
#define REMOTEFOCUSD_FOCUS_MOTOR_TOTALSTEP				1740		//1849
#define REMOTEFOCUSD_ZOOM_MOTOR_POSITIONINGSTEP			866 		//greater than spec's 766
#define REMOTEFOCUSD_FOCUS_MOTOR_POSITIONINGSTEP		1949 		//greater than spec's 1849
#define REMOTEFOCUSD_FIRST_LEVEL_SEARCH_RANGE			576
#define REMOTEFOCUSD_SECOND_LEVEL_SEARCH_RANGE			48
#define REMOTEFOCUSD_THIRD_LEVEL_SEARCH_RANGE			8
#define REMOTEFOCUSD_FINE_TUNE_SEARCH_STEP				1
#define REMOTEFOCUSD_SMALL_SEARCH_STEP					6
#define REMOTEFOCUSD_LARGE_SEARCH_STEP					24
#define REMOTEFOCUSD_ADJUST_STEP						54

#define REMOTEFOCUSD_FOCUS_ONESTEP_TIME					5172 		//us
#define REMOTEFOCUSD_ZOOM_ONESTEP_TIME					6666 		//us			
#define REMOTEFOCUSD_ONESTEP_SEARCH_TIME				300000 		//us
#define REMOTEFOCUSD_OPEN_IRIS_TIME						5000000 	//us

#define REMOTEFOCUSD_FOCUS_POSITION_INTERVAL			116
#define REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE			15
#define REMOTEFOCUSD_FOCUS_VALUE_TABLE_SIZE				1740

#define REMOTEFOCUSD_ZOOM_POSITION_INTERVAL             46          // 700/15=46

#define REMOTEFOCUSD_SELECT_TIMEOUT						3000000
