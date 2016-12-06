/*
*******************************************************************************
* $Header: $
*
*  Copyright (c) 2000-2010 VIVOTEK Inc. All rights reserved.
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
* $History: $
*/

/*!
*******************************************************************************
* Copyright 2000-2010 VIVOTEK, Inc. All rights reserved.
*
* \file
* focusmotor.h
*
* \brief
* Header file of focus motor control driver
*
* \date
* 2010/12/21
*
* \author
* James Ye (reference piris driver written by Evan)
*
*******************************************************************************
*/

/* ============================================================================= */
#ifndef _FOCUSMOTOR_H_
#define _FOCUSMOTOR_H_

#include "typedef.h"

/* The drive method of Motor device */
typedef enum {
	mdmGPIO = 1,		// GPIO Mode
	mdmIOEXPAND = 2		// I2C Expander Mode
}EMotorDriveMethod;

/* user 'M' as magic number M=0x4D */
#define MOTOR_MAGIC				'M'

/* Set the motor forward direction */
#define MOTOR_SET_FORWARD		_IO (MOTOR_MAGIC, 0)
/* Set the motor backward direction */
#define MOTOR_SET_BACKWARD		_IO (MOTOR_MAGIC, 1)

/* Set the speed value of motor */
#define MOTOR_SET_SPEED			_IOW (MOTOR_MAGIC, 2, DWORD)
/* Set the walk steps value of motor */
#define MOTOR_WALK_STEPS		_IOW (MOTOR_MAGIC, 3, DWORD)
/* Clear the current walk steps value of motor */
#define MOTOR_WALK_STEPS_CLR	_IO (MOTOR_MAGIC, 4)

/* Turn off motors for thermal issue */
#define MOTOR_TURN_OFF			_IO (MOTOR_MAGIC, 5)

/* Set the walk steps value of motor with blocking mode*/
#define MOTOR_WALK_STEPS_BLOCK _IOW (MOTOR_MAGIC, 6, DWORD)

/* Set the debug message level of driver status */
#define MOTOR_SET_MSG_LEVEL		_IOW (MOTOR_MAGIC, 11, DWORD)
/* List the current system log of driver status */
#define MOTOR_SYS_LOG_LIST		_IOW (MOTOR_MAGIC, 12, DWORD)

/* The max number of ioctl's */
#define MOTOR_MAXNR 13

#endif // _FOCUSMOTOR_H_
