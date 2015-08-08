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
#ifndef __WL_DOWNLOAD_WINDOW_H__
#define __WL_DOWNLOAD_WINDOW_H__

#include "wldownloader.h"
#include "wlurldialog.h"
#include "wlbtfilechooser.h"

G_BEGIN_DECLS
#define WL_TYPE_DOWNLOAD_WINDOW	(wl_download_window_get_type())
#define WL_DOWNLOAD_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST(\
			(obj),WL_TYPE_DOWNLOAD_WINDOW,WlDownloadWindow))
#define WL_IS_DOWNLOAD_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE(\
			(obj),WL_TYPE_DOWNLOAD_WINDOW))
#define WL_DOWNLOAD_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST(\
			(klass),WL_TYPE_DOWNLOAD_WINDOW,WlDownloadWindowClass))
#define WL_IS_DOWNLOAD_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE(\
			(klass),WL_TYPE_DOWNLOAD_WINDOW))
#define WL_DOWNLOAD_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_TYPE(\
			(obj),WL_TYPE_DOWNLOAD_WINDOW,WlDownloadWindowClass))
typedef struct _WlDownloadWindow WlDownloadWindow;
typedef struct _WlDownloadWindowClass WlDownloadWindowClass;

struct _WlDownloadWindow {
	GtkWindow parent;			/* Parent */

	GtkToolItem *start;
	GtkToolItem *pause;
	GtkToolItem *remove;
	WlDownloader *downloader;
	WlUrlDialog *urlDialog;

	WlBtFileChooser *bf_chooser;
};

struct _WlDownloadWindowClass {
	GtkWindowClass parentClass;
};

GType wl_download_window_get_type(void) G_GNUC_CONST;

/*************************************************
 * PUBLIC
 ***********************************************/
WlDownloadWindow *wl_download_window_new(void);

G_END_DECLS						/* __WL_DOWNLOAD_WINDOW_H__ */
#endif
