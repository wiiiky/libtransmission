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
#include "wldownloader.h"
#include "libtransmission/transmission.h"
#include "libtransmission/variant.h"
#include "wlbtermenu.h"

enum {
	WL_DOWNLOADER_PROPERTY_SPACING = 1,
};

G_DEFINE_TYPE(WlDownloader, wl_downloader, GTK_TYPE_SCROLLED_WINDOW);

static void wl_downloader_get_property(GObject * object, guint property_id,
									   GValue * value, GParamSpec * ps);
static void wl_downloader_set_property(GObject * object, guint property_id,
									   const GValue * value,
									   GParamSpec * ps);

static gpointer wl_downloader_pressed_callback(GtkWidget * widget,
											   GdkEventButton *
											   event, gpointer data);
static inline void wl_downloader_set_selected(WlDownloader * dl,
											  gpointer obj);

static inline void wl_downloader_save_httper(WlDownloader * dl);
static inline void wl_downloader_load_httper(WlDownloader * dl);
static inline void wl_downloader_save_bter(WlDownloader * dl);
static inline void wl_downloader_load_bter(WlDownloader * dl);

static void wl_downloader_init(WlDownloader * dl)
{
	g_object_set(G_OBJECT(dl), "hadjustment", NULL,
				 "vadjustment", NULL, NULL);

	GtkWidget *vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(dl), vBox);

	tr_session *session;
	tr_variant settings;
	const gchar *configDir = tr_getDefaultConfigDir("wdl");
	dl->keyFilePath = g_strdup_printf("%s/http.conf", configDir);
	tr_variantInitDict(&settings, 0);
	if (!tr_sessionLoadSettings(&settings, configDir, "wdl"))
		tr_sessionGetDefaultSettings(&settings);
	session = tr_sessionInit("gtk", configDir, true, &settings);
	tr_variantFree(&settings);

	tr_formatter_speed_init(SPEED_K, SPEED_K_STR, SPEED_M_STR, SPEED_G_STR,
							SPEED_T_STR);

	dl->session = session;

	dl->vBox = vBox;
	dl->list = NULL;
	dl->selected = NULL;
	dl->selectedCB = NULL;
	dl->selectedCBData = NULL;
	dl->httperStatus = NULL;
	dl->httperStatusData = NULL;
	dl->bterStatus = NULL;
	dl->bterStatusData = NULL;
}

static void wl_downloader_finalize(GObject * object)
{
	WlDownloader *dl = WL_DOWNLOADER(object);
	if (dl->list)
		g_list_free(dl->list);
	if (dl->keyFilePath)
		g_free(dl->keyFilePath);
	tr_sessionClose(dl->session);
}

static void wl_downloader_class_init(WlDownloaderClass * klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_downloader_get_property;
	objClass->set_property = wl_downloader_set_property;
	objClass->finalize = wl_downloader_finalize;

	GParamSpec *ps;
	ps = g_param_spec_uint("spacing",
						   "box spacing",
						   "Box Spacing",
						   0, G_MAXUINT,
						   0, G_PARAM_READABLE | G_PARAM_WRITABLE);
	g_object_class_install_property(objClass,
									WL_DOWNLOADER_PROPERTY_SPACING, ps);
}

