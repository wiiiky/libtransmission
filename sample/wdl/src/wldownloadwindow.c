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

#include "wldownloadwindow.h"


#define WL_DL_WINDOW_TITLE  "wdl"
#define WL_DL_WINDOW_WIDTH  (600)
#define WL_DL_WINDOW_HEIGHT (420)

#define WL_ICON_NAME "gnome-terminal"

enum {
	WL_DL_WINDOW_PROPERTY_TITLE = 1,
};

G_DEFINE_TYPE(WlDownloadWindow, wl_download_window, GTK_TYPE_WINDOW);

static void wl_download_window_getter(GObject * object, guint property_id,
									  GValue * value, GParamSpec * ps);
static void wl_download_window_setter(GObject * object, guint property_id,
									  const GValue * value,
									  GParamSpec * ps);

static inline void wl_dl_window_set_start_enabled(WlDownloadWindow *
												  window,
												  gboolean enabled);
static inline void wl_dl_window_set_pause_enabled(WlDownloadWindow *
												  window,
												  gboolean enabled);
static inline void wl_dl_window_set_remove_enabled(WlDownloadWindow *
												   window,
												   gboolean enabled);
static inline void
wl_dl_window_enable_button_by_httper_status(WlDownloadWindow * window,
											WlHttperStatus status);

static void wl_dl_window_open_url(GtkMenuItem * item, gpointer data);
static void wl_dl_window_open_torrent(GtkMenuItem * item, gpointer data);
static void wl_dl_window_show_about_dialog(GtkMenuItem * item,
										   gpointer data);
static void wl_dl_window_start_download(GtkToolButton * button,
										gpointer data);
static void wl_dl_window_pause_download(GtkToolButton * button,
										gpointer data);
static void wl_dl_window_remove_download(GtkToolButton * button,
										 gpointer data);
static void wl_dl_window_downloader_selected_callback(WlDownloader * dl,
													  gpointer data);
static void wl_dl_window_httper_status_callback(WlHttper * httper,
												gpointer data);
static void wl_dl_window_bter_status_callback(WlBter * bter,
											  gpointer data);

static void wl_dl_window_destroy(GtkWidget * window, gpointer data);
static GtkWidget *wl_dl_window_about_dialog(void);
/* 返回TRUE表示覆盖，FALSE则取消 */
static gboolean wl_dl_window_overwrite_dialog(WlDownloadWindow * window,
											  const gchar * path);


