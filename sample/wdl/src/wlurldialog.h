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
#ifndef __WL_URL_DIALOG_H__
#define __WL_URL_DIALOG_H__

#include <gtk/gtk.h>
#include <curl/curl.h>

G_BEGIN_DECLS
#define WL_TYPE_URL_DIALOG	(wl_url_dialog_get_type())
#define WL_URL_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST(\
			(obj),WL_TYPE_URL_DIALOG,WlUrlDialog))
#define WL_IS_URL_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE(\
			(obj),WL_TYPE_URL_DIALOG))
#define WL_URL_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST(\
			(klass),WL_TYPE_URL_DIALOG,WlUrlDialogClass))
#define WL_IS_URL_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE(\
			(klass),WL_TYPE_URL_DIALOG))
#define WL_URL_DIALOG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_TYPE(\
			(obj),WL_TYPE_URL_DIALOG,WlUrlDialogClass))
typedef struct _WlUrlDialog WlUrlDialog;
typedef struct _WlUrlDialogClass WlUrlDialogClass;

struct _WlUrlDialog {
	GtkDialog parent;
	GtkWidget *urlEntry;
	GtkWidget *fileChooser;
	GtkWidget *filenameEntry;
	GtkWidget *expander;
	GtkWidget *infoLabel;
	GtkWidget *urlBox;
	GtkWidget *spinner;
	const gchar *url;
	const gchar *location;
	guint threadCounter;
};

struct _WlUrlDialogClass {
	GtkDialogClass parentClass;
};

GType wl_url_dialog_get_type(void) G_GNUC_CONST;

/*************************************************
 * PUBLIC
 ***********************************************/
WlUrlDialog *wl_url_dialog_new();
/*
 * @description 打开对话框
 * @param url 下载链接，可以为NULL
 * @param path 保存路径，可以为NULL，默认为~/Download
 * @return 返回GTK_RESPONSE_OK表示用户按下确定
 */
gint wl_url_dialog_run(WlUrlDialog * dialog,
					   const gchar * url, const gchar * path);
/*
 * @description 获取URL和保存路径
 */
const gchar *wl_url_dialog_get_url(WlUrlDialog * dialog);
const gchar *wl_url_dialog_get_folder(WlUrlDialog * dialog);
const gchar *wl_url_dialog_get_filename(WlUrlDialog * dialog);

G_END_DECLS						/* __WL_URL_DIALOG_H__ */
#endif