static void wl_downloader_get_property(GObject * object, guint property_id,
									   GValue * value, GParamSpec * ps)
{
	WlDownloader *dl = WL_DOWNLOADER(object);
	switch (property_id) {
	case WL_DOWNLOADER_PROPERTY_SPACING:
		g_value_set_uint(value, gtk_box_get_spacing(GTK_BOX(dl->vBox)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_downloader_set_property(GObject * object, guint property_id,
									   const GValue * value,
									   GParamSpec * ps)
{
	WlDownloader *dl = WL_DOWNLOADER(object);
	switch (property_id) {
	case WL_DOWNLOADER_PROPERTY_SPACING:
		gtk_box_set_spacing(GTK_BOX(dl->vBox), g_value_get_uint(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static inline void wl_downloader_set_selected(WlDownloader * dl,
											  gpointer obj)
{
	if (dl->selected == obj)
		return;
	if (dl->selected) {
		if (WL_IS_HTTPER(dl->selected)) {
			wl_httper_clear_highlight(WL_HTTPER(dl->selected));
			wl_httper_set_status_callback(WL_HTTPER(dl->selected), NULL,
										  NULL);
		} else if (WL_IS_BTER(dl->selected)) {
			wl_bter_clear_highlight(WL_BTER(dl->selected));
			wl_bter_set_status_callback(WL_BTER(dl->selected), NULL, NULL);
		}
	}
	dl->selected = NULL;
	if (obj == NULL) {
		goto CALLBACK;
	} else if (WL_IS_HTTPER(obj)) {
		wl_httper_highlight(WL_HTTPER(obj));
		wl_httper_set_status_callback(WL_HTTPER(obj), dl->httperStatus,
									  dl->httperStatusData);
	} else if (WL_IS_BTER(obj)) {
		wl_bter_highlight(WL_BTER(obj));
		wl_bter_set_status_callback(WL_BTER(obj), dl->bterStatus,
									dl->bterStatusData);
	} else
		return;					/* unknown type */

	dl->selected = obj;
  CALLBACK:
	if (dl->selectedCB)
		dl->selectedCB(dl, dl->selectedCBData);
}

static void on_remove_httper_activate(GtkMenuItem * item, gpointer data)
{
	WlHttper *httper = (WlHttper *) data;
	WlDownloader *dl = wl_httper_get_user_data(httper);
	wl_downloader_remove_httper(dl, httper);
}

static void on_delete_files_activate(GtkMenuItem * item, gpointer data)
{
	WlHttper *httper = (WlHttper *) data;
	WlDownloader *dl = wl_httper_get_user_data(httper);

	const gchar *path = wl_httper_get_path(httper);

	GtkWidget *dialog = gtk_message_dialog_new(NULL,
											   GTK_DIALOG_MODAL |
											   GTK_DIALOG_DESTROY_WITH_PARENT,
											   GTK_MESSAGE_INFO,
											   GTK_BUTTONS_OK_CANCEL,
											   "Delete files!");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
											 "This operation will delete files from file system");
	gint res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if (res != GTK_RESPONSE_OK)
		return;
	/* 先暂停 */
	wl_httper_pause(httper);
	if (path) {
		GFile *file = g_file_new_for_path(path);
		/* 将文件放入回收站或者彻底删除 */
		if (g_file_trash(file, NULL, NULL) == FALSE) {
			g_file_delete(file, NULL, NULL);
		}
		g_object_unref(file);
	}
	wl_downloader_remove_httper(dl, httper);
}

static inline GtkWidget *wl_downloader_httper_popmenu(WlHttper * httper)
{
	GtkWidget *menu = wl_httper_menu_new(httper);
	wl_httper_menu_append_separator(WL_HTTPER_MENU(menu));

	GtkWidget *rmHttper =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_REMOVE, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(rmHttper), "Remove");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (rmHttper), TRUE);
	g_signal_connect(G_OBJECT(rmHttper), "activate",
					 G_CALLBACK(on_remove_httper_activate), httper);
	wl_httper_menu_append(WL_HTTPER_MENU(menu), rmHttper);

	GtkWidget *dFile =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(dFile),
							"Remove and Delete Files");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (dFile), TRUE);
	g_signal_connect(G_OBJECT(dFile), "activate",
					 G_CALLBACK(on_delete_files_activate), httper);
	wl_httper_menu_append(WL_HTTPER_MENU(menu), dFile);
	return menu;
}

static void on_remove_bter_activate(GtkMenuItem * item, gpointer data)
{
	WlBter *bter = (WlBter *) data;
	WlDownloader *dl = wl_bter_get_user_data(bter);
	wl_downloader_remove_bter(dl, bter, FALSE);
}

static void on_delete_bter_activate(GtkMenuItem * item, gpointer data)
{
	WlBter *bter = (WlBter *) data;
	WlDownloader *dl = wl_bter_get_user_data(bter);
	GtkWidget *dialog = gtk_message_dialog_new(NULL,
											   GTK_DIALOG_MODAL |
											   GTK_DIALOG_DESTROY_WITH_PARENT,
											   GTK_MESSAGE_INFO,
											   GTK_BUTTONS_OK_CANCEL,
											   "Delete files!");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
											 "This operation will delete files from file system");
	gint res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if (res != GTK_RESPONSE_OK)
		return;

	wl_downloader_remove_bter(dl, bter, TRUE);
}

static inline GtkWidget *wl_downloader_bter_popmenu(WlBter * bter)
{
	WlBterMenu *menu = wl_bter_menu_new(bter);
	wl_bter_menu_append_separator(menu);

	GtkWidget *rmHttper =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_REMOVE, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(rmHttper), "Remove");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (rmHttper), TRUE);
	g_signal_connect(G_OBJECT(rmHttper), "activate",
					 G_CALLBACK(on_remove_bter_activate), bter);
	wl_bter_menu_append(menu, rmHttper);

	GtkWidget *dFile =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(dFile),
							"Remove and Delete Files");
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
											  (dFile), TRUE);
	g_signal_connect(G_OBJECT(dFile), "activate",
					 G_CALLBACK(on_delete_bter_activate), bter);
	wl_bter_menu_append(menu, dFile);

	gtk_widget_show_all(GTK_WIDGET(menu));

	return GTK_WIDGET(menu);
}