static void wl_download_window_init(WlDownloadWindow * window)
{
	gtk_window_set_title(GTK_WINDOW(window), WL_DL_WINDOW_TITLE);
	//gtk_window_set_icon_name (GTK_WINDOW(window),"midori");
	gtk_window_set_default_icon_name(WL_ICON_NAME);
	gtk_widget_set_size_request(GTK_WIDGET(window),
								WL_DL_WINDOW_WIDTH, WL_DL_WINDOW_HEIGHT);
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
	gtk_window_move(GTK_WINDOW(window), gdk_screen_width() / 5,
					gdk_screen_height() / 8);

	GtkWidget *mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), mainBox);

	GtkWidget *menuBar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(mainBox), menuBar, FALSE, FALSE, 0);
	GtkWidget *fileMenuItem = gtk_menu_item_new_with_label("File");
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), fileMenuItem);
	GtkWidget *fileMenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMenuItem), fileMenu);
	GtkWidget *urlMenuItem = gtk_menu_item_new_with_label("Open URL");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), urlMenuItem);
	GtkWidget *btMenuItem = gtk_menu_item_new_with_label("Open Torrent");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), btMenuItem);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu),
						  gtk_separator_menu_item_new());
	GtkWidget *quitMenuItem = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMenuItem);

	GtkWidget *helpMenuItem = gtk_menu_item_new_with_label("Help");
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), helpMenuItem);
	GtkWidget *helpMenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpMenuItem), helpMenu);
	GtkWidget *aboutMenuItem = gtk_menu_item_new_with_label("About");
	gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu), aboutMenuItem);

	GtkWidget *toolBar = gtk_toolbar_new();
	gtk_box_pack_start(GTK_BOX(mainBox), toolBar, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(toolBar),
								GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
	gtk_container_set_border_width(GTK_CONTAINER(toolBar), 0);

	GtkToolItem *menuButton =
		gtk_menu_tool_button_new_from_stock(GTK_STOCK_NEW);
	GtkWidget *newMenu = gtk_menu_new();
	GtkWidget *httpItem = gtk_menu_item_new_with_label("Open URL");
	gtk_menu_shell_append(GTK_MENU_SHELL(newMenu), httpItem);
	GtkWidget *btItem = gtk_menu_item_new_with_label("Open Torrent");
	gtk_menu_shell_append(GTK_MENU_SHELL(newMenu), btItem);
	gtk_widget_show_all(newMenu);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(menuButton),
								  newMenu);
	gtk_toolbar_insert(GTK_TOOLBAR(toolBar), menuButton, -1);

	GtkToolItem *startButton =
		gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	gtk_widget_set_tooltip_text(GTK_WIDGET(startButton), "Start");
	gtk_toolbar_insert(GTK_TOOLBAR(toolBar), startButton, -1);

	GtkToolItem *pauseButton =
		gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PAUSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(pauseButton), "Pause");
	gtk_toolbar_insert(GTK_TOOLBAR(toolBar), pauseButton, -1);

	gtk_toolbar_insert(GTK_TOOLBAR(toolBar),
					   gtk_separator_tool_item_new(), -1);

	GtkToolItem *rmButton =
		gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(rmButton), "Remove");
	gtk_toolbar_insert(GTK_TOOLBAR(toolBar), rmButton, -1);

	WlDownloader *downloader = wl_downloader_new();
	wl_downloader_set_selected_callback(downloader,
										wl_dl_window_downloader_selected_callback,
										window);
	wl_downloader_set_httper_status_callback(downloader,
											 wl_dl_window_httper_status_callback,
											 window);
	wl_downloader_set_bter_status_callback(downloader,
										   wl_dl_window_bter_status_callback,
										   window);
	gtk_box_pack_start(GTK_BOX(mainBox), GTK_WIDGET(downloader), TRUE,
					   TRUE, 0);

	/* 创建快捷键 */
	GtkAccelGroup *accelGroup = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accelGroup);
	gtk_widget_add_accelerator(urlMenuItem, "activate",
							   accelGroup, GDK_KEY_u, GDK_CONTROL_MASK,
							   GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(btMenuItem, "activate",
							   accelGroup, GDK_KEY_t, GDK_CONTROL_MASK,
							   GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(quitMenuItem, "activate",
							   accelGroup, GDK_KEY_q, GDK_CONTROL_MASK,
							   GTK_ACCEL_VISIBLE);

	g_signal_connect(G_OBJECT(startButton), "clicked",
					 G_CALLBACK(wl_dl_window_start_download), window);
	g_signal_connect(G_OBJECT(pauseButton), "clicked",
					 G_CALLBACK(wl_dl_window_pause_download), window);
	g_signal_connect(G_OBJECT(rmButton), "clicked",
					 G_CALLBACK(wl_dl_window_remove_download), window);

	g_signal_connect(G_OBJECT(menuButton), "clicked",
					 G_CALLBACK(wl_dl_window_open_url), window);
	g_signal_connect(G_OBJECT(urlMenuItem), "activate",
					 G_CALLBACK(wl_dl_window_open_url), window);
	g_signal_connect(G_OBJECT(httpItem), "activate",
					 G_CALLBACK(wl_dl_window_open_url), window);
	g_signal_connect(G_OBJECT(btItem), "activate",
					 G_CALLBACK(wl_dl_window_open_torrent), window);
	g_signal_connect(G_OBJECT(btMenuItem), "activate",
					 G_CALLBACK(wl_dl_window_open_torrent), window);
	g_signal_connect(G_OBJECT(quitMenuItem), "activate",
					 G_CALLBACK(wl_dl_window_destroy), window);
	g_signal_connect(G_OBJECT(aboutMenuItem), "activate",
					 G_CALLBACK(wl_dl_window_show_about_dialog), NULL);
	g_signal_connect(G_OBJECT(window), "destroy",
					 G_CALLBACK(wl_dl_window_destroy), window);

	window->start = startButton;
	window->pause = pauseButton;
	window->remove = rmButton;
	window->downloader = downloader;
	window->urlDialog = wl_url_dialog_new();
	window->bf_chooser =
		wl_bt_file_chooser_new(wl_downloader_create_ctor(downloader));

	wl_dl_window_set_start_enabled(window, FALSE);
	wl_dl_window_set_pause_enabled(window, FALSE);
	wl_dl_window_set_remove_enabled(window, FALSE);
}

static void wl_download_window_finalize(GObject * object)
{
	WlDownloadWindow *window = WL_DOWNLOAD_WINDOW(object);
	gtk_widget_destroy(GTK_WIDGET(window->urlDialog));
	g_object_unref(window->bf_chooser);
}

static void wl_download_window_class_init(WlDownloadWindowClass * klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_download_window_getter;
	objClass->set_property = wl_download_window_setter;
	objClass->finalize = wl_download_window_finalize;

	GParamSpec *ps;
	ps = g_param_spec_string("title",
							 "window title",
							 "Window Title",
							 WL_DL_WINDOW_TITLE,
							 G_PARAM_READABLE | G_PARAM_WRITABLE);
	g_object_class_install_property(objClass, WL_DL_WINDOW_PROPERTY_TITLE,
									ps);
}

static void wl_download_window_getter(GObject * object, guint property_id,
									  GValue * value, GParamSpec * ps)
{
	WlDownloadWindow *type = WL_DOWNLOAD_WINDOW(object);
	switch (property_id) {
	case WL_DL_WINDOW_PROPERTY_TITLE:
		g_value_set_string(value, gtk_window_get_title(GTK_WINDOW(type)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_download_window_setter(GObject * object, guint property_id,
									  const GValue * value,
									  GParamSpec * ps)
{
	WlDownloadWindow *type = WL_DOWNLOAD_WINDOW(object);
	switch (property_id) {
	case WL_DL_WINDOW_PROPERTY_TITLE:
		gtk_window_set_title(GTK_WINDOW(type), g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static inline void wl_dl_window_set_start_enabled(WlDownloadWindow *
												  window, gboolean enabled)
{
	gtk_widget_set_sensitive(GTK_WIDGET(window->start), enabled);
}

static inline void wl_dl_window_set_pause_enabled(WlDownloadWindow *
												  window, gboolean enabled)
{
	gtk_widget_set_sensitive(GTK_WIDGET(window->pause), enabled);
}

static inline void wl_dl_window_set_remove_enabled(WlDownloadWindow *
												   window,
												   gboolean enabled)
{
	gtk_widget_set_sensitive(GTK_WIDGET(window->remove), enabled);
}

static inline void
wl_dl_window_enable_button_by_httper_status(WlDownloadWindow * window,
											WlHttperStatus status)
{

	switch (status) {
	case WL_HTTPER_STATUS_NOT_START:
	case WL_HTTPER_STATUS_ABORT:
	case WL_HTTPER_STATUS_PAUSE:
	case WL_HTTPER_STATUS_RESUME:
		wl_dl_window_set_start_enabled(window, TRUE);
		wl_dl_window_set_pause_enabled(window, FALSE);
		wl_dl_window_set_remove_enabled(window, TRUE);
		break;
	case WL_HTTPER_STATUS_START:
		wl_dl_window_set_start_enabled(window, FALSE);
		wl_dl_window_set_pause_enabled(window, TRUE);
		wl_dl_window_set_remove_enabled(window, TRUE);
		break;
	case WL_HTTPER_STATUS_COMPLETE:
		wl_dl_window_set_start_enabled(window, FALSE);
		wl_dl_window_set_pause_enabled(window, FALSE);
		wl_dl_window_set_remove_enabled(window, TRUE);
		break;
	default:
		g_warning("unknown status!");
	}
}

static void wl_dl_window_downloader_selected_callback(WlDownloader * dl,
													  gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	gpointer selected = wl_downloader_get_selected(dl);
	if (selected) {
		if (WL_IS_HTTPER(selected)) {
			WlHttperStatus status =
				wl_httper_get_status(WL_HTTPER(selected));
			wl_dl_window_enable_button_by_httper_status(window, status);
		} else if (WL_IS_BTER(selected)) {
			WlBterStatus status = wl_bter_get_status(WL_BTER(selected));
			wl_dl_window_enable_button_by_httper_status(window, status);
		}
	} else {
		wl_dl_window_set_pause_enabled(window, FALSE);
		wl_dl_window_set_remove_enabled(window, FALSE);
		wl_dl_window_set_start_enabled(window, FALSE);
	}
}

static void wl_dl_window_httper_status_callback(WlHttper * httper,
												gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	WlHttperStatus status = wl_httper_get_status(httper);
	wl_dl_window_enable_button_by_httper_status(window, status);
}

static void wl_dl_window_bter_status_callback(WlBter * bter, gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	WlBterStatus status = wl_bter_get_status(bter);
	wl_dl_window_enable_button_by_httper_status(window, status);
}

static void wl_dl_window_destroy(GtkWidget * window, gpointer data)
{
	WlDownloadWindow *dlWindow = WL_DOWNLOAD_WINDOW(data);
	wl_downloader_save_tasks(dlWindow->downloader);
	gtk_main_quit();
}

#define DEFAULT_SITE_NAME   "index.html"

static gchar *url_get_last(const gchar * url)
{
	if (url == NULL)
		return NULL;
	const gchar *ptr = g_strstr_len(url, -1, "//");
	if (ptr) {
		ptr = ptr + 2;
	} else
		ptr = url;
	const gchar *tptr = ptr;
	ptr = g_strstr_len(tptr, -1, "/");
	while (ptr) {
		ptr = g_strstr_len(tptr, -1, "/");
		if (ptr == NULL) {
			ptr = tptr;
			break;
		} else {
			tptr = ptr + 1;
		}
	}
	if (ptr == NULL || *ptr == '\0')
		return g_strdup(DEFAULT_SITE_NAME);
	const gchar *mark;
	mark = g_strstr_len(ptr, -1, "?");
	if (mark == NULL)
		mark = g_strstr_len(ptr, -1, "#");
	if (mark == NULL)
		return g_strdup(ptr);
	if (mark - ptr == 0)
		return g_strdup(DEFAULT_SITE_NAME);
	return g_strndup(ptr, mark - ptr);
}

static gchar *wl_dl_window_get_filename_from_url(const gchar * url,
												 const gchar * filename)
{
	if (url == NULL || filename == NULL)
		return NULL;
	if (filename[0] != '\0')
		return g_strdup(filename);
	return url_get_last(url);
}

static gboolean wl_dl_window_overwrite_dialog(WlDownloadWindow * window,
											  const gchar * path)
{
	if (g_file_test(path, G_FILE_TEST_EXISTS)) {
		gchar *message = g_strdup_printf("'%s' exists! Overwrite?",
										 path);
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
												   GTK_DIALOG_MODAL |
												   GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_INFO,
												   GTK_BUTTONS_YES_NO,
												   "%s", message);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		g_free(message);
		if (response == GTK_RESPONSE_YES)
			return TRUE;
		return FALSE;
	}
	return TRUE;
}

static void wl_dl_window_open_url(GtkMenuItem * item, gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	const gchar *url;
	const gchar *dir;
	const gchar *filename;
	gint response = wl_url_dialog_run(window->urlDialog, NULL, NULL);
	if (response == GTK_RESPONSE_YES) {
		url = wl_url_dialog_get_url(window->urlDialog);
		dir = wl_url_dialog_get_folder(window->urlDialog);
		filename = wl_url_dialog_get_filename(window->urlDialog);
		if (dir == NULL || dir[0] == '\0')
			return;
		gchar *basename =
			wl_dl_window_get_filename_from_url(url, filename);
		if (basename == NULL)
			return;
		gchar *fullpath = g_strdup_printf("%s/%s", dir, basename);
		if (wl_dl_window_overwrite_dialog(window, fullpath))
			wl_downloader_append_httper(window->downloader, url, fullpath);
		g_free(fullpath);
		g_free(basename);
	}
}

static void wl_dl_window_open_torrent(GtkMenuItem * item, gpointer data)
{
	WlDownloadWindow *window = WL_DOWNLOAD_WINDOW(data);
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Torrent",
													GTK_WINDOW(window),
													GTK_FILE_CHOOSER_ACTION_OPEN,
													"Open",
													GTK_RESPONSE_ACCEPT,
													"Cancel",
													GTK_RESPONSE_CANCEL,
													NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
										g_get_home_dir());
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Torrent Files");
	gtk_file_filter_add_pattern(filter, "*.torrent");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "All Files");
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gint response;
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gchar *file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	gtk_widget_destroy(dialog);
	if (response == GTK_RESPONSE_ACCEPT) {
		tr_torrent *torrent =
			wl_bt_file_chooser_run(window->bf_chooser, file);
		if (torrent) {
			WlBter *bter =
				wl_downloader_append_bter(window->downloader, torrent);
		}
	}
	g_free(file);
}

static gboolean wl_dl_window_about_dialog_close(GtkWidget * widget,
												GdkEvent * event,
												gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

static GtkWidget *wl_dl_window_about_dialog(void)
{
	static GtkWidget *dialog = NULL;
	if (dialog == NULL) {
		dialog = gtk_about_dialog_new();
		gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "wdl");
		gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
									   "Copyright (c) Wiky L");
		gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog),
											WL_ICON_NAME);
		gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
									  "A Simple Download Manager");
		gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
		gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),
									 "https://launchpad.net/wdl");
		const gchar *authors[] = {
			"Wiky L(wiiiky@yeah.net)",
			NULL
		};
		gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
		gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(dialog), authors);
		gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog),
										  GTK_LICENSE_GPL_3_0);
		gtk_about_dialog_set_wrap_license(GTK_ABOUT_DIALOG(dialog), TRUE);

		GtkWidget *close =
			gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
											   GTK_RESPONSE_CANCEL);
		g_signal_connect_swapped(G_OBJECT(close), "pressed",
								 G_CALLBACK(gtk_widget_hide), dialog);
		g_signal_connect(G_OBJECT(dialog), "delete-event",
						 G_CALLBACK(wl_dl_window_about_dialog_close),
						 NULL);
	}
	return dialog;
}

