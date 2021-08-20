/*
* wdi-simple.c: Console Driver Installer for a single USB device
* Copyright (c) 2010-2018 Pete Batard <pete@akeo.ie>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 3 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libwdi.h"

#if defined(_PREFAST_)
/* Disable "Banned API Usage:" errors when using WDK's OACR/Prefast */
#pragma warning(disable:28719)
/* Disable "Consider using 'GetTickCount64' instead of 'GetTickCount'" when using WDK's OACR/Prefast */
#pragma warning(disable:28159)
#endif

#define INF_NAME    "libusbk.inf"

int __cdecl main(int argc, char** argv)
{
	static struct wdi_device_info *ldev, dev = {NULL, 0, 0, FALSE, 0, NULL, NULL, NULL, NULL};
	static struct wdi_options_create_list ocl = { .list_all = TRUE, .list_hubs = TRUE, .trim_whitespaces = TRUE };
	static struct wdi_options_prepare_driver opd = { WDI_LIBUSBK };
	static struct wdi_options_install_driver oid = { 0 };
	static int log_level = WDI_LOG_LEVEL_INFO;
	int r;
	if (argc < 5) {
		printf("Not all required arguments were passed. Please invoke with VID, PID, Description and temporary directory");
		return -1;
	}

	char *ext_dir = argv[4];

	dev.vid = (unsigned short)strtol(argv[1], NULL, 0);
	dev.pid = (unsigned short)strtol(argv[2], NULL, 0);
	dev.desc = argv[3];

	wdi_set_log_level(log_level);

	printf("Extracting driver files...\n");
	r = wdi_prepare_driver(&dev, ext_dir, INF_NAME, &opd);
	printf("  %s\n", wdi_strerror(r));
	if (r != WDI_SUCCESS)
		return r;

	printf("Installing driver(s)...\n");

	// Try to match against a plugged device to avoid device manager prompts
	if (wdi_create_list(&ldev, &ocl) == WDI_SUCCESS) {
		r = WDI_SUCCESS;
		for (; (ldev != NULL) && (r == WDI_SUCCESS); ldev = ldev->next) {
			if ( (ldev->vid == dev.vid) && (ldev->pid == dev.pid) && (ldev->mi == dev.mi) &&(ldev->is_composite == dev.is_composite) ) {
				
				dev.hardware_id = ldev->hardware_id;
				dev.device_id = ldev->device_id;
				printf("  %s: ", dev.hardware_id);
				fflush(stdout);
				r = wdi_install_driver(&dev, ext_dir, INF_NAME, &oid);
				printf("%s\n", wdi_strerror(r));
			}
		}
	}

	return r;
}