static gpointer wl_downloader_pressed_callback(GtkWidget * widget,
											   GdkEventButton *
											   event, gpointer data)
{
	WlDownloader *dl = (WlDownloader *) data;
	GtkWidget *popmenu;
	if (event->type == GDK_BUTTON_PRESS) {
		/* 单击 */
		wl_downloader_set_selected(dl, widget);
		if (event->button == 1) {
			/* 左键 */
			/*wl_downloader_set_httper_selected(dl, httper); */
		} else if (event->button == 3) {
			/* 右键 */
			if (WL_IS_HTTPER(widget)) {
				WlHttper *httper = WL_HTTPER(widget);
				popmenu = wl_httper_get_popmenu(httper);
				gtk_menu_popup(GTK_MENU(popmenu), NULL, NULL, NULL, NULL,
							   event->button,
							   gdk_event_get_time((GdkEvent *) event));
			} else if (WL_IS_BTER(widget)) {
				WlBter *bter = WL_BTER(widget);
				popmenu = wl_bter_get_popmenu(bter);
				gtk_menu_popup(GTK_MENU(popmenu), NULL, NULL, NULL, NULL,
							   event->button,
							   gdk_event_get_time((GdkEvent *) event));
			}
		}
	} else if (event->type == GDK_2BUTTON_PRESS) {
		/* 双击 */
		if (event->button == 1) {
			/* 左键 */
		} else if (event->button == 3) {
			/* 右键 */
		}
	}
	return FALSE;
}

#define KEY_TITLE   "title"
#define KEY_URL		"url"
#define KEY_PATH	"path"
#define	KEY_TOTAL_SIZE  "total size"
#define KEY_DL_SIZE "download size"
#define KEY_STATUS  "status"

static inline void wl_downloader_save_httper(WlDownloader * dl)
{
	GKeyFile *key_file = g_key_file_new();

	GList *lp = dl->list;
	guint i = 0;
	gchar group[20];
	while (lp) {
		if (WL_IS_HTTPER(lp->data)) {
			WlHttper *httper = lp->data;
			g_snprintf(group, 20, "%u", i++);
			g_key_file_set_string(key_file, group, KEY_TITLE,
								  wl_httper_get_title(httper));
			g_key_file_set_string(key_file, group, KEY_URL,
								  wl_httper_get_url(httper));
			g_key_file_set_string(key_file, group, KEY_PATH,
								  wl_httper_get_path(httper));
			g_key_file_set_uint64(key_file, group, KEY_TOTAL_SIZE,
								  wl_httper_get_total_size(httper));
			g_key_file_set_uint64(key_file, group, KEY_DL_SIZE,
								  wl_httper_get_dl_size(httper));
			g_key_file_set_uint64(key_file, group, KEY_STATUS,
								  wl_httper_get_status(httper));
			/*g_message("%s", g_key_file_to_data(key_file, NULL, NULL));*/

		}
		lp = g_list_next(lp);
	}
	g_file_set_contents(dl->keyFilePath,
						g_key_file_to_data(key_file, NULL, NULL), -1,
						NULL);

	g_key_file_free(key_file);
}

static inline void wl_downloader_load_httper(WlDownloader * dl)
{
	GKeyFile *key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, dl->keyFilePath, G_KEY_FILE_NONE,
							  NULL);
	gchar **groups = g_key_file_get_groups(key_file, NULL);
	if (groups == NULL) {
		g_key_file_unref(key_file);
		return;
	}
	gint i = 0;
	gchar *group = groups[i++];
	while (group) {
		gchar *title =
			g_key_file_get_string(key_file, group, KEY_TITLE, NULL);
		gchar *url = g_key_file_get_string(key_file, group, KEY_URL, NULL);
		gchar *path =
			g_key_file_get_string(key_file, group, KEY_PATH, NULL);
		guint64 total_size =
			g_key_file_get_uint64(key_file, group, KEY_TOTAL_SIZE, NULL);
		guint64 dl_size =
			g_key_file_get_uint64(key_file, group, KEY_DL_SIZE, NULL);
		guint64 status =
			g_key_file_get_uint64(key_file, group, KEY_STATUS, NULL);

		if (title && url && path) {	/* 三个值都必须存在 */
			WlHttper *httper = wl_downloader_append_httper(dl, url, path);
			wl_httper_load(httper, total_size, dl_size, status);
		}

		g_free(title);
		g_free(url);
		g_free(path);
		group = groups[i++];
	}
	g_strfreev(groups);
}

