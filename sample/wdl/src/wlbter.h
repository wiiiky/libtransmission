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
#ifndef __WL_BTER_H__
#define __WL_BTER_H__

#include <gtk/gtk.h>
#include "libtransmission/transmission.h"
#include "libtransmission/variant.h"

G_BEGIN_DECLS
/*
 * Macro Type
 */
#define WL_TYPE_BTER	(wl_bter_get_type())
#define WL_BTER(obj) (G_TYPE_CHECK_INSTANCE_CAST(\
			(obj),WL_TYPE_BTER,WlBter))
#define WL_IS_BTER(obj) (G_TYPE_CHECK_INSTANCE_TYPE(\
			(obj),WL_TYPE_BTER))
#define WL_BTER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST(\
			(klass),WL_TYPE_BTER,WlBterClass))
#define WL_IS_BTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE(\
			(klass),WL_TYPE_BTER))
#define WL_BTER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_TYPE(\
			(obj),WL_TYPE_BTER,WlBterClass))
typedef struct _WlBter WlBter;
typedef struct _WlBterClass WlBterClass;
typedef enum _WlBterStatus WlBterStatus;

typedef void (*WlBterStatusCallback) (WlBter * bter, gpointer data);

enum _WlBterStatus {
	WL_BTER_STATUS_START = 11111,
	WL_BTER_STATUS_PAUSE = 22222,
	WL_BTER_STATUS_COMPLETE = 33333,
	WL_BTER_STATUS_NOT_START = 44444,
	WL_BTER_STATUS_ABORT = 55555,
};

struct _WlBter {
	GtkEventBox parent;
	/* GUI */
	GtkWidget *icon;
	GtkWidget *titleLabel;
	GtkWidget *dlLabel;
	GtkWidget *totalLabel;
	GtkWidget *speedLabel;
	GtkWidget *timeLabel;
	GtkWidget *progressBar;
	/* Menu */
	GtkWidget *popMenu;

	guint64 totalLast;

	/* libtransmission的会话，由外部传递进来 */
	tr_session *session;
	/* torrent的指针，可以是外部传递，也可以是自身创建 */
	tr_torrent *torrent;
	/* 当前的下载状态 */
	WlBterStatus status;
	/* 定时器 */
	guint timeout;

	/* 状态改变回调函数 */
	WlBterStatusCallback statusCB;
	gpointer statusCBData;

	/* */
	gpointer userData;
};

struct _WlBterClass {
	GtkEventBoxClass parentClass;
};

GType wl_bter_get_type(void) G_GNUC_CONST;

/*************************************************
 * PUBLIC
 ***********************************************/
WlBter *wl_bter_new(tr_session * session, tr_torrent * torrent);
WlBter *wl_bter_new_from_file(tr_session * session, const gchar * path);
WlBter *wl_bter_new_from_magnetlink(tr_session * session,
									const gchar * link);

/*
 * @description 开始下载任务
 */
void wl_bter_start(WlBter * bter);
/* 继续任务 */
#define wl_bter_continue(bter)  wl_bter_start ((bter));
/*
 * @description 暂停任务
 */
void wl_bter_pause(WlBter * bter);
/*
 * @description 当前下载状态
 * @return WlBterStatus
 */
gint wl_bter_get_status(WlBter * bter);
/*
 * @description 获取tr_torrent对象
 */
tr_torrent *wl_bter_get_torrent(WlBter * bter);
/*
 * @description 获取下载目录
 */
const gchar *wl_bter_get_path(WlBter * bter);
/*
 * @description 获取magnet link
 */
const gchar *wl_bter_get_magnet(WlBter * bter);
/*
 * @description 高亮\取消高亮下载任务
 */
void wl_bter_highlight(WlBter * bter);
void wl_bter_clear_highlight(WlBter * bter);

/*
 * @description 设置状态改变回调函数
 */
void wl_bter_set_status_callback(WlBter * bter,
								 WlBterStatusCallback callback,
								 gpointer data);

/*
 * @description 设置右键菜单
 */
void wl_bter_set_popmenu(WlBter * bter, GtkWidget * menu);
GtkWidget *wl_bter_get_popmenu(WlBter * bter);

/*
 * @description 设置用户数据
 */
void wl_bter_set_user_data(WlBter * bter, gpointer data);
gpointer wl_bter_get_user_data(WlBter * bter);


G_END_DECLS						/* __WL_BTER_H__ */
#endif
