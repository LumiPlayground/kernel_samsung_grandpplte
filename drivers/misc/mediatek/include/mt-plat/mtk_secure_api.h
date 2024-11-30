/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


#ifndef _MTK_SECURE_API_H_
#define _MTK_SECURE_API_H_

#include <linux/kernel.h>

#ifdef CONFIG_ARM64
#define MTK_SIP_SMC_AARCH_BIT			0x40000000
#else
#define MTK_SIP_SMC_AARCH_BIT			0x00000000
#endif

/* TRNG */
#define MTK_SIP_KERNEL_GET_RND (0x8200026A | MTK_SIP_SMC_AARCH_BIT)

#endif /* _MTK_SECURE_API_H_ */