static inline void wl_downloader_save_bter(WlDownloader * dl)
{
}

static inline void wl_downloader_load_bter(WlDownloader * dl)
{
	tr_ctor *ctor = tr_ctorNew(dl->session);
	gint count = 0;
	tr_torrent **tors;
	if (tors = tr_sessionLoadTorrents(dl->session, ctor, &count)) {
		gint i;
		for (i = 0; i < count; i++) {
			const tr_info *info = tr_torrentInfo(tors[i]);
			wl_downloader_append_bter(dl, tors[i]);
			/*g_message("%s", info->originalName);*/
		}
	}
	tr_ctorFree(ctor);
}

/**************************************************
 * PUBLIC
 ***************************************************/
WlDownloader *wl_downloader_new(void)
{
	WlDownloader *dl =
		(WlDownloader *) g_object_new(WL_TYPE_DOWNLOADER, NULL);
	return dl;
}

WlHttper *wl_downloader_append_httper(WlDownloader * dl, const gchar * url,
									  const gchar * path)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	WlHttper *httper = wl_httper_new(url, path);
	GtkWidget *menu = wl_downloader_httper_popmenu(httper);
	wl_httper_set_user_data(httper, dl);
	/*wl_httper_start (httper); */
	g_signal_connect(G_OBJECT(httper), "button-press-event",
					 G_CALLBACK(wl_downloader_pressed_callback), dl);

	gtk_box_pack_start(GTK_BOX(dl->vBox), GTK_WIDGET(httper), FALSE, FALSE,
					   0);
	dl->list = g_list_append(dl->list, httper);
	gtk_widget_show_all(GTK_WIDGET(httper));
	return httper;
}

void wl_downloader_remove_httper(WlDownloader * dl, WlHttper * httper)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl) && WL_IS_HTTPER(httper));
	dl->list = g_list_remove(dl->list, httper);
	wl_downloader_set_selected(dl, NULL);
	wl_httper_abort(httper);
	gtk_container_remove(GTK_CONTAINER(dl->vBox), GTK_WIDGET(httper));
}

void wl_downloader_remove_bter(WlDownloader * dl, WlBter * bter,
							   gboolean local)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl) && WL_IS_BTER(bter));
	dl->list = g_list_remove(dl->list, bter);
	wl_downloader_set_selected(dl, NULL);
	tr_torrentRemove(wl_bter_get_torrent(bter), local, NULL);
	gtk_container_remove(GTK_CONTAINER(dl->vBox), GTK_WIDGET(bter));
}

WlBter *wl_downloader_append_bter(WlDownloader * dl, tr_torrent * torrent)
{
	g_return_val_if_fail(WL_IS_DOWNLOADER(dl), NULL);
	WlBter *bter = wl_bter_new(dl->session, torrent);

	if (bter == NULL)
		return NULL;

	//wl_bter_menu_new(bter);
	wl_downloader_bter_popmenu(bter);
	wl_bter_set_user_data(bter, dl);
	wl_bter_start(bter);

	g_signal_connect(G_OBJECT(bter), "button-press-event",
					 G_CALLBACK(wl_downloader_pressed_callback), dl);

	gtk_box_pack_start(GTK_BOX(dl->vBox), GTK_WIDGET(bter), FALSE, FALSE,
					   0);
	dl->list = g_list_append(dl->list, bter);
	gtk_widget_show_all(GTK_WIDGET(bter));
	return bter;
}

WlBter *wl_downloader_append_bter_from_file(WlDownloader * dl,
											const gchar * path)
{
	g_return_val_if_fail(WL_IS_DOWNLOADER(dl), NULL);
	WlBter *bter = wl_bter_new_from_file(dl->session, path);

	if (bter == NULL)
		return NULL;

	g_signal_connect(G_OBJECT(bter), "button-press-event",
					 G_CALLBACK(wl_downloader_pressed_callback), dl);

	gtk_box_pack_start(GTK_BOX(dl->vBox), GTK_WIDGET(bter), FALSE, FALSE,
					   0);
	dl->list = g_list_append(dl->list, bter);
	gtk_widget_show_all(GTK_WIDGET(bter));
	return bter;
}

