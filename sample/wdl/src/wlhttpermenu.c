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
#include "wlhttpermenu.h"

enum {
	WL_HTTPER_MENU_PROPERTY_HTTPER = 1,
};

G_DEFINE_TYPE(WlHttperMenu, wl_httper_menu, GTK_TYPE_MENU);

static void wl_httper_menu_getter(GObject * object, guint property_id,
								  GValue * value, GParamSpec * ps);
static void wl_httper_menu_setter(GObject * object, guint property_id,
								  const GValue * value, GParamSpec * ps);

static void on_properties_activate(GtkMenuItem * item, gpointer data);
static void on_open_folder_activate(GtkMenuItem * item, gpointer data);
static void on_copy_url_activate(GtkMenuItem * item, gpointer data);
static void on_httper_status_changed(WlHttper * httper, gpointer data);
static void on_start_action_activate(GtkMenuItem * item, gpointer data);
static void on_pause_action_activate(GtkMenuItem * item, gpointer data);
static void on_abort_action_activate(GtkMenuItem * item, gpointer data);
static void on_redl_action_activate(GtkMenuItem * item, gpointer data);
static WlHttperProperties *wl_httper_properties_dialog(WlHttperMenu *
													   menu);

static void wl_httper_menu_init(WlHttperMenu * menu)
{
	GtkWidget *properties =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(properties), "Properties");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (properties), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), properties);
	GtkWidget *openDir =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(openDir), "Open Folder");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(openDir),
											  TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), openDir);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu),
						  gtk_separator_menu_item_new());

	GtkWidget *startAction =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PLAY, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(startAction), "Start");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (startAction), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), startAction);
	GtkWidget *pauseAction =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PAUSE, NULL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (pauseAction), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), pauseAction);
	GtkWidget *abortAction =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_STOP, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(abortAction), "Abort");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (abortAction), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), abortAction);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu),
						  gtk_separator_menu_item_new());

	GtkWidget *redlAction = gtk_menu_item_new_with_label("Re-Download");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), redlAction);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu),
						  gtk_separator_menu_item_new());

	GtkWidget *copyURL = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY,
															NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(copyURL),
							"Copy Download URL to Clipboard");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (copyURL), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), copyURL);

	gtk_widget_show_all(GTK_WIDGET(menu));

	menu->httper = NULL;
	menu->properties = properties;
	menu->openDir = openDir;
	menu->copyURL = copyURL;
	menu->startAction = startAction;
	menu->pauseAction = pauseAction;
	menu->abortAction = abortAction;
	menu->redlAction = redlAction;
	menu->propertiesDialog = NULL;
}

static void wl_httper_menu_finalize(GObject * object)
{
	WlHttperMenu *menu = (WlHttperMenu *) object;
	if (menu->httper)
		wl_httper_set_popmenu(menu->httper, NULL);
	if (menu->propertiesDialog)
		gtk_widget_destroy(GTK_WIDGET(menu->propertiesDialog));
}

