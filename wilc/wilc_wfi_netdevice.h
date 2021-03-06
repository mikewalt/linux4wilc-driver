/* SPDX-License-Identifier: GPL-2.0 */
/*!
 *  @file	wilc_wfi_netdevice.h
 *  @brief	Definitions for the network module
 *  @author	mdaftedar
 *  @date	01 MAR 2012
 *  @version	1.0
 */
#ifndef WILC_WFI_NETDEVICE
#define WILC_WFI_NETDEVICE

#define WILC_MAX_NUM_PMKIDS  16
#define PMKID_LEN  16
#define PMKID_FOUND 1
 #define NUM_STA_ASSOCIATED 8

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <net/ieee80211_radiotap.h>
#include <linux/if_arp.h>
#include <linux/in6.h>
#include <asm/checksum.h>
#include "host_interface.h"
#include "wilc_wlan.h"
#include "wilc_wlan_if.h"
#include <linux/wireless.h>
#include <linux/completion.h>
#include <linux/mutex.h>

#define FLOW_CONTROL_LOWER_THRESHOLD	128
#define FLOW_CONTROL_UPPER_THRESHOLD	256

#define ANT_SWTCH_INVALID_GPIO_CTRL 	0
#define ANT_SWTCH_SNGL_GPIO_CTRL 	1
#define ANT_SWTCH_DUAL_GPIO_CTRL	2

struct wilc_wfi_stats {
	unsigned long rx_packets;
	unsigned long tx_packets;
	unsigned long rx_bytes;
	unsigned long tx_bytes;
	u64 rx_time;
	u64 tx_time;

};

#define NUM_REG_FRAME 2

struct wilc_wfi_key {
	u8 *key;
	u8 *seq;
	int key_len;
	int seq_len;
	u32 cipher;
};

struct wilc_wfi_wep_key {
	u8 *key;
	u8 key_len;
	u8 key_idx;
};

struct sta_info {
	u8 sta_associated_bss[MAX_NUM_STA][ETH_ALEN];
};

/*Parameters needed for host interface for  remaining on channel*/
struct wilc_wfi_p2p_listen_params {
	struct ieee80211_channel *listen_ch;
	u32 listen_duration;
	u64 listen_cookie;
	u32 listen_session_id;
};

/* Struct to buffer eapol 1/4 frame */
struct wilc_buffered_eap {
	unsigned int size;
	unsigned int pkt_offset;
	u8 *buff;
};

struct wilc_priv {
	struct wireless_dev *wdev;
	struct cfg80211_scan_request *scan_req;
	struct wilc_wfi_p2p_listen_params remain_on_ch_params;
	u64 tx_cookie;
	bool cfg_scanning;
	u32 rcvd_ch_cnt;

	u8 associated_bss[ETH_ALEN];
	struct sta_info assoc_stainfo;
	struct net_device_stats stats;
	u8 monitor_flag;
	int status;
	struct sk_buff *skb;
	spinlock_t lock;
	struct net_device *dev;
	struct host_if_drv *hif_drv;
	struct host_if_pmkid_attr pmkid_list;
	struct wilc_wfi_stats netstats;
	u8 wep_key[4][WLAN_KEY_LEN_WEP104];
	u8 wep_key_len[4];
	/* The real interface that the monitor is on */
	struct net_device *real_ndev;
	struct wilc_wfi_key *wilc_gtk[MAX_NUM_STA];
	struct wilc_wfi_key *wilc_ptk[MAX_NUM_STA];
	u8 wilc_groupkey;

	struct mutex scan_req_lock;
	/*  */
	bool auto_rate_adjusted;

	bool p2p_listen_state;
	struct wilc_buffered_eap *buffered_eap;

	struct timer_list eap_buff_timer;
	struct timer_list during_ip_timer;
};

struct frame_reg {
	u16 type;
	bool reg;
};

typedef struct {
	bool p2p_mode;
	u8 ant_swtch_mode;
	u8 antenna1;
	u8 antenna2;
} sysfs_attr_group;

struct wilc_vif {
	u8 idx;
	u8 iftype;
	int monitor_flag;
	int mac_opened;
	struct frame_reg frame_reg[NUM_REG_FRAME];
	struct net_device_stats netstats;
	struct wilc *wilc;
	u8 src_addr[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	struct host_if_drv *hif_drv;
	struct net_device *ndev;
	u8 ifc_id;

	sysfs_attr_group attr_sysfs;
#ifdef DISABLE_PWRSAVE_AND_SCAN_DURING_IP
	bool pwrsave_current_state;
#endif
};

struct wilc {
	u64 tmp_buf;
	const struct wilc_hif_func *hif_func;
	int io_type;
	int mac_status;
	int gpio_irq;
	int gpio_reset;
	int gpio_chip_en;
	bool initialized;
	int dev_irq_num;
	int close;
	u8 vif_num;
	struct wilc_vif *vif[NUM_CONCURRENT_IFC];
	u8 open_ifcs;

	struct mutex txq_add_to_head_cs;
	spinlock_t txq_spinlock;

	struct mutex rxq_cs;
	struct mutex hif_cs;

	struct completion cfg_event;
	struct completion sync_event;
	struct completion txq_event;
	struct completion txq_thread_started;
	struct completion debug_thread_started;
	struct task_struct *txq_thread;
	struct task_struct *debug_thread;

	int quit;
	int cfg_frame_in_use;
	struct wilc_cfg_frame cfg_frame;
	u32 cfg_frame_offset;
	int cfg_seq_no;

	u8 *rx_buffer;
	u32 rx_buffer_offset;
	u8 *tx_buffer;

	unsigned long txq_spinlock_flags;

	struct txq_entry_t *txq_head;
	struct txq_entry_t *txq_tail;
	struct txq_handle txq[NQUEUES];
	int txq_entries;
	int txq_exit;

	struct rxq_entry_t *rxq_head;
	struct rxq_entry_t *rxq_tail;
	int rxq_entries;
	int rxq_exit;

	unsigned char eth_src_address[NUM_CONCURRENT_IFC][6];

	const struct firmware *firmware;

	struct device *dev;

	struct rf_info dummy_statistics;

	enum wilc_chip_type chip;

	uint8_t power_status[PWR_DEV_SRC_MAX];
	uint8_t keep_awake[PWR_DEV_SRC_MAX];
	struct mutex cs;

	struct timer_list aging_timer;
	struct wilc_vif *aging_timer_vif;
};

struct WILC_WFI_mon_priv {
	struct net_device *real_ndev;
};

void wilc_frmw_to_linux(struct wilc *wilc, u8 *buff, u32 size, u32 pkt_offset,
			u8 status);
void wilc_mac_indicate(struct wilc *wilc, int flag);
void wilc_netdev_cleanup(struct wilc *wilc);
int wilc_netdev_init(struct wilc **wilc, struct device *dev, int io_type,
		     const struct wilc_hif_func *ops);
void wilc_wfi_mgmt_rx(struct wilc *wilc, u8 *buff, u32 size);
int wilc_wlan_set_bssid(struct net_device *wilc_netdev, u8 *bssid, u8 mode);

#endif
