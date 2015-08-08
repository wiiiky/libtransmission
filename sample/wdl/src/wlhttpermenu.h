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
#ifndef __WL_HTTPER_MENU_H__
#define __WL_HTTPER_MENU_H__

#include "wlhttper.h"
#include "wlhttperproperties.h"

G_BEGIN_DECLS
#define WL_TYPE_HTTPER_MENU	(wl_httper_menu_get_type())
#define WL_HTTPER_MENU(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj),\
				WL_TYPE_HTTPER_MENU,WlHttperMenu))
#define WL_IS_HTTPER_MENU(obj)	(G_TYPE_CHECK_INSTANCE_TYPE((obj),\
				WL_TYPE_HTTPER_MENU))
#define WL_HTTPER_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),\
				WL_TYPE_HTTPER_MENU,WlHttperMenuClass))
#define WL_IS_HTTPER_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE(\
				(klass),WL_TYPE_HTTPER_MENU))
#define WL_HTTPER_MENU_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_TYPE(\
				(obj),WL_TYPE_HTTPER_MENU,WlHttperMenuClass))
typedef struct _WlHttperMenu WlHttperMenu;
typedef struct _WlHttperMenuClass WlHttperMenuClass;


struct _WlHttperMenu {
	GtkMenu parent;
	GtkWidget *properties;
	GtkWidget *openDir;
	GtkWidget *copyURL;
	GtkWidget *startAction;
	GtkWidget *pauseAction;
	GtkWidget *abortAction;
	GtkWidget *redlAction;
	WlHttper *httper;
	/*GtkWidget *propertiesDialog; */
	WlHttperProperties *propertiesDialog;
};

struct _WlHttperMenuClass {
	GtkMenuClass parentClass;
};

GType wl_httper_menu_get_type(void) G_GNUC_CONST;

/************************************************
 * PUBLIC
 **************************************************/
GtkWidget *wl_httper_menu_new(WlHttper * httper);
/*
 * @description 添加一个item
 * @param item 要添加的GtkMenuItem
 */
void wl_httper_menu_append(WlHttperMenu * menu, GtkWidget * item);
/*
 * @description 插入一个item
 * @param item 要插入的GtkMenuItem
 * @param pos 插入的位置，0到n-1
 */
void wl_httper_menu_insert(WlHttperMenu * menu, GtkWidget * item,
						   gint pos);
/*
 * @description 添加一个分隔符
 */
void wl_httper_menu_append_separator(WlHttperMenu * menu);
/*
 * @description 在指定位置插入一个分隔符
 * @param pos 插入位置
 */
void wl_httper_menu_insert_separator(WlHttperMenu * menu, gint pos);
/*
 * @description 返回相关的Httper
 */
WlHttper *wl_httper_menu_get_httper(WlHttperMenu * menu);
/*
 * @description 返回WlHttper的详细信息窗口
 */
WlHttperProperties *wl_httper_menu_get_properties(WlHttperMenu * menu);
/*
 * @description 设置WlHttper的详细信息窗口
 */
void wl_httper_menu_set_properties(WlHttperMenu * menu,
								   WlHttperProperties * dialog);

void wl_httper_menu_set_sensitive(WlHttperMenu * menu);



G_END_DECLS
#endif	/*__WL_HTTPER_MENU_H__ */
