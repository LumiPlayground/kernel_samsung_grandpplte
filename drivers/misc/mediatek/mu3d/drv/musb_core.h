/*
 * MUSB OTG driver defines
 *
 * Copyright 2005 Mentor Graphics Corporation
 * Copyright (C) 2005-2006 by Texas Instruments
 * Copyright (C) 2006-2007 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __MUSB_CORE_H__
#define __MUSB_CORE_H__

#include <linux/slab.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb.h>
#include <linux/usb/otg.h>
#include <linux/usb/musb.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
/*#include <mt-plat/battery_common.h>*/
#ifndef CONFIG_MTK_FPGA
#include <mt-plat/charging.h>
#endif

struct musb;
struct musb_hw_ep;
struct musb_ep;

#ifdef CONFIG_MTK_KERNEL_POWER_OFF_CHARGING
#include <mt-plat/mt_boot_common.h>
#endif
extern u32 fake_CDP;

extern struct musb *_mu3d_musb;
#if defined(CONFIG_MTK_SMART_BATTERY)
extern void BATTERY_SetUSBState(int usb_state_value);
extern CHARGER_TYPE mt_get_charger_type(void);
#endif
/* Helper defines for struct musb->hwvers */
#define MUSB_HWVERS_MAJOR(x)	((x >> 10) & 0x1f)
#define MUSB_HWVERS_MINOR(x)	(x & 0x3ff)
#define MUSB_HWVERS_RC		0x8000
#define MUSB_HWVERS_1300	0x52C
#define MUSB_HWVERS_1400	0x590
#define MUSB_HWVERS_1800	0x720
#define MUSB_HWVERS_1900	0x784
#define MUSB_HWVERS_2000	0x800

#include "musb_debug.h"
#include "musb_dma.h"

#include "musb_io.h"
#include "musb_regs.h"		/* We don't want to use original musb registers any more. */

#include "musb_gadget.h"
#include <linux/usb/hcd.h>

#define USB_GADGET_SUPERSPEED
#define EP_PROFILING

#define MUSB_DRIVER_NAME "musb-hdrc"

#define	is_peripheral_enabled(musb)	((musb)->board_mode != MUSB_HOST)
#define	is_host_enabled(musb)		((musb)->board_mode != MUSB_PERIPHERAL)
#define	is_otg_enabled(musb)		((musb)->board_mode == MUSB_OTG)

/* NOTE:  otg and peripheral-only state machines start at B_IDLE.
 * OTG or host-only go to A_IDLE when ID is sensed.
 */
#define is_peripheral_active(m)		(!(m)->is_host)
#define is_host_active(m)		((m)->is_host)

#ifndef CONFIG_HAVE_CLK
/* Dummy stub for clk framework */
#define clk_get(dev, id)	NULL
#define clk_put(clock)		do {} while (0)
#define clk_enable(clock)	do {} while (0)
#define clk_disable(clock)	do {} while (0)
#endif

#ifdef CONFIG_PROC_FS
#include <linux/fs.h>
#define MUSB_CONFIG_PROC_FS
#endif

static inline struct usb_hcd *musb_to_hcd(struct musb *musb)
{
	return container_of((void *)musb, struct usb_hcd, hcd_priv);
}

static inline struct musb *hcd_to_musb(struct usb_hcd *hcd)
{
	return (struct musb *)(hcd->hcd_priv);
}

/****************************** PERIPHERAL ROLE *****************************/

#define	is_peripheral_capable()	(1)

extern irqreturn_t musb_g_ep0_irq(struct musb *);
extern void musb_g_tx(struct musb *, u8);
extern void musb_g_rx(struct musb *, u8);
extern void musb_g_reset(struct musb *);
extern void musb_g_suspend(struct musb *);
extern void musb_g_resume(struct musb *);
extern void musb_g_wakeup(struct musb *);
extern void musb_g_disconnect(struct musb *);

/****************************** HOST ROLE ***********************************/

#define	is_host_capable()	(1)

extern irqreturn_t musb_h_ep0_irq(struct musb *);
extern void musb_host_tx(struct musb *, u8);
extern void musb_host_rx(struct musb *, u8);

/****************************** CONSTANTS ********************************/

#ifndef MUSB_C_NUM_EPS
#define MUSB_C_NUM_EPS ((u8)9)
#endif

#ifndef MUSB_MAX_END0_PACKET
#define MUSB_MAX_END0_PACKET ((u16)MUSB_EP0_FIFOSIZE)
#endif

