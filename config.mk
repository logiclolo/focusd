#---------------------------------------------------------------------
#  $Header:$
#  Copyright (c) 2000-2007 Vivotek Inc. All rights reserved.
#
#  +-----------------------------------------------------------------+
#  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
#  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
#  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
#  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
#  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
#  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
#  |                                                                 |
#  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
#  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
#  | VIVOTEK INC.                                                    |
#  +-----------------------------------------------------------------+
#
#  Project name         : NONE
#  Module name          : config.mk
#  Module description   : configuration for makefile inclusion
#  Author               : Joe Wu
#  Created at           : 2006/08/02
#  $History:$
#
#---------------------------------------------------------------------

#---------------------------------------------------------------------
# Configurations
#---------------------------------------------------------------------

# general configuration, for each configuration
#---------------------------------------------------------------------
PLATFORM 	= _LINUX
INCDIR	 	= 
SYSDEFS  	= $(remotefocusd_sysdef)

CFLAGS	 	= 
USRDEFS  	= $(remotefocusd_usrdef) 

LINKFLAGS	= -lpthread
#TOOL_CONFIG = GenericLinux
#TOOL_CONFIG = ARMLinux
BUILD_CONF	= armlinux_release
LIBS 		= $(common_lib) $(osisolate_lib) $(expat_lib) $(xmlsparser_lib) $(message_lib) $(fdipc_lib) $(fdcgi_lib)

ifneq "" "$(filter IP8372 FD8372 FD8372E, $(PRODUCTNAME))"
SYSDEFS += _YC41AM
endif # end of FD8372

ifneq "" "$(filter IP8362 FD8362 FD8162, $(PRODUCTNAME))"
SYSDEFS += _DF010NA0000
endif # end of FD8362

ifneq "" "$(filter FD8173H FD8373HV IB8373H IP8173H, $(PRODUCTNAME))"
SYSDEFS += _TT02812PB
USRDEFS += _BLOCKING_MODE 
endif # end of FD8373

ifneq "" "$(filter FD8365 FD8355, $(PRODUCTNAME))"
SYSDEFS += _MSVF33X0313IR
USRDEFS += _BLOCKING_MODE _FOCUS_CALIBRATION_REVERS
endif

ifneq "" "$(filter IP8365 IP8355, $(PRODUCTNAME))"
SYSDEFS += _ASP3Z0312LMRP
USRDEFS += _BLOCKING_MODE _FOCUS_CALIBRATION_REVERS
endif

ifneq "" "$(filter IP8165 IP8155, $(PRODUCTNAME))"
SYSDEFS += _ABFMOTOR
USRDEFS += _BLOCKING_MODE _FOCUS_CALIBRATION_REVERS _FOCUS_MOTOR_ONLY
endif

# specific configuration
#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "linux_release"
CFLAGS	 	= -O3 -Wall
endif
#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "linux_debug"
CFLAGS		= -O0 -Wall
endif

#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "armlinux_release"
#CFLAGS	 	= -O3 -D_NDEBUG -Wall
CFLAGS		= -O0 -Wall -g 
TOOL_CONFIG = ARMLinux
endif

#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "vcwin32_release"
PLATFORM 	= _WIN32_
TOOL_CONFIG = VCWin32
LIBS 		= common
CFLAGS	 	= /O2 -D_NDEBUG
endif

#---------------------------------------------------------------------
# tool settings
#---------------------------------------------------------------------

include $(MAKEINC)/tools.mk
ifeq "$(TOOL_CONFIG)" "CONFIG_NAME"
# you can add your additional tools configuration here
endif


