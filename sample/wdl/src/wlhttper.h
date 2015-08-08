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
#ifndef __WL_HTTPER_H__
#define __WL_HTTPER_H__

#include <gtk/gtk.h>
#include <curl/curl.h>

G_BEGIN_DECLS
/*
 * Type macros.
 */
#define WL_TYPE_HTTPER	(wl_httper_get_type())
#define WL_HTTPER(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj),WL_TYPE_HTTPER,\
				WlHttper))
#define WL_IS_HTTPER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE((obj),\
				WL_TYPE_HTTPER))
#define WL_HTPPER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),\
				WL_TYPE_HTTPER,WlHttperClass))
#define WL_IS_HTTPER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),\
				WL_TYPE_HTTPER))
#define WL_HTTPER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_TYPE((obj),\
				WL_TYPE_HTTPER,WlHttperClass))
 /**/ typedef struct _WlHttper WlHttper;
typedef struct _WlHttperClass WlHttperClass;
typedef enum _WlHttperStatus WlHttperStatus;
typedef void (*WlHttperCallback) (gint code, gpointer data);
typedef void (*WlHttperStatusCallback) (WlHttper * httper, gpointer data);


struct _WlHttper {
	GtkEventBox parent;
	GtkWidget *iconImage;
	GtkWidget *titleLabel;
	GtkWidget *progressBar;
	GtkWidget *dlLabel;
	GtkWidget *totalLabel;
	GtkWidget *speedLabel;
	GtkWidget *timeLabel;

	GtkIconSize iconSize;
	gchar *url;
	/* 保存路径和文件流 */
	gchar *savePath;
	GFileOutputStream *fOutput;
	GThread *thread;
	CURL *easyCURL;
	/* 下载速度 */
	gdouble speed;
	/* 下载完成百分比 */
	gdouble percentDone;
	/*
	 * 这两个字段用来计算下载速度
	 * dlData表示在dlTime时间内下载的字节数
	 */
	guint64 dlData;
	guint64 dlTime;
	/*
	 * 下面的子段在断点续传用到，表示已经下载的数据
	 */
	guint64 alreadyHave;
	/* */
	guint timeout;
	/* 已下载的数据和总数据 */
	guint64 dlNow;
	guint64 dlTotal;
	guint64 totalLast;
	/* 当前的状态 */
	gint status;
	/* 完成后返回结果 */
	GAsyncQueue *rqueue;
	/* 发送取消命令 */
	GAsyncQueue *cqueue;
	/* 创建时间，不可设置，初始化自动填写 */
	GDateTime *cdt;
	/* 右键菜单 */
	GtkWidget *popMenu;
	/* 用户自定义数据 */
	gpointer userData;
	/* 完成下载后的回调函数,第一个参数是下载结果 */
	WlHttperCallback finishCallback;
	gpointer cbData;
	/* 状态改变的回调函数 */
	WlHttperStatusCallback statusCallback;
	gpointer statusData;
};

struct _WlHttperClass {
	GtkEventBoxClass parent_class;
};

enum _WlHttperStatus {
	WL_HTTPER_STATUS_START = 11111,
	WL_HTTPER_STATUS_PAUSE = 22222,
	WL_HTTPER_STATUS_COMPLETE = 33333,
	WL_HTTPER_STATUS_NOT_START = 44444,
	WL_HTTPER_STATUS_ABORT = 55555,
	WL_HTTPER_STATUS_RESUME = 66666,
};

GType wl_httper_get_type() G_GNUC_CONST;

/*
 * public functions
 */


/*
 * @description,创建一个新的HTTP下载
 * @param url 下载的URL链接可以是HTTP或者是FTP
 * @param savePath 保存路径
 */
WlHttper *wl_httper_new(const gchar * url, const gchar * savePath);
/*
 */
 void wl_httper_set_title(WlHttper * httper, const gchar * title);
 void wl_httper_set_title_from_path(WlHttper * httper,
										  const gchar * path);
 void wl_httper_set_icon(WlHttper * httper, GIcon * icon);
 void wl_httper_set_icon_from_name(WlHttper * httper,
										 const gchar * name);
 void wl_httper_set_icon_from_file(WlHttper * httper,
										 const gchar * path);
/*
 * @descriptioin 获取和设置用户自定义数据
 */
 gpointer wl_httper_get_user_data(WlHttper * httper);
 void wl_httper_set_user_data(WlHttper * httper, gpointer data);
/*
 * @description 设置完成下载后的回调函数
 */
 void wl_httper_set_callback(WlHttper * httper,
								   WlHttperCallback callback,
								   gpointer data);
/*
 * @description 设置和获取右键菜单
 */
 void wl_httper_set_popmenu(WlHttper * httper, GtkWidget * menu);
 GtkWidget *wl_httper_get_popmenu(WlHttper * httper);
/*
 * @description 返回保存路径
 */
 const gchar *wl_httper_get_path(WlHttper * httper);
/*
 * @description 返回下载URL
 */
 const gchar *wl_httper_get_url(WlHttper * httper);

/*
 * @description 打开文件的保存路径
 */
gboolean wl_httper_launch_path(WlHttper * httper);

/*
 * @description 获取当前的状态
 */
gint wl_httper_get_status(WlHttper * httper);
/*
 * @description 开始下载，调用该函数(重新)开始下载，
 * 				如果是暂停以后继续使用wl_httper_continue;
 */
void wl_httper_start(WlHttper * httper);
/*
 * @description 暂停下载
 */
void wl_httper_pause(WlHttper * httper);
/*
 * @description 暂停下载后继续
 */
void wl_httper_continue(WlHttper * httper);
/*
 * @description 停止下载
 */
void wl_httper_abort(WlHttper * httper);
/*
 * @description 重新下载
 */
void wl_httper_redownload(WlHttper * httper);
/*
 * @description 设置背景
 */
void wl_httper_highlight(WlHttper * httper);
/*
 * @description 清除背景颜色
 */
void wl_httper_clear_highlight(WlHttper * httper);
/*
 * @description 获取httper的添加时间
 */
const GDateTime *wl_httper_get_ctime(WlHttper * httper);
/*
 * @description 获取文件名
 */
const gchar *wl_httper_get_title(WlHttper * httper);
/*
 * @descriptio 设置状态改变回调函数
 */
void wl_httper_set_status_callback(WlHttper * httper,
								   WlHttperStatusCallback callback,
								   gpointer data);
/*
 * @description 获取文件总长度/已下载长度
 */
guint64 wl_httper_get_total_size(WlHttper * httper);
guint64 wl_httper_get_dl_size(WlHttper * httper);
/*
 * @description
 */
void wl_httper_load(WlHttper * httper, guint64 total_size, guint64 dl_size,
					guint status);

G_END_DECLS
#endif							/* __WL_HTTPER_H__ */
