/*
Copyright (c) 2026 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/


#ifndef LIBUSEFUL_LINUX_CAPABILITIES
#define LIBUSEFUL_LINUX_CAPABILITIES

#include "includes.h"


/*
This module relates to the Linux Capabilities system. You would not normally call the below functions
directly, but would instead call ProcessApplyConfig("caps=cap_net_admin,cap_bind_service")
*/


#define LU_CAPABILITIES_UID  1
#define LU_CAPABILITIES_KEEP 2

int ProcessSetCapabilities(const char *Capabilties, const char *IneritCapabilites, int Flags);

#endif
