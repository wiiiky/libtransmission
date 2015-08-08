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

#include "wlhttperproperties.h"

G_DEFINE_TYPE(WlHttperProperties, wl_httper_properties, GTK_TYPE_DIALOG);

static void wl_httper_properties_getter(GObject * object,
										guint property_id, GValue * value,
										GParamSpec * ps);
static void wl_httper_properties_setter(GObject * object,
										guint property_id,
										const GValue * value,
										GParamSpec * ps);

static void on_properties_dialog_response(GtkDialog * dialog,
										  gint responseId, gpointer data);
static gboolean on_properties_dialog_close(GtkWidget * widget,
										   GdkEvent * event,
										   gpointer data);

static void wl_httper_properties_init(WlHttperProperties * prop)
{
	gtk_window_set_title(GTK_WINDOW(prop), "http properties");
	gtk_widget_set_size_request(GTK_WIDGET(prop), 400, 100);
	gtk_dialog_add_button(GTK_DIALOG(prop), GTK_STOCK_CLOSE,
						  GTK_RESPONSE_CLOSE);

	GtkWidget *vBox = gtk_dialog_get_content_area(GTK_DIALOG(prop));
	gtk_container_set_border_width(GTK_CONTAINER(prop), 10);
	gtk_box_set_spacing(GTK_BOX(vBox), 6);

	PangoAttrList *list = pango_attr_list_new();
	pango_attr_list_insert(list, pango_attr_foreground_new(0, 0, 0));

	GtkWidget *urlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(vBox), urlBox, FALSE, FALSE, 0);
	GtkWidget *label = gtk_label_new("URL:");
	GtkWidget *urlLabel = gtk_label_new("http://");
	gtk_widget_set_halign(urlLabel, GTK_ALIGN_START);
	gtk_label_set_selectable(GTK_LABEL(urlLabel), TRUE);
	gtk_label_set_ellipsize(GTK_LABEL(urlLabel), PANGO_ELLIPSIZE_END);
	gtk_label_set_attributes(GTK_LABEL(urlLabel), list);
	gtk_box_pack_start(GTK_BOX(urlBox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(urlBox), urlLabel, TRUE, TRUE, 0);

	GtkWidget *pathBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(vBox), pathBox, FALSE, FALSE, 0);
	label = gtk_label_new("Location:");
	GtkWidget *savePath = gtk_label_new("/");
	gtk_widget_set_halign(savePath, GTK_ALIGN_START);
	gtk_label_set_selectable(GTK_LABEL(savePath), TRUE);
	gtk_label_set_ellipsize(GTK_LABEL(savePath), PANGO_ELLIPSIZE_END);
	gtk_label_set_attributes(GTK_LABEL(savePath), list);
	gtk_box_pack_start(GTK_BOX(pathBox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pathBox), savePath, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(prop), "response",
					 G_CALLBACK(on_properties_dialog_response), NULL);
	g_signal_connect(G_OBJECT(prop), "delete-event",
					 G_CALLBACK(on_properties_dialog_close), NULL);

	pango_attr_list_unref(list);

	prop->urlLabel = urlLabel;
	prop->locationLabel = savePath;
}

static void wl_httper_properties_class_init(WlHttperPropertiesClass *
											klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_httper_properties_getter;
	objClass->set_property = wl_httper_properties_setter;
}

static void wl_httper_properties_getter(GObject * object,
										guint property_id, GValue * value,
										GParamSpec * ps)
{
	WlHttperProperties *type = WL_HTTPER_PROPERTIES(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_httper_properties_setter(GObject * object,
										guint property_id,
										const GValue * value,
										GParamSpec * ps)
{
	WlHttperProperties *type = WL_HTTPER_PROPERTIES(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void on_properties_dialog_response(GtkDialog * dialog,
										  gint responseId, gpointer data)
{
	if (responseId == GTK_RESPONSE_CLOSE)
		gtk_widget_hide(GTK_WIDGET(dialog));
}

static gboolean on_properties_dialog_close(GtkWidget * widget,
										   GdkEvent * event, gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/****************************************************
 * PUBLIC
 ****************************************************/
WlHttperProperties *wl_httper_properties_new(void)
{
	WlHttperProperties *prop =
		g_object_new(WL_TYPE_HTTPER_PROPERTIES, NULL);
	return prop;
}

void wl_httper_properties_show(WlHttperProperties * dialog)
{
	g_return_if_fail(WL_IS_HTTPER_PROPERTIES(dialog));
	gtk_widget_show_all(GTK_WIDGET(dialog));
}

void wl_httper_properties_set_url(WlHttperProperties * dialog,
								  const gchar * url)
{
	g_return_if_fail(WL_IS_HTTPER_PROPERTIES(dialog));
	gtk_label_set_text(GTK_LABEL(dialog->urlLabel), url);
	gtk_widget_set_tooltip_text(dialog->urlLabel, url);
}

void wl_httper_properties_set_location(WlHttperProperties * dialog,
									   const gchar * path)
{
	g_return_if_fail(WL_IS_HTTPER_PROPERTIES(dialog));
	gtk_label_set_text(GTK_LABEL(dialog->locationLabel), path);
	gtk_widget_set_tooltip_text(dialog->locationLabel, path);
}

void wl_httper_properties_set_title(WlHttperProperties * dialog,
									const gchar * title)
{
	g_return_if_fail(WL_IS_HTTPER_PROPERTIES(dialog));
	gtk_window_set_title(GTK_WINDOW(dialog), title);
}
