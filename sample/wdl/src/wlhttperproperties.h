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
#ifndef __WL_HTTPER_PROPERTIES_H__
#define __WL_HTTPER_PROPERTIES_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
#define WL_TYPE_HTTPER_PROPERTIES	(wl_httper_properties_get_type())
#define WL_HTTPER_PROPERTIES(obj) (G_TYPE_CHECK_INSTANCE_CAST(\
			(obj),WL_TYPE_HTTPER_PROPERTIES,WlHttperProperties))
#define WL_IS_HTTPER_PROPERTIES(obj) (G_TYPE_CHECK_INSTANCE_TYPE(\
			(obj),WL_TYPE_HTTPER_PROPERTIES))
#define WL_HTTPER_PROPERTIES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST(\
			(klass),WL_TYPE_HTTPER_PROPERTIES,WlHttperPropertiesClass))
#define WL_IS_HTTPER_PROPERTIES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE(\
			(klass),WL_TYPE_HTTPER_PROPERTIES))
#define WL_HTTPER_PROPERTIES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_TYPE(\
			(obj),WL_TYPE_HTTPER_PROPERTIES,WlHttperPropertiesClass))
typedef struct _WlHttperProperties WlHttperProperties;
typedef struct _WlHttperPropertiesClass WlHttperPropertiesClass;

struct _WlHttperProperties {
	GtkDialog parent;
	GtkWidget *urlLabel;
	GtkWidget *locationLabel;
};

struct _WlHttperPropertiesClass {
	GtkDialogClass parentClass;
};

GType wl_httper_properties_get_type(void) G_GNUC_CONST;

/*************************************************
 * PUBLIC
 ***********************************************/
WlHttperProperties *wl_httper_properties_new(void);
/*
 * @descriptioin 显示窗口
 */
void wl_httper_properties_show(WlHttperProperties * dialog);
/*
 * @description 设置URL和文件保存路径
 */
void wl_httper_properties_set_url(WlHttperProperties * dialog,
								  const gchar * url);
void wl_httper_properties_set_location(WlHttperProperties * dialog,
									   const gchar * path);
void wl_httper_properties_set_title(WlHttperProperties * dialog,
									const gchar * title);

G_END_DECLS						/* __WL_HTTPER_PROPERTIES_H__ */
#endif
