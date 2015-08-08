/*
 * Copyright (C) 2014-2014 Wiky L(wiiiky@yeah.net)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __WL_DOWNLOADER_H__
#define __WL_DOWNLOADER_H__

#include "wlhttper.h"
#include "wlhttpermenu.h"
#include "wlbter.h"
#include "libtransmission/transmission.h"

G_BEGIN_DECLS
/***
****
***/
#define MEM_K 1024
#define MEM_K_STR "KiB"
#define MEM_M_STR "MiB"
#define MEM_G_STR "GiB"
#define MEM_T_STR "TiB"
#define DISK_K 1000
#define DISK_B_STR   "B"
#define DISK_K_STR "kB"
#define DISK_M_STR "MB"
#define DISK_G_STR "GB"
#define DISK_T_STR "TB"
#define SPEED_K 1000
#define SPEED_B_STR  "B/s"
#define SPEED_K_STR "kB/s"
#define SPEED_M_STR "MB/s"
#define SPEED_G_STR "GB/s"
#define SPEED_T_STR "TB/s"
/*
 * Type macros
 */
#define WL_TYPE_DOWNLOADER (wl_downloader_get_type())
#define WL_DOWNLOADER(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj),\
				WL_TYPE_DOWNLOADER,WlDownloader))
#define WL_IS_DOWNLOADER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE((obj),\
				WL_TYPE_DOWNLOADER))
#define WL_DOWNLOADER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),\
				WL_TYPE_DOWNLOADER,WlDownloaderClass))
#define WL_IS_DOWNLOADER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),\
				WL_TYPE_DOWNLOADER))
#define WL_DOWNLOADER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_TYPE((obj),\
				WL_TYPE_DOWNLOADER,WlDownloaderClass))
typedef struct _WlDownloader WlDownloader;
typedef struct _WlDownloaderClass WlDownloaderClass;

typedef void (*WlDownloaderSelectedCallback) (WlDownloader * dl,
											  gpointer data);

struct _WlDownloader {
	GtkScrolledWindow parent;
	/* Private */
	GtkWidget *vBox;
	GList *list;
	gpointer *selected;
	tr_session *session;
	/* Configure */
	gchar *keyFilePath;

	/* Callback */
	WlDownloaderSelectedCallback selectedCB;
	gpointer selectedCBData;
	WlHttperStatusCallback httperStatus;
	gpointer httperStatusData;
	WlBterStatusCallback bterStatus;
	gpointer bterStatusData;
};

struct _WlDownloaderClass {
	GtkScrolledWindowClass parentClass;
};

GType wl_downloader_get_type(void) G_GNUC_CONST;
/*
 * public functions
 */
WlDownloader *wl_downloader_new(void);
/*
 * @description 添加一个HTTP下载任务
 * @param url HTTP下载地址
 * @param path 文件保存路径
 * @return 返回添加的WlHttper对象
 */
WlHttper *wl_downloader_append_httper(WlDownloader * dl, const gchar * url,
									  const gchar * path);
/*
 * @description 移除一个下载任务
 * @param httper 移除的目标
 */
void wl_downloader_remove_httper(WlDownloader * dl, WlHttper * httper);
/*
 * @param local是否移除下载文件
 */
void wl_downloader_remove_bter(WlDownloader * dl, WlBter * bter,
							   gboolean local);
/*
 * @description 添加一个BT下载任务
 * @param torrent 一个tr_torrent指针
 * @return 返回添加的WlBter对象
 */
WlBter *wl_downloader_append_bter(WlDownloader * dl, tr_torrent * torrent);
/*
 * @description 添加一个BT下载任务，指定种子源文件
 * @param path 种子文件的路径
 * @param 返回添加的WlBter对象,失败返回NULL
 */
WlBter *wl_downloader_append_bter_from_file(WlDownloader * dl,
											const gchar * path);
/*
 * @description 开始选中的下载任务
 */
void wl_downloader_start_selected(WlDownloader * dl);
/*
 * @description 暂停选中的下载任务
 */
void wl_downloader_pause_selected(WlDownloader * dl);
/*
 * @description 继续选中的下载任务
 */
void wl_downloader_continue_selected(WlDownloader * dl);
/*
 * @description 获取选中任务的状态
 * @return 如果没有选中，返回0
 */
gint wl_downloader_get_selected_status(WlDownloader * dl);
/*
 * @description 选中的目标
 * @return 返回选中的目标或者NULL
 */
gpointer wl_downloader_get_selected(WlDownloader * dl);
/*
 * @description 删除选中的任务
 */
void wl_downloader_remove_selected(WlDownloader * dl, gboolean local);

/*
 * @description 设置选中的HTTPER改变时的回调函数
 * @param callback
 * @param data
 */
void wl_downloader_set_selected_callback(WlDownloader * dl,
										 WlDownloaderSelectedCallback
										 callback, gpointer data);
/*
 * @description
 * @param callback
 * @param data
 */
void wl_downloader_set_httper_status_callback(WlDownloader * dl,
											  WlHttperStatusCallback
											  callback, gpointer data);
void wl_downloader_set_bter_status_callback(WlDownloader * dl,
											WlBterStatusCallback callback,
											gpointer data);
/*
 * @description 创建种子tr_torrent*对象，但不添加Wlbter
 * @param path 种子文件路径
 * @return 返回tr_torrent*对象
 */
tr_torrent *wl_downloader_create_torrent(WlDownloader * dl,
										 const gchar * path);
/*
 * @description 创建种子构造对象tr_ctor
 * @param path种子文件路径
 * @return tr_ctor
 */
tr_ctor *wl_downloader_create_ctor(WlDownloader * dl);

/*
 * @description 保存和读取下载任务
 */
void wl_downloader_save_tasks(WlDownloader * dl);
void wl_downloader_load_tasks(WlDownloader * dl);


G_END_DECLS
#endif							/* __WL_DOWNLOADER_H__ */
