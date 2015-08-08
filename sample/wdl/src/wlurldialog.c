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
#include "wlurldialog.h"

G_DEFINE_TYPE(WlUrlDialog, wl_url_dialog, GTK_TYPE_DIALOG);

typedef void (*CheckUrlCallback) (const gchar * url, gboolean valid,
								  gpointer data);

static void wl_url_dialog_getter(GObject * object, guint property_id,
								 GValue * value, GParamSpec * ps);
static void wl_url_dialog_setter(GObject * object, guint property_id,
								 const GValue * value, GParamSpec * ps);

static gboolean wl_url_dialog_close(GtkWidget * widget, GdkEvent * event,
									gpointer data);
static void wl_url_dialog_response(GtkDialog * dialog, gint responseId,
								   gpointer data);
static void wl_url_dialog_entry_activate(GtkEntry * entry, gpointer data);
static inline void wl_url_dialog_start_spinner(WlUrlDialog * dialog);
static inline void wl_url_dialog_stop_spinner(WlUrlDialog * dialog);
static inline void wl_url_dialog_set_invalid_url_text(WlUrlDialog *
													  dialog);
static inline void wl_url_dialog_clear_invalid_url_text(WlUrlDialog *
														dialog);
static void wl_url_dialog_check_url(WlUrlDialog * dialog,
									const gchar * url,
									CheckUrlCallback callback,
									gpointer data);
static void wl_url_dialog_check_url_callback(const gchar * url,
											 gboolean valid,
											 gpointer data);
static inline void wl_url_dialog_block_ok(WlUrlDialog * dialog);
static inline void wl_url_dialog_unblock_ok(WlUrlDialog * dialog);

static void wl_url_dialog_init(WlUrlDialog * dialog)
{
	gtk_widget_set_size_request(GTK_WIDGET(dialog), 400, 120);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
						   GTK_STOCK_OK, GTK_RESPONSE_OK,
						   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_window_set_default(GTK_WINDOW(dialog),
						   gtk_dialog_get_widget_for_response(GTK_DIALOG
															  (dialog),
															  GTK_RESPONSE_OK));

	GtkWidget *mainBox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_set_spacing(GTK_BOX(mainBox), 8);

	GtkWidget *urlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(mainBox), urlBox, FALSE, FALSE, 0);
	GtkWidget *urlLabel = gtk_label_new("URL:");
	gtk_box_pack_start(GTK_BOX(urlBox), urlLabel, FALSE, FALSE, 0);
	GtkWidget *urlEntry = gtk_entry_new();
	gtk_entry_set_input_purpose(GTK_ENTRY(urlEntry),
								GTK_INPUT_PURPOSE_URL);
	gtk_entry_set_placeholder_text(GTK_ENTRY(urlEntry),
								   "Ftp,Http or Magnet");
	gtk_box_pack_start(GTK_BOX(urlBox), urlEntry, TRUE, TRUE, 0);

	GtkWidget *locationBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(mainBox), locationBox, FALSE, FALSE, 0);
	GtkWidget *locationLabel = gtk_label_new("Location:");
	gtk_box_pack_start(GTK_BOX(locationBox), locationLabel, FALSE, FALSE,
					   0);
	GtkWidget *fileChooser = gtk_file_chooser_button_new("Save File to",
														 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileChooser),
										g_get_user_special_dir
										(G_USER_DIRECTORY_DOWNLOAD));
	gtk_box_pack_start(GTK_BOX(locationBox), fileChooser, FALSE, FALSE, 0);
	GtkWidget *infoLabel = gtk_label_new("");
	gtk_label_set_use_markup(GTK_LABEL(infoLabel), TRUE);
	gtk_box_pack_start(GTK_BOX(locationBox), infoLabel, TRUE, TRUE, 0);

	GtkWidget *expander = gtk_expander_new("");
	gtk_box_pack_start(GTK_BOX(mainBox), expander, FALSE, FALSE, 0);
	GtkWidget *expanderBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_container_add(GTK_CONTAINER(expander), expanderBox);
	GtkWidget *filenameLabel = gtk_label_new("Filename:");
	gtk_box_pack_start(GTK_BOX(expanderBox), filenameLabel, FALSE, FALSE,
					   0);
	GtkWidget *filenameEntry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(filenameEntry),
								   "Default Filename");
	gtk_box_pack_start(GTK_BOX(expanderBox), filenameEntry, FALSE, FALSE,
					   0);

	g_signal_connect(G_OBJECT(urlEntry), "activate",
					 G_CALLBACK(wl_url_dialog_entry_activate), dialog);
	g_signal_connect(G_OBJECT(dialog), "delete-event",
					 G_CALLBACK(wl_url_dialog_close), NULL);
	g_signal_connect(G_OBJECT(dialog), "response",
					 G_CALLBACK(wl_url_dialog_response), NULL);

	gtk_widget_show_all(mainBox);

	dialog->urlEntry = urlEntry;
	dialog->fileChooser = fileChooser;
	dialog->filenameEntry = filenameEntry;
	dialog->expander = expander;
	dialog->infoLabel = infoLabel;
	dialog->urlBox = urlBox;
	dialog->spinner = NULL;
	dialog->threadCounter = 0;
}

