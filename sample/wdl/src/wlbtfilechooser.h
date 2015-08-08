/*
 * Copyright (C) 2014-2014 Wiky L(wiiiky@yeah.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WL_BT_FILE_CHOOSER_H__
#define __WL_BT_FILE_CHOOSER_H__

#include <gtk/gtk.h>
#include "libtransmission/transmission.h"


#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "./data"
#endif

#define UI_FILE "./data/torrent.ui"

G_BEGIN_DECLS
/* Macro for casting a pointer to a WlBtFileChooser or WlBtFileChooserClass pointer.
 * Macros for testing whether `object' or `klass' are of type WL_TYPE_BT_FILE_CHOOSER.
 */
#define WL_TYPE_BT_FILE_CHOOSER	(wl_bt_file_chooser_get_type())
#define WL_BT_FILE_CHOOSER(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WL_TYPE_BT_FILE_CHOOSER, WlBtFileChooser))
#define WL_BT_FILE_CHOOSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), WL_TYPE_BT_FILE_CHOOSER, WlBtFileChooserClass))
#define WL_IS_BT_FILE_CHOOSER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), WL_TYPE_BT_FILE_CHOOSER))
#define WL_IS_BT_FILE_CHOOSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), WL_TYPE_BT_FILE_CHOOSER))
#define WL_BT_FILE_CHOOSER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), WL_TYPE_BT_FILE_CHOOSER, WlBtFileChooserClass))
typedef struct _WlBtFileChooser WlBtFileChooser;
typedef struct _WlBtFileChooserClass WlBtFileChooserClass;


struct _WlBtFileChooser {
	GtkBuilder parent;
	/*Private */
	GtkWidget *window;
	tr_torrent *torrent;
	tr_ctor *ctor;
	gchar *default_path;		/* 默认的下载位置 */
	GMainLoop *loop;
};

struct _WlBtFileChooserClass {
	GtkBuilderClass parent_class;
	/*Private */
};

GType wl_bt_file_chooser_get_type(void) G_GNUC_CONST;

/* Public */
/*
 * @description 创建WlBtFileChooser对象
 */
WlBtFileChooser *wl_bt_file_chooser_new(tr_ctor * ctor);

/*
 * @description 设置默认的下载位置
 */
void wl_bt_file_chooser_set_download_path(WlBtFileChooser * chooser,
										  const gchar * path);
/*
 * @description 显示选择文件对话框
 * @param path 种子文件路径
 * @return 返回有效的tr_torrent，取消返回NULL
 */
tr_torrent *wl_bt_file_chooser_run(WlBtFileChooser * chooser,
								   const gchar * path);

/*
 * @description 获取种子的保存路径
 *				调用者应该自己清楚什么时候调用下面这些函数才是有效的
 */
const gchar *wl_bt_file_chooser_get_path(WlBtFileChooser * chooser);
/*
 * @description 获取种子元数据文件的路径
 */
const gchar *wl_bt_file_chooser_get_torrent_file(WlBtFileChooser *
												 chooser);
/*
 * @description 是否直接开始下载
 */
gboolean wl_bt_file_chooser_auto_start(WlBtFileChooser * chooser);


G_END_DECLS
#endif