static void wl_httper_menu_class_init(WlHttperMenuClass * klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_httper_menu_getter;
	objClass->set_property = wl_httper_menu_setter;
	objClass->finalize = wl_httper_menu_finalize;

	GParamSpec *ps;
	ps = g_param_spec_pointer("httper",
							  "the httper",
							  "The Httper",
							  G_PARAM_READABLE | G_PARAM_WRITABLE |
							  G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(objClass,
									WL_HTTPER_MENU_PROPERTY_HTTPER, ps);
}

static void wl_httper_menu_getter(GObject * object, guint property_id,
								  GValue * value, GParamSpec * ps)
{
	WlHttperMenu *menu = WL_HTTPER_MENU(object);
	switch (property_id) {
	case WL_HTTPER_MENU_PROPERTY_HTTPER:
		g_value_set_pointer(value, menu->httper);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_httper_menu_setter(GObject * object, guint property_id,
								  const GValue * value, GParamSpec * ps)
{
	WlHttperMenu *menu = WL_HTTPER_MENU(object);
	WlHttper *httper;
	switch (property_id) {
	case WL_HTTPER_MENU_PROPERTY_HTTPER:
		httper = g_value_get_pointer(value);
		menu->httper = httper;
		wl_httper_set_popmenu(httper, GTK_WIDGET(menu));
		g_signal_connect(G_OBJECT(menu->properties), "activate",
						 G_CALLBACK(on_properties_activate), menu);
		g_signal_connect(G_OBJECT(menu->openDir), "activate",
						 G_CALLBACK(on_open_folder_activate), httper);
		g_signal_connect(G_OBJECT(menu->copyURL), "activate",
						 G_CALLBACK(on_copy_url_activate), httper);
		g_signal_connect(G_OBJECT(menu->startAction), "activate",
						 G_CALLBACK(on_start_action_activate), menu);
		g_signal_connect(G_OBJECT(menu->pauseAction), "activate",
						 G_CALLBACK(on_pause_action_activate), menu);
		g_signal_connect(G_OBJECT(menu->abortAction), "activate",
						 G_CALLBACK(on_abort_action_activate), menu);
		g_signal_connect(G_OBJECT(menu->redlAction), "activate",
						 G_CALLBACK(on_redl_action_activate), menu);
		wl_httper_menu_set_sensitive(menu);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void on_open_folder_activate(GtkMenuItem * item, gpointer data)
{
	WlHttper *httper = (WlHttper *) data;
	wl_httper_launch_path(httper);
}

static void on_copy_url_activate(GtkMenuItem * item, gpointer data)
{
	WlHttper *httper = (WlHttper *) data;
	const gchar *url = wl_httper_get_url(httper);
	if (url == NULL)
		return;
	GtkClipboard *cb = gtk_widget_get_clipboard(GTK_WIDGET(httper),
												GDK_SELECTION_CLIPBOARD);
	if (cb) {
		gtk_clipboard_set_text(cb, url, -1);
	}
}

static WlHttperProperties *wl_httper_properties_dialog(WlHttperMenu * menu)
{
	WlHttperProperties *dialog = wl_httper_menu_get_properties(menu);
	WlHttper *httper = wl_httper_menu_get_httper(menu);
	if (dialog == NULL) {
		dialog = wl_httper_properties_new();
		wl_httper_properties_set_url(dialog, wl_httper_get_url(httper));
		wl_httper_properties_set_location(dialog,
										  wl_httper_get_path(httper));
		wl_httper_properties_set_title(dialog,
									   wl_httper_get_title(httper));
		wl_httper_menu_set_properties(menu, dialog);
	}
	return dialog;
}

static void on_properties_activate(GtkMenuItem * item, gpointer data)
{
	WlHttperMenu *menu = (WlHttperMenu *) data;
	WlHttperProperties *dialog = wl_httper_properties_dialog(menu);
	wl_httper_properties_show(dialog);
}

static void on_start_action_activate(GtkMenuItem * item, gpointer data)
{
	WlHttperMenu *menu = (WlHttperMenu *) data;
	WlHttper *httper = wl_httper_menu_get_httper(menu);
	gint status = wl_httper_get_status(httper);
	if (status == WL_HTTPER_STATUS_PAUSE)
		wl_httper_continue(httper);
	else if (status != WL_HTTPER_STATUS_START &&
			 status != WL_HTTPER_STATUS_COMPLETE)
		wl_httper_start(httper);
	wl_httper_menu_set_sensitive(menu);
}

static void on_pause_action_activate(GtkMenuItem * item, gpointer data)
{
	WlHttperMenu *menu = (WlHttperMenu *) data;
	WlHttper *httper = wl_httper_menu_get_httper(menu);
	gint status = wl_httper_get_status(httper);
	/*if (status == WL_HTTPER_STATUS_START) */
	wl_httper_pause(httper);
	wl_httper_menu_set_sensitive(menu);
}

static void on_abort_action_activate(GtkMenuItem * item, gpointer data)
{
	WlHttperMenu *menu = (WlHttperMenu *) data;
	WlHttper *httper = wl_httper_menu_get_httper(menu);
	wl_httper_abort(httper);
	wl_httper_menu_set_sensitive(menu);
}

static void on_redl_action_activate(GtkMenuItem * item, gpointer data)
{
	WlHttperMenu *menu = (WlHttperMenu *) data;
	WlHttper *httper = wl_httper_menu_get_httper(menu);
	/*gint status = wl_httper_get_status(httper); */
	/*wl_httper_abort(httper); */
	/*wl_httper_start(httper); */
	wl_httper_redownload(httper);
	wl_httper_menu_set_sensitive(menu);
}

static void on_httper_status_changed(WlHttper * httper, gpointer data)
{
	WlHttperMenu *menu = (WlHttperMenu *) data;
	wl_httper_menu_set_sensitive(menu);
}

void wl_httper_menu_set_sensitive(WlHttperMenu * menu)
{
	WlHttper *httper = wl_httper_menu_get_httper(menu);
	gint status = wl_httper_get_status(httper);
	switch (status) {
	case WL_HTTPER_STATUS_COMPLETE:
		gtk_widget_set_sensitive(menu->startAction, FALSE);
		gtk_widget_set_sensitive(menu->pauseAction, FALSE);
		/*gtk_widget_set_sensitive(menu->redlAction, TRUE); */
		break;
		/*case WL_HTTPER_STATUS_NOT_START: */
		/*case WL_HTTPER_STATUS_PAUSE: */
		/*gtk_widget_set_sensitive(menu->startAction,TRUE); */
		/*gtk_widget_set_sensitive(menu->pauseAction,FALSE); */
		/*gtk_widget_set_sensitive(menu->redlAction,FALSE); */
		/*break; */
	case WL_HTTPER_STATUS_START:
		gtk_widget_set_sensitive(menu->startAction, FALSE);
		gtk_widget_set_sensitive(menu->pauseAction, TRUE);
		/*gtk_widget_set_sensitive(menu->redlAction, FALSE); */
		break;
	case WL_HTTPER_STATUS_NOT_START:
	case WL_HTTPER_STATUS_PAUSE:
		gtk_widget_set_sensitive(menu->startAction, TRUE);
		gtk_widget_set_sensitive(menu->pauseAction, FALSE);
		break;
	default:
		gtk_widget_set_sensitive(menu->startAction, FALSE);
		gtk_widget_set_sensitive(menu->pauseAction, FALSE);
		/*gtk_widget_set_sensitive(menu->abortAction,FALSE); */
		/*gtk_widget_set_sensitive(menu->redlAction, FALSE); */
		break;
	}
	if (status == WL_HTTPER_STATUS_START ||
		status == WL_HTTPER_STATUS_PAUSE)
		gtk_widget_set_sensitive(menu->abortAction, TRUE);
	else
		gtk_widget_set_sensitive(menu->abortAction, FALSE);
}

/*************************************************************
 * PUBLIC
 ***********************************************************/
GtkWidget *wl_httper_menu_new(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	WlHttperMenu *menu = g_object_new(WL_TYPE_HTTPER_MENU,
									  "httper", httper, NULL);

	return GTK_WIDGET(menu);
}

void wl_httper_menu_append(WlHttperMenu * menu, GtkWidget * item)
{
	g_return_if_fail(WL_IS_HTTPER_MENU(menu));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_show(item);
}

void wl_httper_menu_insert(WlHttperMenu * menu, GtkWidget * item, gint pos)
{
	g_return_if_fail(WL_IS_HTTPER_MENU(menu));
	gtk_menu_shell_insert(GTK_MENU_SHELL(menu), item, pos);
	gtk_widget_show(item);
}

void wl_httper_menu_append_separator(WlHttperMenu * menu)
{
	g_return_if_fail(WL_IS_HTTPER_MENU(menu));
	GtkWidget *separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_widget_show(separator);
}

void wl_httper_menu_insert_separator(WlHttperMenu * menu, gint pos)
{
	g_return_if_fail(WL_IS_HTTPER_MENU(menu));
	GtkWidget *separator = gtk_separator_menu_item_new();
	gtk_menu_shell_insert(GTK_MENU_SHELL(menu), separator, pos);
	gtk_widget_show(separator);
}

WlHttper *wl_httper_menu_get_httper(WlHttperMenu * menu)
{
	g_return_val_if_fail(WL_IS_HTTPER_MENU(menu), NULL);
	return menu->httper;
}

WlHttperProperties *wl_httper_menu_get_properties(WlHttperMenu * menu)
{
	g_return_val_if_fail(WL_IS_HTTPER_MENU(menu), NULL);
	return menu->propertiesDialog;
}

void wl_httper_menu_set_properties(WlHttperMenu * menu,
								   WlHttperProperties * dialog)
{
	g_return_if_fail(WL_IS_HTTPER_MENU(menu));
	if (menu->propertiesDialog)
		gtk_widget_destroy(GTK_WIDGET(menu->propertiesDialog));
	menu->propertiesDialog = dialog;
}