static void wl_url_dialog_finalize(GObject * object)
{
	WlUrlDialog *dialog = WL_URL_DIALOG(object);
}

static void wl_url_dialog_class_init(WlUrlDialogClass * klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_url_dialog_getter;
	objClass->set_property = wl_url_dialog_setter;
}

static void wl_url_dialog_getter(GObject * object, guint property_id,
								 GValue * value, GParamSpec * ps)
{
	WlUrlDialog *type = WL_URL_DIALOG(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_url_dialog_setter(GObject * object, guint property_id,
								 const GValue * value, GParamSpec * ps)
{
	WlUrlDialog *type = WL_URL_DIALOG(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static inline void wl_url_dialog_block_ok(WlUrlDialog * dialog)
{
	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog),
									  GTK_RESPONSE_OK, FALSE);
}

static inline void wl_url_dialog_unblock_ok(WlUrlDialog * dialog)
{
	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog),
									  GTK_RESPONSE_OK, TRUE);
}

static gboolean wl_url_dialog_close(GtkWidget * widget, GdkEvent * event,
									gpointer data)
{
	gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_DELETE_EVENT);
	return TRUE;
}

static void wl_url_dialog_check_url_callback(const gchar * url,
											 gboolean valid, gpointer data)
{
	WlUrlDialog *dialog = (WlUrlDialog *) data;
	wl_url_dialog_stop_spinner(dialog);
	wl_url_dialog_unblock_ok(dialog);
	if (valid) {
		wl_url_dialog_clear_invalid_url_text(dialog);
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
	} else {
		wl_url_dialog_set_invalid_url_text(dialog);
	}
}

static void wl_url_dialog_response(GtkDialog * dialog, gint responseId,
								   gpointer data)
{
	WlUrlDialog *urlDialog = WL_URL_DIALOG(dialog);
	if (responseId == GTK_RESPONSE_OK) {
		const gchar *url = wl_url_dialog_get_url(urlDialog);
		if (url == NULL || url[0] == '\0') {
			wl_url_dialog_set_invalid_url_text(urlDialog);
		} else {
			wl_url_dialog_start_spinner(urlDialog);
			wl_url_dialog_clear_invalid_url_text(urlDialog);
			wl_url_dialog_check_url(urlDialog, url,
									wl_url_dialog_check_url_callback,
									urlDialog);
			wl_url_dialog_block_ok(urlDialog);
		}
	} else {					/* 取消了或者成功返回 */
		gtk_widget_hide(GTK_WIDGET(dialog));
		urlDialog->threadCounter++;
	}
}

static void wl_url_dialog_set_invalid_url_text(WlUrlDialog * dialog)
{
	gtk_label_set_markup(GTK_LABEL(dialog->infoLabel),
						 "<span foreground=\"red\">Invalid URL</span>");
}

static void wl_url_dialog_clear_invalid_url_text(WlUrlDialog * dialog)
{
	gtk_label_set_markup(GTK_LABEL(dialog->infoLabel), "<span></span>");
}

static void wl_url_dialog_entry_activate(GtkEntry * entry, gpointer data)
{
	gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_OK);
}

static void wl_url_dialog_start_spinner(WlUrlDialog * dialog)
{
	GtkWidget *spinner = gtk_spinner_new();
	gtk_box_pack_start(GTK_BOX(dialog->urlBox), spinner, FALSE, FALSE, 0);
	gtk_spinner_start(GTK_SPINNER(spinner));
	gtk_widget_show(spinner);

	dialog->spinner = spinner;
}

static void wl_url_dialog_stop_spinner(WlUrlDialog * dialog)
{
	if (dialog->spinner) {
		gtk_container_remove(GTK_CONTAINER(dialog->urlBox),
							 dialog->spinner);
		dialog->spinner = NULL;
	}
}

/*
 * 异步检查URL是否有效
 */