static void wl_dl_window_show_about_dialog(GtkMenuItem * item,
										   gpointer data)
{
	GtkWidget *dialog = wl_dl_window_about_dialog();
	gtk_dialog_run(GTK_DIALOG(dialog));

}

static void wl_dl_window_start_download(GtkToolButton * button,
										gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	gint status = wl_downloader_get_selected_status(window->downloader);
	if (status == WL_HTTPER_STATUS_PAUSE
		|| status == WL_HTTPER_STATUS_RESUME)
		wl_downloader_continue_selected(window->downloader);
	else if (status != WL_HTTPER_STATUS_COMPLETE)
		wl_downloader_start_selected(window->downloader);
}

static void wl_dl_window_pause_download(GtkToolButton * button,
										gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	wl_downloader_pause_selected(window->downloader);
}

static GtkWidget *wl_dl_window_remove_dialog(WlDownloadWindow * window)
{
	static GtkWidget *dialog = NULL;
	if (dialog == NULL) {
		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
										GTK_DIALOG_MODAL |
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_OK_CANCEL,
										"Remove task?");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG
												 (dialog),
												 "The file will not be delete from the file system!");
	}
	return dialog;
}

static void wl_dl_window_remove_download(GtkToolButton * button,
										 gpointer data)
{
	WlDownloadWindow *window = (WlDownloadWindow *) data;
	GtkWidget *dialog = wl_dl_window_remove_dialog(window);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	if (response == GTK_RESPONSE_OK)
		wl_downloader_remove_selected(window->downloader, FALSE);
}

/******************************************************
 * PUBLIC
 ***************************************************/
WlDownloadWindow *wl_download_window_new(void)
{
	WlDownloadWindow *window = g_object_new(WL_TYPE_DOWNLOAD_WINDOW, NULL);
	wl_downloader_load_tasks(window->downloader);
	return window;
}
