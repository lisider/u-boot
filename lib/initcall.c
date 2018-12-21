// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 */

#include <common.h>
#include <initcall.h>
#include <efi.h>
#include <suws_debug.h>

DECLARE_GLOBAL_DATA_PTR;

int initcall_run_list(const init_fnc_t init_sequence[])
{
	const init_fnc_t *init_fnc_ptr;
	SUWS_PRINT("suws_u-boot bringup initcall_run_list +++ %s,%s,%d\n",__FILE__,__func__,__LINE__);
	int num = 0;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		num ++;
		SUWS_PRINT("suws_u-boot bringup init_func %d +++ %s,%s,%d\n",num,__FILE__,__func__,__LINE__);
		unsigned long reloc_ofs = 0;
		int ret;

		if (gd->flags & GD_FLG_RELOC)
			reloc_ofs = gd->reloc_off;
#ifdef CONFIG_EFI_APP
		reloc_ofs = (unsigned long)image_base;
#endif
		debug("initcall: %p", (char *)*init_fnc_ptr - reloc_ofs);
		if (gd->flags & GD_FLG_RELOC)
			debug(" (relocated to %p)\n", (char *)*init_fnc_ptr);
		else
			debug("\n");
		ret = (*init_fnc_ptr)();
		if (ret) {
			printf("initcall sequence %p failed at call %p (err=%d)\n",
			       init_sequence,
			       (char *)*init_fnc_ptr - reloc_ofs, ret);
			return -1;
		}
		SUWS_PRINT("suws_u-boot bringup init_func %d --- %s,%s,%d\n",num,__FILE__,__func__,__LINE__);
	}
	SUWS_PRINT("suws_u-boot bringup initcall_run_list --- %s,%s,%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