typedef struct {
	CheckUrlCallback callback;
	gpointer data;
	gchar *url;
	WlUrlDialog *dialog;
} CheckUrlCallbackData;

static void check_url_callback_wrapper(GObject * object,
									   GAsyncResult * res, gpointer data)
{
	CheckUrlCallbackData *cdata = (CheckUrlCallbackData *) data;
	CheckUrlCallback callback = cdata->callback;
	gpointer userData = cdata->data;
	gchar *url = cdata->url;
	gint code = g_task_propagate_int(G_TASK(res), NULL);
	if (code < 0)
		return;
	callback(url, code == CURLE_WRITE_ERROR, userData);
}

static gsize curl_write_function(char *ptr, gsize size,
								 gsize nmemb, void *data)
{
	return 0;
}


static void check_url_thread(GTask * task, gpointer object,
							 gpointer data, GCancellable * cancellable)
{
	CheckUrlCallbackData *cdata = data;
	gchar *url = cdata->url;
	WlUrlDialog *dialog = cdata->dialog;
	guint counter = dialog->threadCounter;

	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);

	CURLcode ret = curl_easy_perform(curl);
	glong code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	curl_easy_cleanup(curl);
	if (code >= 400)
		ret = CURLE_WRITE_ERROR;
	if (counter != dialog->threadCounter) {
		g_task_return_int(task, -2);
		return;
	}
	g_task_return_int(task, ret);
}

static void check_url_task_destroy(gpointer data)
{
	CheckUrlCallbackData *cdata = data;
	g_free(cdata->url);
	g_free(cdata);
}

static void wl_url_dialog_check_url(WlUrlDialog * dialog,
									const gchar * url,
									CheckUrlCallback callback,
									gpointer data)
{
	CheckUrlCallbackData *cdata = g_malloc(sizeof(CheckUrlCallbackData));
	cdata->callback = callback;
	cdata->data = data;
	cdata->url = g_strdup(url);
	cdata->dialog = dialog;
	GTask *task = g_task_new(dialog, NULL,
							 check_url_callback_wrapper, cdata);
	g_task_set_check_cancellable(task, TRUE);
	g_task_set_task_data(task, cdata, check_url_task_destroy);
	g_task_set_return_on_cancel(task, TRUE);
	g_task_run_in_thread(task, check_url_thread);
	g_object_unref(task);
}

/*******************************************************
 * PUBLIC
 ******************************************************/
WlUrlDialog *wl_url_dialog_new()
{
	WlUrlDialog *dialog = g_object_new(WL_TYPE_URL_DIALOG, NULL);
	return dialog;
}

gint wl_url_dialog_run(WlUrlDialog * dialog,
					   const gchar * url, const gchar * path)
{
	g_return_val_if_fail(WL_IS_URL_DIALOG(dialog), GTK_RESPONSE_CANCEL);
	if (url)
		gtk_entry_set_text(GTK_ENTRY(dialog->urlEntry), url);
	else
		gtk_entry_set_text(GTK_ENTRY(dialog->urlEntry), "");
	if (path)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER
											(dialog->fileChooser), path);
	else
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER
											(dialog->fileChooser),
											g_get_user_special_dir
											(G_USER_DIRECTORY_DOWNLOAD));

	gtk_entry_set_text(GTK_ENTRY(dialog->filenameEntry), "");
	gtk_expander_set_expanded(GTK_EXPANDER(dialog->expander), FALSE);
	gtk_window_resize(GTK_WINDOW(dialog), 400, 120);
	wl_url_dialog_clear_invalid_url_text(dialog);
	wl_url_dialog_stop_spinner(dialog);
	wl_url_dialog_unblock_ok(dialog);
	dialog->threadCounter++;

	gint response;
	do {
		response = gtk_dialog_run(GTK_DIALOG(dialog));
	} while (response == GTK_RESPONSE_OK);

	return response;
}

const gchar *wl_url_dialog_get_url(WlUrlDialog * dialog)
{
	g_return_val_if_fail(WL_IS_URL_DIALOG(dialog), NULL);
	return gtk_entry_get_text(GTK_ENTRY(dialog->urlEntry));
}

const gchar *wl_url_dialog_get_folder(WlUrlDialog * dialog)
{
	g_return_val_if_fail(WL_IS_URL_DIALOG(dialog), NULL);
	return
		gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER
											(dialog->fileChooser));
}

const gchar *wl_url_dialog_get_filename(WlUrlDialog * dialog)
{
	g_return_val_if_fail(WL_IS_URL_DIALOG(dialog), NULL);
	return gtk_entry_get_text(GTK_ENTRY(dialog->filenameEntry));
}