void wl_downloader_start_selected(WlDownloader * dl)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	if (dl->selected == NULL)
		return;
	if (WL_IS_HTTPER(dl->selected)) {
		WlHttper *httper = WL_HTTPER(dl->selected);
		wl_httper_start(httper);
	} else if (WL_IS_BTER(dl->selected)) {
		wl_bter_start(WL_BTER(dl->selected));
	}
}

void wl_downloader_pause_selected(WlDownloader * dl)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	if (dl->selected == NULL)
		return;
	if (WL_IS_HTTPER(dl->selected)) {
		WlHttper *httper = WL_HTTPER(dl->selected);
		wl_httper_pause(httper);
	} else if (WL_IS_BTER(dl->selected)) {
		wl_bter_pause(WL_BTER(dl->selected));
	}
}

void wl_downloader_continue_selected(WlDownloader * dl)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	if (dl->selected == NULL)
		return;
	if (WL_IS_HTTPER(dl->selected)) {
		WlHttper *httper = WL_HTTPER(dl->selected);
		wl_httper_continue(httper);
	} else if (WL_IS_BTER(dl->selected)) {
		wl_bter_continue(WL_BTER(dl->selected));
	}
}

gint wl_downloader_get_selected_status(WlDownloader * dl)
{
	g_return_val_if_fail(WL_IS_DOWNLOADER(dl), 0);
	if (dl->selected == NULL)
		return 0;
	if (WL_IS_HTTPER(dl->selected))
		return wl_httper_get_status(WL_HTTPER(dl->selected));
	else if (WL_IS_BTER(dl->selected))
		return wl_bter_get_status(WL_BTER(dl->selected));
	return 0;
}

gpointer wl_downloader_get_selected(WlDownloader * dl)
{
	g_return_val_if_fail(WL_IS_DOWNLOADER(dl), NULL);
	return dl->selected;
}

void wl_downloader_remove_selected(WlDownloader * dl, gboolean local)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	if (dl->selected == NULL)
		return;
	if (WL_IS_HTTPER(dl->selected)) {
		wl_downloader_remove_httper(dl, WL_HTTPER(dl->selected));
	} else if (WL_IS_BTER(dl->selected)) {
		wl_downloader_remove_bter(dl, WL_BTER(dl->selected), local);
	}
	wl_downloader_set_selected(dl, NULL);
}

void wl_downloader_set_selected_callback(WlDownloader * dl,
										 WlDownloaderSelectedCallback
										 callback, gpointer data)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	dl->selectedCB = callback;
	dl->selectedCBData = data;
}

void wl_downloader_set_httper_status_callback(WlDownloader * dl,
											  WlHttperStatusCallback
											  callback, gpointer data)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	dl->httperStatus = callback;
	dl->httperStatusData = data;
}

void wl_downloader_set_bter_status_callback(WlDownloader * dl,
											WlBterStatusCallback callback,
											gpointer data)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	dl->bterStatus = callback;
	dl->bterStatusData = data;
}

tr_torrent *wl_downloader_create_torrent(WlDownloader * dl,
										 const gchar * path)
{
	g_return_val_if_fail(WL_IS_DOWNLOADER(dl), NULL);
	tr_ctor *ctor = tr_ctorNew(dl->session);
	tr_ctorSetMetainfoFromFile(ctor, path);
	tr_torrent *torrent = tr_torrentNew(ctor, NULL, NULL);
	tr_ctorFree(ctor);

	return torrent;
}

tr_ctor *wl_downloader_create_ctor(WlDownloader * dl)
{
	g_return_val_if_fail(WL_IS_DOWNLOADER(dl), NULL);
	tr_ctor *ctor = tr_ctorNew(dl->session);
	return ctor;
}

void wl_downloader_save_tasks(WlDownloader * dl)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	wl_downloader_save_httper(dl);
	wl_downloader_save_bter(dl);
}

void wl_downloader_load_tasks(WlDownloader * dl)
{
	g_return_if_fail(WL_IS_DOWNLOADER(dl));
	wl_downloader_load_bter(dl);
	wl_downloader_load_httper(dl);
}
