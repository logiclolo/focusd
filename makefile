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
#  Module name          : mod1_test
#  Module description   : module1 testing program
#  Author               : Joe Wu
#  Created at           : 2006/08/08
#  $History:$
#
#---------------------------------------------------------------------

# environment settings
include $(MAKEINC)/default.mk


include $(CONFIGFILE)
# image sensor type 
include $(MAKEINC)/apps.mk

# Targets
all: $(LIBS) $(CUSTOMLIBS) app
	
# install variables
package_init = init.d-remotefocusd
package_sbin_dir = /usr/sbin
package_init_dir = /etc/init.d
host_package_bin_dir=${ROOTFSDIR}$(package_sbin_dir)
host_package_init_dir=${ROOTFSDIR}${package_init_dir}
INSTALLED_APP := $(host_package_bin_dir)/$(MODNAME).$(EXESUFFIX)
NEW_APP := $(host_package_bin_dir)/remotefocusd

install:
	install src/${package_init} ${host_package_init_dir}/remotefocusd
	install $(APPTARGET) $(host_package_bin_dir)
	$(MV) $(INSTALLED_APP) $(NEW_APP)
	$(STRIP) $(NEW_APP)