/* USB working mode */
typedef enum {
	CABLE_MODE_CHRG_ONLY = 0,
	CABLE_MODE_NORMAL,
	CABLE_MODE_HOST_ONLY,
	CABLE_MODE_MAX
} CABLE_MODE;

typedef enum {
	USB_SUSPEND = 0,
	USB_UNCONFIGURED,
	USB_CONFIGURED
} usb_state_enum;

/* host side ep0 states */
enum musb_h_ep0_state {
	MUSB_EP0_IDLE,
	MUSB_EP0_START,		/* expect ack of setup */
	MUSB_EP0_IN,		/* expect IN DATA */
	MUSB_EP0_OUT,		/* expect ack of OUT DATA */
	MUSB_EP0_STATUS,	/* expect ack of STATUS */
} __packed;

/* peripheral side ep0 states */
enum musb_g_ep0_state {
	MUSB_EP0_STAGE_IDLE,	/* idle, waiting for SETUP */
	MUSB_EP0_STAGE_SETUP,	/* received SETUP */
	MUSB_EP0_STAGE_TX,	/* IN data */
	MUSB_EP0_STAGE_RX,	/* OUT data */
	MUSB_EP0_STAGE_STATUSIN,	/* (after OUT data) */
	MUSB_EP0_STAGE_STATUSOUT,	/* (after IN data) */
	MUSB_EP0_STAGE_ACKWAIT,	/* after zlp, before statusin */
} __packed;

/*
 * OTG protocol constants.  See USB OTG 1.3 spec,
 * sections 5.5 "Device Timings" and 6.6.5 "Timers".
 */
#define OTG_TIME_A_WAIT_VRISE	100	/* msec (max) */
#define OTG_TIME_A_WAIT_BCON	1100	/* min 1 second */
#define OTG_TIME_A_AIDL_BDIS	200	/* min 200 msec */
#define OTG_TIME_B_ASE0_BRST	100	/* min 3.125 ms */


/*************************** REGISTER ACCESS ********************************/

/* Endpoint registers (other than dynfifo setup) can be accessed either
 * directly with the "flat" model, or after setting up an index register.
 */

#if defined(CONFIG_ARCH_DAVINCI) || defined(CONFIG_SOC_OMAP2430) \
		|| defined(CONFIG_SOC_OMAP3430) || defined(CONFIG_BLACKFIN) \
		|| defined(CONFIG_ARCH_OMAP4)
/* REVISIT indexed access seemed to
 * misbehave (on DaVinci) for at least peripheral IN ...
 */
#define	MUSB_FLAT_REG
#endif

/* TUSB mapping: "flat" plus ep0 special cases */
#if	defined(CONFIG_USB_TUSB6010)
#define musb_ep_select(_mbase, _epnum) \
	musb_writeb((_mbase), MUSB_INDEX, (_epnum))
#define	MUSB_EP_OFFSET			MUSB_TUSB_OFFSET

/* "flat" mapping: each endpoint has its own i/o address */
#elif	defined(MUSB_FLAT_REG)
#define musb_ep_select(_mbase, _epnum)	(((void)(_mbase)), ((void)(_epnum)))
#define	MUSB_EP_OFFSET			MUSB_FLAT_OFFSET

/* "indexed" mapping: INDEX register controls register bank select */
#else
#define musb_ep_select(_mbase, _epnum) \
	musb_writeb((_mbase), MUSB_INDEX, (_epnum))
#define	MUSB_EP_OFFSET			MUSB_INDEXED_OFFSET
#endif




