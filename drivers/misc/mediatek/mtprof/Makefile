#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ccflags-y += -Idrivers/misc/mediatek/aee/aed/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)
LINUXINCLUDE += -include $(srctree)/kernel/sched/sched.h

obj-y := mtprof.o bootprof.o
obj-y += sched_monitor.o monitor_debug_out.o
# obj-$(CONFIG_MT_LOCK_DEBUG) += lockprof.o
obj-$(CONFIG_MTK_WQ_DEBUG) += mt_wq_debug.o
mtprof-y += prof_ctl.o prof_main.o common.o
# obj-y += mt_prv_lock.o
# obj-$(CONFIG_MT_PRINTK_UART_CONSOLE) += mt_printk_ctrl.o
obj-$(CONFIG_MT_RT_THROTTLE_MON) += rt_monitor.o
