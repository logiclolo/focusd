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
#include "remotefocusd.h"

#ifdef _FOCUSING_START
	#ifdef _FD8372
		const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
					{ 386, 608, 830, 1022, 1160,
					 1268, 1334, 1382, 1424, 1448,
					 1466, 1460, 1454, 1436, 1412};
					 
	#elif defined(_IP8372)
		const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
					{ 626, 788, 1010, 1172, 1250,
					  1334, 1370, 1430, 1460, 1466,
					  1466, 1442, 1454, 1418, 1382};
					  
	#elif defined(_FD8173) || defined(_FD8373) || defined(_IB8373)
		// virtual step is 1 
		/*const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =*/
					/*{   0,    0,    0,   76,  498,*/
					  /*937, 1255, 1491, 1695, 1862,*/
					 /*2044, 2113, 2186, 2244, 2341};*/
					 
		// virtual step is 4 
		/*const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =*/
					/*{   0,   0,   0,  19, 124,*/
					  /*234, 314, 357, 424, 465,*/
					  /*511, 528, 546, 561, 585};*/

		const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
					{ 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0};

	#elif (defined _FD8365 || defined _FD8355)
		const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
					{    0,    0,  340,  570,  810,
					   970, 1110, 1220, 1295, 1355,
					  1400, 1430, 1450, 1460, 1460,
					  1455};

	#elif (defined _IP8365 || defined _IP8355)
		const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
					{    0,    0, 1347, 2350, 2911,
					  3550, 3815, 4160, 4485, 4553,
					  4710, 4514, 4553, 4494, 4474};
	
	#elif (defined _IP8165 || defined _IP8155)
		const DWORD g_dwtabFocusingStartTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
					{ 0, 0, 0, 0, 0,
					  0, 0, 0, 0, 0,
					  0, 0, 0, 0, 0,
					  0};
	#endif		
	
#endif


#ifndef _TABLE_MAPPING
	#ifdef _FD8372
	const DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{ 906, 1068, 1224, 1362, 1440,
				  1530, 1572, 1608, 1614, 1626,
				  1626, 1620, 1602, 1578, 1584};
				 
	#elif defined(_IP8372)
	const DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{ 1032, 1212, 1338, 1434, 1512,
				  1566, 1602, 1626, 1638, 1644,
				  1638, 1626, 1608, 1584, 1578};
				  
	#elif (defined _FD8362 || _IP8362)
	const DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] = 
				{ 59,  98, 135, 168, 194,
				 218, 236, 251, 263, 272,
				 279, 284, 288, 290, 296};
				 
	#elif defined(_FD8173) || defined(_FD8373) || defined(_IB8373)

	/************************** 
	 * virtual step is 4 
	 **************************/
	// FD8173, FD8373
	const DWORD g_dome_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{  22, 150, 293, 419, 495,
				  563, 622, 672, 705, 736,
				  754, 776, 791, 799, 807};
	// IB8373
	const DWORD g_bullet_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{  54, 167, 306, 409, 492,
				  561, 617, 665, 702, 731,
				  756, 776, 792, 801, 811};

	#elif (defined _FD8365 || defined _FD8355)
	const DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{   95,  400,  730,  966, 1159,
				  1307, 1423, 1525, 1579, 1634,
				  1675, 1703, 1721, 1730, 1728,
				  1725};

	#elif (defined _IP8365 || defined _IP8355)
	const DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{ 560, 1460, 2520, 3320, 3940,
				 4420, 4760, 5042, 5230, 5398,
				 5514, 5552, 5600, 5586, 5568};
	

	#elif (defined _IP8165 || defined _IP8155)
	const DWORD g_dwtabFocusPositionTable[REMOTEFOCUSD_FOCUS_POSITION_TABLE_SIZE] =
				{   95,  400,  730,  966, 1159,
				  1307, 1423, 1525, 1579, 1634,
				  1675, 1703, 1721, 1730, 1728,
				  1725};
	#endif
	
#endif
	