#define	SSUSB_EP_TXCR0_OFFSET(_epnum, _offset)	\
	(U3D_TX1CSR0 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_TXCR1_OFFSET(_epnum, _offset)	\
	(U3D_TX1CSR1 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_TXCR2_OFFSET(_epnum, _offset)	\
	(U3D_TX1CSR2 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_TXMAXP_OFFSET(_epnum, _offset)	\
	(U3D_TX1CSR0 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_RXCR0_OFFSET(_epnum, _offset)	\
	(U3D_RX1CSR0 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_RXCR1_OFFSET(_epnum, _offset)	\
	(U3D_RX1CSR1 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_RXCR2_OFFSET(_epnum, _offset)	\
	(U3D_RX1CSR2 + ((_epnum - 1)*0x10) + (_offset))

#define	SSUSB_EP_RXCR3_OFFSET(_epnum, _offset)	\
	(U3D_RX1CSR3 + ((_epnum - 1)*0x10) + (_offset))



/****************************** FUNCTIONS ********************************/

#define MUSB_HST_MODE(_musb)\
	{ (_musb)->is_host = true; }
#define MUSB_DEV_MODE(_musb) \
	{ (_musb)->is_host = false; }

#define test_devctl_hst_mode(_x) \
	(musb_readb((_x)->mregs, MUSB_DEVCTL)&MUSB_DEVCTL_HM)

#define MUSB_MODE(musb) ((musb)->is_host ? "Host" : "Peripheral")

/******************************** TYPES *************************************/

/**
 * struct musb_platform_ops - Operations passed to musb_core by HW glue layer
 * @init:	turns on clocks, sets up platform-specific registers, etc
 * @exit:	undoes @init
 * @set_mode:	forcefully changes operating mode
 * @try_ilde:	tries to idle the IP
 * @vbus_status: returns vbus status if possible
 * @set_vbus:	forces vbus status
 * @adjust_channel_params: pre check for standard dma channel_program func
 */
struct musb_platform_ops {
	int (*init)(struct musb *musb);
	int (*exit)(struct musb *musb);

	void (*enable)(struct musb *musb);
	void (*disable)(struct musb *musb);

	int (*set_mode)(struct musb *musb, u8 mode);
	void (*try_idle)(struct musb *musb, unsigned long timeout);

	int (*vbus_status)(struct musb *musb);
	void (*set_vbus)(struct musb *musb, int on);

	int (*adjust_channel_params)(struct dma_channel *channel,
				      u16 packet_sz, u8 *mode, dma_addr_t *dma_addr, u32 *len);
};

/*
 * struct musb_hw_ep - endpoint hardware (bidirectional)
 *
 * Ordered slightly for better cacheline locality.
 */
struct musb_hw_ep {
	struct musb *musb;
	void __iomem *fifo;
	void __iomem *regs;

	/* For ssusb+ */
	void __iomem *addr_txcsr0;
	void __iomem *addr_txcsr1;
	void __iomem *addr_txcsr2;
	/* void __iomem          *addr_txmaxpktsz; */

	void __iomem *addr_rxcsr0;
	void __iomem *addr_rxcsr1;
	void __iomem *addr_rxcsr2;
	void __iomem *addr_rxcsr3;
	void __iomem *addr_rxmaxpktsz;
	/* For ssusb- */

#if defined(CONFIG_USB_MUSB_TUSB6010) || \
	defined(CONFIG_USB_MUSB_TUSB6010_MODULE)
	void __iomem *conf;
#endif

	/* index in musb->endpoints[]  */
	u8 epnum;

	/* hardware configuration, possibly dynamic */
	bool is_shared_fifo;
	/* bool                  tx_double_buffered; */
	/* bool                  rx_double_buffered; */
	u16 max_packet_sz_tx;
	u16 max_packet_sz_rx;

	/* For ssusb+ */
	u32 fifoaddr_tx;
	u32 fifoaddr_rx;

	u8 mult_tx;
	u8 mult_rx;

	u8 interval_tx;
	u8 interval_rx;
	/* For ssusb- */

	struct dma_channel *tx_channel;
	struct dma_channel *rx_channel;

#if defined(CONFIG_USB_MUSB_TUSB6010) || \
	defined(CONFIG_USB_MUSB_TUSB6010_MODULE)
	/* TUSB has "asynchronous" and "synchronous" dma modes */
	dma_addr_t fifo_async;
	dma_addr_t fifo_sync;
	void __iomem *fifo_sync_va;
#endif

	void __iomem *target_regs;

	/* currently scheduled peripheral endpoint */
	struct musb_qh *in_qh;
	struct musb_qh *out_qh;

	u8 rx_reinit;
	u8 tx_reinit;

	/* peripheral side */
	struct musb_ep ep_in;	/* TX */
	struct musb_ep ep_out;	/* RX */
};

static inline struct musb_request *next_in_request(struct musb_hw_ep *hw_ep)
{
	return next_request(&hw_ep->ep_in);
}

static inline struct musb_request *next_out_request(struct musb_hw_ep *hw_ep)
{
	return next_request(&hw_ep->ep_out);
}

#ifdef NEVER
struct musb_csr_regs {
	/* FIFO registers */
	u16 txmaxp, txcsr, rxmaxp, rxcsr;
	u16 rxfifoadd, txfifoadd;
	u8 txtype, txinterval, rxtype, rxinterval;
	u8 rxfifosz, txfifosz;
	u8 txfunaddr, txhubaddr, txhubport;
	u8 rxfunaddr, rxhubaddr, rxhubport;
};

struct musb_context_registers {

	u8 power;
	u16 intrtxe, intrrxe;
	u8 intrusbe;
	u16 frame;
	u8 index, testmode;

	u8 devctl, busctl, misc;

	struct musb_csr_regs index_regs[MUSB_C_NUM_EPS];
};
#endif				/* NEVER */

#ifdef CONFIG_USB_MU3D_DRV
struct musb_csr_regs {
	/* FIFO registers */
	/* u32 txcsr0, txcsr1, txcsr2; */
	/* u32 rxcsr0, rxcsr1, rxcsr2; */
#ifdef USE_SSUSB_QMU
	u32 txqmuaddr, rxqmuaddr;
#endif
	/* u16 txmaxp, txcsr, rxmaxp, rxcsr; */
	/* u16 rxfifoadd, txfifoadd; */
	/* u8 txtype, txinterval, rxtype, rxinterval; */
	/* u8 rxfifosz, txfifosz; */

	/* u8 txfunaddr, txhubaddr, txhubport; */
	/* u8 rxfunaddr, rxhubaddr, rxhubport; */
};

struct musb_context_registers {

	/* u8 power; */
	/* u16 intrtxe, intrrxe; */
	/* u8 intrusbe; */
	/* u16 frame; */
	/* u8 index, testmode; */

	/* u8 devctl, busctl, misc; */
	/* u32 intr_ep; */
	/* u32 ep0_csr; */
	/* u32 qmu_crs, intr_qmu_done; */
	struct musb_csr_regs index_regs[MUSB_C_NUM_EPS];
};
#endif

/*
 * struct musb - Driver instance data.
 */
struct musb {
	/* device lock */
	spinlock_t lock;
	struct semaphore musb_lock;

	const struct musb_platform_ops *ops;
	struct musb_context_registers context;

	 irqreturn_t (*isr)(int, void *);
	struct work_struct irq_work;
	u32 hwvers;

/* this hub status bit is reserved by USB 2.0 and not seen by usbcore */
#define MUSB_PORT_STAT_RESUME	(1 << 31)

	u32 port1_status;

	unsigned long rh_timer;

	enum musb_h_ep0_state ep0_stage;

	/* bulk traffic normally dedicates endpoint hardware, and each
	 * direction has its own ring of host side endpoints.
	 * we try to progress the transfer at the head of each endpoint's
	 * queue until it completes or NAKs too much; then we try the next
	 * endpoint.
	 */
	struct musb_hw_ep *bulk_ep;

	struct list_head control;	/* of musb_qh */
	struct list_head in_bulk;	/* of musb_qh */
	struct list_head out_bulk;	/* of musb_qh */

	struct timer_list otg_timer;
	struct notifier_block nb;

	struct dma_controller *dma_controller;

	struct device *controller;
	void __iomem *ctrl_base;
	void __iomem *mregs;

#if defined(CONFIG_USB_MUSB_TUSB6010) || \
	defined(CONFIG_USB_MUSB_TUSB6010_MODULE)
	dma_addr_t async;
	dma_addr_t sync;
	void __iomem *sync_va;
#endif

	/* passed down from chip/board specific irq handlers */
	u32 int_usb;
	u16 int_rx;
	u16 int_tx;

	struct usb_phy *xceiv;

	int nIrq;
	unsigned irq_wake:1;

	struct musb_hw_ep endpoints[MUSB_C_NUM_EPS];
#define control_ep		endpoints

#define VBUSERR_RETRY_COUNT	3
	u16 vbuserr_retry;
	u16 epmask;
	u8 nr_endpoints;

	u8 board_mode;		/* enum musb_mode */
	int (*board_set_power)(int state);

	u8 min_power;		/* vbus for periph, in mA/2 */

	bool is_host;

	int a_wait_bcon;	/* VBUS timeout in msecs */
	unsigned long idle_timeout;	/* Next timeout in jiffies */

	/* active means connected and not suspended */
	unsigned is_active:1;

	unsigned is_multipoint:1;
	unsigned ignore_disconnect:1;	/* during bus resets */

	unsigned hb_iso_rx:1;	/* high bandwidth iso rx? */
	unsigned hb_iso_tx:1;	/* high bandwidth iso tx? */
	unsigned dyn_fifo:1;	/* dynamic FIFO supported? */

	unsigned bulk_split:1;
#define	can_bulk_split(musb, type) \
	(((type) == USB_ENDPOINT_XFER_BULK) && (musb)->bulk_split)

	unsigned bulk_combine:1;
#define	can_bulk_combine(musb, type) \
	(((type) == USB_ENDPOINT_XFER_BULK) && (musb)->bulk_combine)

	/* is_suspended means USB B_PERIPHERAL suspend */
	unsigned is_suspended:1;

	/* may_wakeup means remote wakeup is enabled */
	unsigned may_wakeup:1;

	/* is_self_powered is reported in device status and the
	 * config descriptor.  is_bus_powered means B_PERIPHERAL
	 * draws some VBUS current; both can be true.
	 */
	unsigned is_self_powered:1;
	unsigned is_bus_powered:1;

	unsigned set_address:1;
	unsigned test_mode:1;
	unsigned softconnect:1;

	u8 address;
	u8 test_mode_nr;
	bool in_ipo_off;
	u32 ackpend;		/* ep0 *//*We don't maintain Max Packet size in it. */
	enum musb_g_ep0_state ep0_state;
	struct usb_gadget g;	/* the gadget */
	struct usb_gadget_driver *gadget_driver;	/* its driver */

	/*
	 * FIXME: Remove this flag.
	 *
	 * This is only added to allow Blackfin to work
	 * with current driver. For some unknown reason
	 * Blackfin doesn't work with double buffering
	 * and that's enabled by default.
	 *
	 * We added this flag to forcefully disable double
	 * buffering until we get it working.
	 */
	unsigned double_buffer_not_ok:1;

	struct musb_hdrc_config *config;

#ifdef MUSB_CONFIG_PROC_FS
	struct proc_dir_entry *proc_entry;
#endif

	u32 txfifoadd_offset;
	u32 rxfifoadd_offset;
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_root;
#endif

	unsigned is_clk_on;
	unsigned usb_mode;
	unsigned active_ep;
	struct work_struct suspend_work;
	struct wake_lock usb_wakelock;
	struct delayed_work connection_work;
	struct delayed_work check_ltssm_work;
#ifndef CONFIG_USBIF_COMPLIANCE
	struct delayed_work reconnect_work;
#endif
#ifdef EP_PROFILING
	struct delayed_work ep_prof_work;
#endif

#ifdef USE_SSUSB_QMU
	struct tasklet_struct qmu_done;
	u32 qmu_done_intr;

	struct tasklet_struct error_recovery;
	u32 error_wQmuVal;
	u32 error_wErrVal;
#endif
};

static inline struct musb *gadget_to_musb(struct usb_gadget *g)
{
	return container_of(g, struct musb, g);
}

#ifdef CONFIG_BLACKFIN
static inline int musb_read_fifosize(struct musb *musb, struct musb_hw_ep *hw_ep, u8 epnum)
{
	musb->nr_endpoints++;
	musb->epmask |= (1 << epnum);

	if (epnum < 5) {
		hw_ep->max_packet_sz_tx = 128;
		hw_ep->max_packet_sz_rx = 128;
	} else {
		hw_ep->max_packet_sz_tx = 1024;
		hw_ep->max_packet_sz_rx = 1024;
	}
	hw_ep->is_shared_fifo = false;

	return 0;
}

static inline void musb_configure_ep0(struct musb *musb)
{
	musb->endpoints[0].max_packet_sz_tx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].max_packet_sz_rx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].is_shared_fifo = true;
}

#else

static inline int musb_read_fifosize(struct musb *musb, struct musb_hw_ep *hw_ep, u8 epnum)
{
	void __iomem *mbase = musb->mregs;
	u8 reg = 0;

	/* read from core using indexed model */
	reg = musb_readb(mbase, MUSB_EP_OFFSET(epnum, MUSB_FIFOSIZE));
	/* 0's returned when no more endpoints */
	if (!reg)
		return -ENODEV;

	musb->nr_endpoints++;
	musb->epmask |= (1 << epnum);

	hw_ep->max_packet_sz_tx = 1 << (reg & 0x0f);

	/* shared TX/RX FIFO? */
	if ((reg & 0xf0) == 0xf0) {
		hw_ep->max_packet_sz_rx = hw_ep->max_packet_sz_tx;
		hw_ep->is_shared_fifo = true;
	} else {
		hw_ep->max_packet_sz_rx = 1 << ((reg & 0xf0) >> 4);
		hw_ep->is_shared_fifo = false;
	}

	return 0;
}

static inline void musb_configure_ep0(struct musb *musb)
{
	musb->endpoints[0].max_packet_sz_tx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].max_packet_sz_rx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].is_shared_fifo = true;
}
#endif				/* CONFIG_BLACKFIN */


/***************************** Glue it together *****************************/

extern const char musb_driver_name[];

extern void musb_start(struct musb *musb);
extern void musb_stop(struct musb *musb);

extern void musb_write_fifo(struct musb_hw_ep *ep, u16 len, const u8 *src);
extern void musb_read_fifo(struct musb_hw_ep *ep, u16 len, u8 *dst);

extern void musb_load_testpacket(struct musb *);

extern irqreturn_t musb_interrupt(struct musb *);

extern void musb_hnp_stop(struct musb *musb);

static inline void musb_platform_set_vbus(struct musb *musb, int is_on)
{
	if (musb->ops->set_vbus)
		musb->ops->set_vbus(musb, is_on);
}

static inline void musb_platform_enable(struct musb *musb)
{
	if (musb->ops->enable)
		musb->ops->enable(musb);
}

static inline void musb_platform_disable(struct musb *musb)
{
	if (musb->ops->disable)
		musb->ops->disable(musb);
}

static inline int musb_platform_set_mode(struct musb *musb, u8 mode)
{
	if (!musb->ops->set_mode)
		return 0;

	return musb->ops->set_mode(musb, mode);
}

static inline void musb_platform_try_idle(struct musb *musb, unsigned long timeout)
{
	if (musb->ops->try_idle)
		musb->ops->try_idle(musb, timeout);
}

static inline int musb_platform_get_vbus_status(struct musb *musb)
{
	if (!musb->ops->vbus_status)
		return 0;

	return musb->ops->vbus_status(musb);
}

static inline int musb_platform_init(struct musb *musb)
{
	if (!musb->ops->init)
		return -EINVAL;

	return musb->ops->init(musb);
}

static inline int musb_platform_exit(struct musb *musb)
{
	if (!musb->ops->exit)
		return -EINVAL;

	return musb->ops->exit(musb);
}

extern bool usb_cable_connected(void);
extern void usb_phy_savecurrent(unsigned int clk_on);
extern void usb_phy_recover(unsigned int clk_on);
extern void usb_fake_powerdown(unsigned int clk_on);
extern void connection_work(struct work_struct *data);
extern void check_ltssm_work(struct work_struct *data);

#ifndef CONFIG_USBIF_COMPLIANCE
extern void reconnect_work(struct work_struct *data);
extern struct timespec get_connect_timestamp(void);
#else				/*CONFIG_USBIF_COMPLIANCE */
extern void init_connection_work(void);
extern void init_check_ltssm_work(void);
extern void Charger_Detect_En(bool enable);
#endif				/*CONFIG_USBIF_COMPLIANCE */


extern void musb_sync_with_bat(struct musb *musb, int usb_state);


#ifdef CONFIG_MTK_UART_USB_SWITCH
extern bool in_uart_mode;
extern ssize_t musb_portmode_show(struct device *dev, struct device_attribute *attr, char *buf);
extern ssize_t musb_portmode_store(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count);
extern ssize_t musb_tx_show(struct device *dev, struct device_attribute *attr, char *buf);
extern ssize_t musb_tx_store(struct device *dev, struct device_attribute *attr, const char *buf,
			     size_t count);
extern ssize_t musb_rx_show(struct device *dev, struct device_attribute *attr, char *buf);
extern ssize_t musb_uart_path_show(struct device *dev, struct device_attribute *attr, char *buf);
extern bool usb_phy_check_in_uart_mode(void);
extern void usb_phy_switch_to_uart(void);
#endif				/*CONFIG_MTK_UART_USB_SWITCH */


extern ssize_t musb_cmode_show(struct device *dev, struct device_attribute *attr, char *buf);
extern ssize_t musb_cmode_store(struct device *dev, struct device_attribute *attr, const char *buf,
				size_t count);

extern void usb20_pll_settings(bool host, bool forceOn);

#if defined(FOR_BRING_UP) || !defined(CONFIG_MTK_SMART_BATTERY)
/* implement static function in mt_usb.c */
#else
extern bool upmu_is_chr_det(void);
extern u32 upmu_get_rgs_chrdet(void);
#endif

#ifdef CONFIG_USB_MTK_DUALMODE
extern bool mtk_is_host_mode(void);
extern void mtk_unload_xhci_on_ipo(void);
extern void switch_int_to_host_and_mask(void);
extern void switch_int_to_host(void);
#else
static inline int mtk_is_host_mode(void)
{
	return 0;
}
#endif
#endif	/* __MUSB_CORE_H__ */
