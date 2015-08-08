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

#include "wlhttper.h"
#include "wlhttpermenu.h"

enum {
	WL_HTTPER_PROPERTY_URL = 1,
	WL_HTTPER_PROPERTY_SAVEPATH,
	WL_HTTPER_PROPERTY_CTIME,	/* 创建时间 */
};

/* 定时器的时间周期ms */
#define WL_HTTPER_TIMEOUT   (500)

G_DEFINE_TYPE(WlHttper, wl_httper, GTK_TYPE_EVENT_BOX);

static void wl_httper_getter(GObject * object, guint property_id,
							 GValue * value, GParamSpec * ps);
static void wl_httper_setter(GObject * object, guint property_id,
							 const GValue * value, GParamSpec * ps);
static gpointer wl_httper_download_thread(gpointer data);
static gboolean wl_httper_download_timeout(gpointer data);
static inline void wl_httper_set_dl_speed(WlHttper * httper,
										  gdouble speed);
static inline void wl_httper_set_total_size(WlHttper * httper,
											guint64 size);
static inline void wl_httper_set_dl_size(WlHttper * httper, guint64 size);
static inline void wl_httper_set_rtime(WlHttper * httper, guint64 time);
static inline void wl_httper_set_status(WlHttper * httper,
										WlHttperStatus status);
static inline void wl_httper_set_complete_info(WlHttper * httper);
static inline void wl_httper_set_abort_info(WlHttper * httper);
static inline void wl_httper_set_invalid_info(WlHttper * httper,
											  const gchar * strerror);
static inline void wl_httper_set_resume_info(WlHttper * httper,
											 guint64 now, guint64 total);
static inline void wl_httper_set_pause_info(WlHttper * httper);
static inline void wl_httper_set_start_info(WlHttper * httper);
static inline void wl_httper_set_default_info(WlHttper * httper);
static inline void wl_httper_clear_async_queue(GAsyncQueue * queue);
static inline void wl_httper_open_file(WlHttper * httper);
static inline void wl_httper_close_file(WlHttper * httper);
static inline void wl_httper_async_queue_unref(WlHttper * httper);
static inline void wl_httper_async_queue_new(WlHttper * httper);
static inline void wl_httper_add_timeout(WlHttper * httper);
static inline void wl_httper_remove_timeout(WlHttper * httper);
static inline void wl_httper_set_default_data(WlHttper * httper);

static void wl_httper_init(WlHttper * httper)
{
	GtkWidget *hBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_set_border_width(GTK_CONTAINER(httper), 0);
	gtk_container_add(GTK_CONTAINER(httper), hBox);

	GtkIconSize iconSize = GTK_ICON_SIZE_DIALOG;

	GtkWidget *image = gtk_image_new_from_icon_name("html",
													iconSize);
	gtk_box_pack_start(GTK_BOX(hBox), image, FALSE, FALSE, 0);

	GtkWidget *vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_set_border_width(GTK_CONTAINER(vBox), 8);
	gtk_box_pack_start(GTK_BOX(hBox), vBox, TRUE, TRUE, 0);

	GtkWidget *title = gtk_label_new("New HTTP Download");
	PangoAttrList *attrList = pango_attr_list_new();
	pango_attr_list_insert(attrList,
						   pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes(GTK_LABEL(title), attrList);
	gtk_widget_set_halign(title, GTK_ALIGN_START);
	gtk_label_set_ellipsize(GTK_LABEL(title), PANGO_ELLIPSIZE_END);
	gtk_label_set_single_line_mode(GTK_LABEL(title), TRUE);
	/*gtk_label_set_use_markup(GTK_LABEL(title), TRUE); */
	gtk_box_pack_start(GTK_BOX(vBox), title, TRUE, TRUE, 0);

	/* 进度条 */
	GtkWidget *progressBar = gtk_progress_bar_new();
	/*gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progressBar),0.1); */
	gtk_box_pack_start(GTK_BOX(vBox), progressBar, TRUE, TRUE, 0);


	/* 下载信息，下载速度、下载量等 */
	GtkWidget *iBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(vBox), iBox, TRUE, TRUE, 0);

	GtkWidget *info_dl = gtk_label_new(" 0 KB");
	gtk_widget_set_halign(info_dl, GTK_ALIGN_START);
	/*gtk_label_set_ellipsize(GTK_LABEL(info_dl), PANGO_ELLIPSIZE_END); */
	gtk_label_set_single_line_mode(GTK_LABEL(info_dl), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), info_dl, FALSE, FALSE, 0);

	GtkWidget *info_total = gtk_label_new("of 0 KB -");
	gtk_label_set_single_line_mode(GTK_LABEL(info_total), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), info_total, FALSE, FALSE, 0);

	GtkWidget *arrow =
		gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start(GTK_BOX(iBox), arrow, FALSE, FALSE, 0);
	GtkWidget *info_speed = gtk_label_new("0.00 kB/s");
	gtk_label_set_single_line_mode(GTK_LABEL(info_speed), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), info_speed, FALSE, FALSE, 0);

	GtkWidget *info_time = gtk_label_new(" (unknown time remaining)");
	gtk_label_set_single_line_mode(GTK_LABEL(info_time), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), info_time, FALSE, FALSE, 0);

	curl_global_init(CURL_GLOBAL_ALL);

	httper->iconImage = image;
	httper->titleLabel = title;
	httper->dlLabel = info_dl;
	httper->totalLabel = info_total;
	httper->speedLabel = info_speed;
	httper->timeLabel = info_time;
	httper->progressBar = progressBar;
	httper->iconSize = iconSize;
	httper->url = NULL;
	httper->savePath = NULL;
	httper->thread = NULL;
	httper->easyCURL = NULL;
	httper->speed = 0;
	httper->percentDone = 0;
	httper->timeout = 0;
	httper->dlNow = 0;
	httper->dlTotal = 0;
	httper->totalLast = 0;
	httper->fOutput = NULL;
	httper->status = WL_HTTPER_STATUS_NOT_START;
	httper->cdt = g_date_time_new_now_local();
	httper->rqueue = NULL;
	httper->cqueue = NULL;
	httper->popMenu = NULL;
	httper->userData = NULL;
	httper->finishCallback = NULL;
	httper->cbData = NULL;
	httper->statusCallback = NULL;
	httper->statusData = NULL;
	httper->alreadyHave = 0;
}

static void wl_httper_finalize(GObject * object)
{
	WlHttper *httper = WL_HTTPER(object);

	wl_httper_remove_timeout(httper);
	/* 取消线程 */
	wl_httper_async_queue_unref(httper);
	g_free(httper->url);
	g_free(httper->savePath);
	g_date_time_unref(httper->cdt);
	if (httper->popMenu)
		gtk_widget_destroy(httper->popMenu);
}

static void wl_httper_class_init(WlHttperClass * klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_httper_getter;
	objClass->set_property = wl_httper_setter;
	objClass->finalize = wl_httper_finalize;

	GParamSpec *ps;
	ps = g_param_spec_string("url",
							 "download url",
							 "Download Url",
							 NULL,
							 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READABLE |
							 G_PARAM_WRITABLE);
	g_object_class_install_property(objClass, WL_HTTPER_PROPERTY_URL, ps);

	ps = g_param_spec_string("save-path",
							 "save path",
							 "Save Path",
							 NULL,
							 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READABLE |
							 G_PARAM_WRITABLE);
	g_object_class_install_property(objClass, WL_HTTPER_PROPERTY_SAVEPATH,
									ps);

	ps = g_param_spec_uint64("ctie",
							 "create time",
							 "Create Time",
							 0, G_MAXUINT64, 0, G_PARAM_READABLE);
	g_object_class_install_property(objClass, WL_HTTPER_PROPERTY_CTIME,
									ps);
}

static void wl_httper_getter(GObject * object, guint property_id,
							 GValue * value, GParamSpec * ps)
{
	WlHttper *httper = WL_HTTPER(object);
	switch (property_id) {
	case WL_HTTPER_PROPERTY_URL:
		g_value_set_string(value, httper->url);
		break;
	case WL_HTTPER_PROPERTY_SAVEPATH:
		g_value_set_string(value, httper->savePath);
		break;
	case WL_HTTPER_PROPERTY_CTIME:
		g_value_set_uint64(value, g_date_time_to_unix(httper->cdt));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_httper_setter(GObject * object, guint property_id,
							 const GValue * value, GParamSpec * ps)
{
	WlHttper *httper = WL_HTTPER(object);
	switch (property_id) {
	case WL_HTTPER_PROPERTY_URL:
		g_free(httper->url);
		httper->url = g_strdup(g_value_get_string(value));
		break;
	case WL_HTTPER_PROPERTY_SAVEPATH:
		g_free(httper->savePath);
		httper->savePath = g_strdup(g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static inline void wl_httper_clear_async_queue(GAsyncQueue * queue)
{
	while (g_async_queue_try_pop(queue));
	return;
}

static inline void wl_httper_set_default_data(WlHttper * httper)
{
	httper->speed = 0;
	httper->percentDone = 0;
	httper->dlData = 0;
	httper->dlTime = 0;
	httper->dlNow = 0;
	httper->alreadyHave = 0;
	httper->dlTotal = 0;
}

static inline void wl_httper_set_status(WlHttper * httper,
										WlHttperStatus status)
{
	httper->status = status;
	if (httper->popMenu)
		wl_httper_menu_set_sensitive(WL_HTTPER_MENU(httper->popMenu));
	if (httper->statusCallback)
		httper->statusCallback(httper, httper->statusData);
}

static inline void wl_httper_add_timeout(WlHttper * httper)
{
	httper->timeout =
		g_timeout_add(WL_HTTPER_TIMEOUT, wl_httper_download_timeout,
					  httper);
}

static inline void wl_httper_remove_timeout(WlHttper * httper)
{
	if (httper->timeout) {
		g_source_remove(httper->timeout);
		httper->timeout = 0;
	}
}

static inline void wl_httper_async_queue_unref(WlHttper * httper)
{
	if (httper->cqueue) {
		g_async_queue_push(httper->cqueue, (gpointer) 1);
		g_async_queue_unref(httper->cqueue);
		g_async_queue_unref(httper->rqueue);
		httper->cqueue = NULL;
		httper->rqueue = NULL;
	}
}

static inline void wl_httper_async_queue_new(WlHttper * httper)
{
	httper->cqueue = g_async_queue_new();
	httper->rqueue = g_async_queue_new();
}

static inline void wl_httper_set_default_info(WlHttper * httper)
{
	gtk_label_set_text(GTK_LABEL(httper->dlLabel), " 0 KB");
	/*gtk_label_set_text(GTK_LABEL(httper->totalLabel), "of 0 KB -"); */
	gtk_label_set_text(GTK_LABEL(httper->speedLabel), " 0.00 kB/s");
	gtk_label_set_text(GTK_LABEL(httper->timeLabel),
					   " (unknown time remaining)");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(httper->progressBar),
								  0.0);
	wl_httper_set_total_size(httper, 0);
}

static inline void wl_httper_set_complete_info(WlHttper * httper)
{
	gtk_label_set_text(GTK_LABEL(httper->speedLabel), "complete");
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(0, 65535 / 3,
														 65535 / 2));
	}
	gtk_label_set_attributes(GTK_LABEL(httper->speedLabel), attrList);
	gtk_label_set_text(GTK_LABEL(httper->timeLabel), "");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(httper->progressBar),
								  1.0);
	wl_httper_set_status(httper, WL_HTTPER_STATUS_COMPLETE);
	if (httper->dlTotal == 0) {
		httper->dlTotal = httper->dlNow;
	}
	wl_httper_set_dl_size(httper, httper->dlNow);
	wl_httper_set_total_size(httper, httper->dlTotal);
}

static inline void wl_httper_set_resume_info(WlHttper * httper,
											 guint64 now, guint64 total)
{
	wl_httper_set_pause_info(httper);
	wl_httper_set_dl_size(httper, now);
	wl_httper_set_total_size(httper, total);
	httper->dlTotal = total;
	httper->dlNow = now;
	httper->percentDone = (gdouble) now / (gdouble) total;
	if (total)
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
									  (httper->progressBar),
									  httper->percentDone);
	else
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
									  (httper->progressBar), 0.0);
}

static inline void wl_httper_set_invalid_info(WlHttper * httper,
											  const gchar * strerror)
{
	gchar *label = g_strdup_printf(" %s", strerror);
	gtk_label_set_text(GTK_LABEL(httper->speedLabel), label);
	gtk_label_set_text(GTK_LABEL(httper->timeLabel), "");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(httper->progressBar),
								  0.0);
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(-1, 0, 0));
	}
	gtk_label_set_attributes(GTK_LABEL(httper->speedLabel), attrList);
	g_free(label);
	wl_httper_set_status(httper, WL_HTTPER_STATUS_ABORT);
}

static inline void wl_httper_set_abort_info(WlHttper * httper)
{
	gtk_label_set_text(GTK_LABEL(httper->speedLabel), "aborted");
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(-1, 0, 0));
	}
	gtk_label_set_attributes(GTK_LABEL(httper->speedLabel), attrList);
	gtk_label_set_text(GTK_LABEL(httper->timeLabel), "");
	if (gtk_progress_bar_get_fraction
		(GTK_PROGRESS_BAR(httper->progressBar)) == 0)
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
									  (httper->progressBar), 0);
	wl_httper_set_status(httper, WL_HTTPER_STATUS_ABORT);
}

static inline void wl_httper_set_pause_info(WlHttper * httper)
{
	gtk_label_set_text(GTK_LABEL(httper->speedLabel), "paused");
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(65535 / 2,
														 65535 / 2, 0));
	}
	gtk_label_set_attributes(GTK_LABEL(httper->speedLabel), attrList);
	wl_httper_set_status(httper, WL_HTTPER_STATUS_PAUSE);
}

static inline void wl_httper_set_start_info(WlHttper * httper)
{
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(0, 65535 / 2,
														 65535 / 2));
	}
	gtk_label_set_attributes(GTK_LABEL(httper->speedLabel), attrList);
	wl_httper_set_status(httper, WL_HTTPER_STATUS_START);
}

static inline void wl_httper_set_dl_speed(WlHttper * httper, gdouble speed)
{
	gchar *label;
	if (speed >= 1000 * 1000)
		label = g_strdup_printf("%.2f mB/s", speed / (1000 * 1000));
	else if (speed >= 1000)
		label = g_strdup_printf("%.2f kB/s", speed / 1000);
	else
		label = g_strdup_printf("%.2f B/s", speed);
	gtk_label_set_text(GTK_LABEL(httper->speedLabel), label);
	g_free(label);
}

static inline void wl_httper_set_rtime(WlHttper * httper, guint64 time)
{
	gchar *label;
	if (time >= 24 * 60 * 60)
		label = g_strdup_printf(" (%lu d %lu h remaining)",
								time / (24 * 60 * 60),
								time % (24 * 60 * 60) / (60 * 60));
	else if (time >= 60 * 60)
		label = g_strdup_printf(" (%lu h %lu m remaining)",
								time / (60 * 60), time % (60 * 60) / 60);
	else if (time > 60)
		label = g_strdup_printf(" (%lu m %lu s remaining)",
								time / 60, time % 60);
	else
		label = g_strdup_printf(" (%lu s remaining)", time);
	gtk_label_set_text(GTK_LABEL(httper->timeLabel), label);
	g_free(label);
}

static inline void wl_httper_set_total_size(WlHttper * httper,
											guint64 size)
{
	if (httper->totalLast == size)
		return;
	httper->totalLast = size;
	gchar *label;
	gdouble dsize = (gdouble) size;
	if (size >= 1000 * 1000 * 1000)
		label = g_strdup_printf("of %.2f GB -",
								dsize / (1000 * 1000 * 1000));
	else if (size >= 1000 * 1000)
		label = g_strdup_printf("of %.2f MB -", dsize / (1000 * 1000));
	else if (size >= 1000)
		label = g_strdup_printf("of %.2f KB -", dsize / 1000);
	else
		label = g_strdup_printf("of %lu B -", size);
	gtk_label_set_text(GTK_LABEL(httper->totalLabel), label);
	g_free(label);
}

static inline void wl_httper_set_dl_size(WlHttper * httper, guint64 size)
{
	gchar *label;
	gdouble dsize = (gdouble) size;
	if (size >= 1000 * 1000 * 1000)
		label = g_strdup_printf(" %.2f GB", dsize / (1000 * 1000 * 1000));
	else if (size >= 1000 * 1000)
		label = g_strdup_printf(" %.2f MB", dsize / (1000 * 1000));
	else if (size >= 1000)
		label = g_strdup_printf(" %.2f KB", dsize / 1000);
	else
		label = g_strdup_printf(" %lu B", size);
	gtk_label_set_text(GTK_LABEL(httper->dlLabel), label);
	g_free(label);
}

typedef struct {
	WlHttper *httper;
	GAsyncQueue *cqueue;
} CurlCallbackData;

static gpointer wl_httper_download_thread(gpointer data)
{
	WlHttper *httper = data;
	GAsyncQueue *cqueue = httper->cqueue;
	GAsyncQueue *rqueue = httper->rqueue;
	CURL *curl = httper->easyCURL;
	/* 回调函数的参数结构 */
	CurlCallbackData *cdata = g_malloc(sizeof(CurlCallbackData));
	cdata->httper = httper;
	cdata->cqueue = cqueue;
	/* 接收到数据后的回调函数的用户自定义参数 */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, cdata);
	/* 进度回调函数的自定义参数 */
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, cdata);
	/* 开始下载 */
	CURLcode code = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if (code == CURLE_OK)
		g_async_queue_push(rqueue, (gpointer) WL_HTTPER_STATUS_COMPLETE);
	else
		g_async_queue_push(rqueue, (gpointer) code);
	g_async_queue_unref(rqueue);
	g_async_queue_unref(cqueue);
	g_free(cdata);
	g_message("download thread exits");
	return NULL;
}

static gboolean wl_httper_download_timeout(gpointer data)
{
	WlHttper *httper = (WlHttper *) data;
	gpointer popdata = g_async_queue_try_pop(httper->rqueue);
	if (popdata) {
		switch ((gulong) popdata) {
		case WL_HTTPER_STATUS_COMPLETE:	/* 下载完成 */
			wl_httper_set_complete_info(httper);
			break;
		case CURLE_ABORTED_BY_CALLBACK:	/* 取消 */
			wl_httper_set_abort_info(httper);
			break;
		default:				/* 网络错误 */
			wl_httper_set_invalid_info(httper, curl_easy_strerror((gulong)
																  popdata));
			break;
		}
		wl_httper_close_file(httper);
		if (httper->finishCallback)
			httper->finishCallback((gulong) popdata, httper->cbData);
		wl_httper_set_icon_from_file(httper, httper->savePath);
		wl_httper_async_queue_unref(httper);
		return FALSE;
	}

	if (httper->percentDone == 0)
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(httper->progressBar));
	else {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR
									  (httper->progressBar),
									  httper->percentDone);
	}

	wl_httper_set_dl_speed(httper, httper->speed);
	wl_httper_set_dl_size(httper, httper->dlNow);
	wl_httper_set_total_size(httper, httper->dlTotal);

	if (httper->dlTotal != 0)
		wl_httper_set_rtime(httper,
							(httper->dlTotal -
							 httper->dlNow) / httper->speed);

	return TRUE;
}

static size_t on_curl_write_callback(gchar * ptr, size_t size,
									 size_t nmemb, gpointer * userdata)
{
	CurlCallbackData *cdata = (CurlCallbackData *) userdata;
	WlHttper *httper = cdata->httper;
	GAsyncQueue *cqueue = cdata->cqueue;
	size_t len = size * nmemb;
	if (len == 0) {
		g_warning("The transfer file is empty!");
		return 0;
	}
	if (g_async_queue_try_pop(cqueue))
		return 0;
	guint64 now = time(NULL);
	if (httper->dlTime != 0) {
		httper->dlData += len;
		if (now - httper->dlTime > 0) {
			httper->speed =
				(gdouble) httper->dlData / (gdouble) (now -
													  httper->dlTime);
			httper->dlTime = now;
			httper->dlData = 0;
		}
	} else
		httper->dlTime = now;
	g_output_stream_write(G_OUTPUT_STREAM(httper->fOutput),
						  ptr, len, 0, NULL);
	return len;
}

static int on_curl_progress_callback(gpointer * clientp, double dltotal,
									 double dlnow, double ultotal,
									 double ulnow)
{
	CurlCallbackData *cdata = (CurlCallbackData *) clientp;
	WlHttper *httper = cdata->httper;
	GAsyncQueue *cqueue = cdata->cqueue;
	if (g_async_queue_try_pop(cqueue))
		return 1;
	if (dlnow == 0)
		return 0;
	httper->dlTotal = dltotal + httper->alreadyHave;
	httper->dlNow = dlnow + httper->alreadyHave;
	if (httper->dlTotal)
		httper->percentDone =
			(gdouble) httper->dlNow / (gdouble) httper->dlTotal;
	else
		httper->percentDone = 0;
	return 0;
}

/*********************************************************
 * PUBLIC
 ***********************************************************/

WlHttper *wl_httper_new(const gchar * url, const gchar * savePath)
{
	WlHttper *httper = g_object_new(WL_TYPE_HTTPER,
									"url", url,
									"save-path", savePath,
									NULL);

	wl_httper_set_title_from_path(httper, savePath);
	return httper;
}

inline void wl_httper_set_title(WlHttper * httper, const gchar * title)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	gtk_label_set_text(GTK_LABEL(httper->titleLabel), title);
}

inline void wl_httper_set_title_from_path(WlHttper * httper,
										  const gchar * path)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	gchar *basename = g_filename_display_basename(path);
	wl_httper_set_title(httper, basename);
	gtk_widget_set_tooltip_text(httper->titleLabel, path);
	g_free(basename);
}

inline void wl_httper_set_icon(WlHttper * httper, GIcon * icon)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	gtk_image_set_from_gicon(GTK_IMAGE(httper->iconImage),
							 icon, httper->iconSize);
}

inline void wl_httper_set_icon_from_name(WlHttper * httper,
										 const gchar * name)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	gtk_image_set_from_icon_name(GTK_IMAGE(httper->iconImage),
								 name, httper->iconSize);
}

inline void wl_httper_set_icon_from_file(WlHttper * httper,
										 const gchar * path)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	GFile *file = g_file_new_for_path(path);
	GFileInfo *finfo = g_file_query_info(file,
										 G_FILE_ATTRIBUTE_STANDARD_ICON,
										 G_FILE_QUERY_INFO_NONE, NULL,
										 NULL);
	if (finfo) {
		gtk_image_set_from_gicon(GTK_IMAGE(httper->iconImage),
								 g_file_info_get_icon(finfo),
								 httper->iconSize);
		g_object_unref(finfo);
	}

	g_object_unref(file);
}

inline void wl_httper_set_popmenu(WlHttper * httper, GtkWidget * menu)
{
	g_return_if_fail(WL_IS_HTTPER(httper) && GTK_IS_MENU(menu));
	if (httper->popMenu)
		gtk_widget_destroy(httper->popMenu);
	httper->popMenu = menu;
}

inline GtkWidget *wl_httper_get_popmenu(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	return httper->popMenu;
}

inline gpointer wl_httper_get_user_data(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	return httper->userData;
}

inline void wl_httper_set_user_data(WlHttper * httper, gpointer data)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	httper->userData = data;
}

static inline void wl_httper_open_file(WlHttper * httper)
{
	wl_httper_close_file(httper);
	GFile *file = g_file_new_for_path(httper->savePath);
	g_file_delete(file, NULL, NULL);
	httper->fOutput = g_file_create(file, G_FILE_CREATE_NONE, NULL, NULL);
	g_object_unref(file);
}

static inline void wl_httper_append_file(WlHttper * httper)
{
	wl_httper_close_file(httper);
	GFile *file = g_file_new_for_path(httper->savePath);
	httper->fOutput =
		g_file_append_to(file, G_FILE_CREATE_NONE, NULL, NULL);
	g_object_unref(file);
}

static inline void wl_httper_close_file(WlHttper * httper)
{
	if (httper->fOutput) {
		g_output_stream_close(G_OUTPUT_STREAM(httper->fOutput), NULL,
							  NULL);
		g_object_unref(httper->fOutput);
		httper->fOutput = NULL;
	}
}

void wl_httper_start(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	g_return_if_fail(httper->status != WL_HTTPER_STATUS_START ||
					 httper->status != WL_HTTPER_STATUS_PAUSE);

	if (httper->status == WL_HTTPER_STATUS_START ||
		httper->status == WL_HTTPER_STATUS_PAUSE)
		return;
	/* 如果保存路径无效则直接退出 */
	wl_httper_open_file(httper);
	if (httper->fOutput == NULL)
		return;

	CURL *easyCURL = curl_easy_init();
	/* 设置URL */
	curl_easy_setopt(easyCURL, CURLOPT_URL, httper->url);
	/* 关闭详细信息 */
	curl_easy_setopt(easyCURL, CURLOPT_VERBOSE, 0L);
	/* 自动重定向 */
	curl_easy_setopt(easyCURL, CURLOPT_FOLLOWLOCATION, 1L);
	/* 设置接收到数据后的回调函数 */
	curl_easy_setopt(easyCURL, CURLOPT_WRITEFUNCTION,
					 on_curl_write_callback);
	/* 设置进度回调函数 */
	curl_easy_setopt(easyCURL, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(easyCURL, CURLOPT_PROGRESSFUNCTION,
					 on_curl_progress_callback);
	/* 启用SSL  */
	curl_easy_setopt(easyCURL, CURLOPT_USE_SSL, CURLUSESSL_TRY);
	/* 关闭证书认证 */
	curl_easy_setopt(easyCURL, CURLOPT_SSL_VERIFYPEER, 0L);
	/* 设置用户代理,冒充FIREFOX */
	curl_easy_setopt(easyCURL, CURLOPT_USERAGENT,
					 "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:24.0)"
					 " Gecko/20100101 Firefox/24.0");
	/* 关闭信号，这可能会引发一个BUG */
	curl_easy_setopt(easyCURL, CURLOPT_NOSIGNAL, 1L);

	httper->easyCURL = easyCURL;
	/* GAsyncQueue */
	wl_httper_async_queue_new(httper);
	g_async_queue_ref(httper->cqueue);
	g_async_queue_ref(httper->rqueue);
	/* 初始化数据 */
	wl_httper_set_default_data(httper);
	/* 重置所有信息 */
	wl_httper_set_default_info(httper);
	/* 设置图标 */
	wl_httper_set_icon_from_file(httper, httper->savePath);
	httper->thread =
		g_thread_new("http-download", wl_httper_download_thread, httper);
	g_thread_unref(httper->thread);
	wl_httper_add_timeout(httper);
	wl_httper_set_start_info(httper);
}

void wl_httper_pause(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	if (httper->status != WL_HTTPER_STATUS_START)
		return;
	wl_httper_remove_timeout(httper);
	curl_easy_pause(httper->easyCURL, CURLPAUSE_RECV);
	wl_httper_set_pause_info(httper);
}

void wl_httper_continue(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	if (httper->status == WL_HTTPER_STATUS_PAUSE) {
		curl_easy_pause(httper->easyCURL, CURLPAUSE_RECV_CONT);
		wl_httper_add_timeout(httper);
		wl_httper_set_start_info(httper);
	} else if (httper->status == WL_HTTPER_STATUS_RESUME) {
		/* GAsyncQueue */
		wl_httper_async_queue_new(httper);
		g_async_queue_ref(httper->cqueue);
		g_async_queue_ref(httper->rqueue);
		httper->thread =
			g_thread_new("http-download", wl_httper_download_thread,
						 httper);
		g_thread_unref(httper->thread);
		wl_httper_add_timeout(httper);
		wl_httper_set_start_info(httper);
	}
}

void wl_httper_abort(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	if (httper->status != WL_HTTPER_STATUS_START &&
		httper->status != WL_HTTPER_STATUS_PAUSE)
		return;
	wl_httper_remove_timeout(httper);
	if (httper->status == WL_HTTPER_STATUS_PAUSE) {
		/*httper->timeout = */
		/*g_timeout_add(300, wl_httper_download_timeout, httper); */
		curl_easy_pause(httper->easyCURL, CURLPAUSE_RECV_CONT);
	}
	wl_httper_set_abort_info(httper);
	wl_httper_close_file(httper);
	wl_httper_async_queue_unref(httper);
}

void wl_httper_redownload(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	if (httper->status == WL_HTTPER_STATUS_PAUSE)
		curl_easy_pause(httper->easyCURL, CURLPAUSE_RECV_CONT);
	wl_httper_remove_timeout(httper);
	wl_httper_async_queue_unref(httper);

	wl_httper_set_status(httper, WL_HTTPER_STATUS_NOT_START);
	wl_httper_start(httper);
}

void wl_httper_highlight(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	static GtkCssProvider *css = NULL;
	if (css == NULL) {
		gchar *str = "GtkEventBox{border-radius:0px;"
			"background-color:rgba(130,160,220,200);}";
		css = gtk_css_provider_new();
		gtk_css_provider_load_from_data(css, str, -1, NULL);
	}
	gtk_style_context_add_provider(gtk_widget_get_style_context
								   (GTK_WIDGET(httper)),
								   GTK_STYLE_PROVIDER(css), G_MAXUINT);
}

void wl_httper_clear_highlight(WlHttper * httper)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	static GtkCssProvider *css = NULL;
	if (css == NULL) {
		gchar *str = "GtkEventBox{border-radius:0px;"
			"background-color:rgba(255,255,255,0);}";
		css = gtk_css_provider_new();
		gtk_css_provider_load_from_data(css, str, -1, NULL);
	}
	gtk_style_context_add_provider(gtk_widget_get_style_context
								   (GTK_WIDGET(httper)),
								   GTK_STYLE_PROVIDER(css), G_MAXUINT);
}

inline gint wl_httper_get_status(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), -1);
	return httper->status;
}

inline const GDateTime *wl_httper_get_ctime(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	return httper->cdt;
}

inline const gchar *wl_httper_get_path(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	return httper->savePath;
}

inline const gchar *wl_httper_get_url(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	return httper->url;
}

gboolean wl_httper_launch_path(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), FALSE);
	const gchar *path = wl_httper_get_path(httper);
	gboolean ret = FALSE;
	if (path == NULL)
		return ret;

	GFile *file = g_file_new_for_path(path);
	GFile *parent = g_file_get_parent(file);
	if (parent) {
		GAppInfo *info = g_file_query_default_handler(parent, NULL, NULL);
		if (info == NULL)
			goto OUT;
		GList *list = g_list_append(NULL, file);
		GAppLaunchContext *context = g_app_launch_context_new();
		if (g_strcmp0("nautilus", g_app_info_get_executable(info)) == 0) {
			/* 如果打开目录的方式是nautilus，则添加-s参数 */
			g_app_launch_context_setenv(context, "-s", NULL);
		}
		ret = g_app_info_launch(info, list, context, NULL);
		g_object_unref(info);
		g_object_unref(context);
		g_list_free(list);
	  OUT:
		g_object_unref(parent);
	}
	g_object_unref(file);
	return ret;
}

inline void wl_httper_set_callback(WlHttper * httper,
								   WlHttperCallback callback,
								   gpointer data)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	httper->finishCallback = callback;
	httper->cbData = data;
}

inline const gchar *wl_httper_get_title(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), NULL);
	return gtk_label_get_text(GTK_LABEL(httper->titleLabel));
}

void wl_httper_set_status_callback(WlHttper * httper,
								   WlHttperStatusCallback callback,
								   gpointer data)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	httper->statusCallback = callback;
	httper->statusData = data;
}

guint64 wl_httper_get_total_size(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), 0);
	return httper->dlTotal;
}

guint64 wl_httper_get_dl_size(WlHttper * httper)
{
	g_return_val_if_fail(WL_IS_HTTPER(httper), 0);
	return httper->dlNow;
}

static inline void wl_httper_resume(WlHttper * httper, guint64 length,
									guint64 total)
{
	g_return_if_fail(WL_IS_HTTPER(httper));
	g_return_if_fail(httper->status == WL_HTTPER_STATUS_START ||
					 httper->status == WL_HTTPER_STATUS_PAUSE ||
					 httper->status == WL_HTTPER_STATUS_RESUME);

	/* 如果保存路径无效则直接退出 */
	wl_httper_append_file(httper);
	if (httper->fOutput == NULL)
		return;

	CURL *easyCURL = curl_easy_init();
	/* 设置URL */
	curl_easy_setopt(easyCURL, CURLOPT_URL, httper->url);
	/* 关闭详细信息 */
	curl_easy_setopt(easyCURL, CURLOPT_VERBOSE, 0L);
	/* 自动重定向 */
	curl_easy_setopt(easyCURL, CURLOPT_FOLLOWLOCATION, 1L);
	/* 设置接收到数据后的回调函数 */
	curl_easy_setopt(easyCURL, CURLOPT_WRITEFUNCTION,
					 on_curl_write_callback);
	/* 设置进度回调函数 */
	curl_easy_setopt(easyCURL, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(easyCURL, CURLOPT_PROGRESSFUNCTION,
					 on_curl_progress_callback);
	/* 启用SSL  */
	curl_easy_setopt(easyCURL, CURLOPT_USE_SSL, CURLUSESSL_TRY);
	/* 关闭证书认证 */
	curl_easy_setopt(easyCURL, CURLOPT_SSL_VERIFYPEER, 0L);
	/* 设置用户代理,冒充FIREFOX */
	curl_easy_setopt(easyCURL, CURLOPT_USERAGENT,
					 "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:24.0)"
					 " Gecko/20100101 Firefox/24.0");
	/* 关闭信号，这可能会引发一个BUG */
	curl_easy_setopt(easyCURL, CURLOPT_NOSIGNAL, 1L);

	/* 断点续传 */
	curl_easy_setopt(easyCURL, CURLOPT_RESUME_FROM, (glong) length);

	httper->easyCURL = easyCURL;
	/* 初始化数据 */
	wl_httper_set_default_data(httper);
	httper->alreadyHave = length;
	/* 重置所有信息 */
	wl_httper_set_default_info(httper);
	/* 设置图标 */
	wl_httper_set_icon_from_file(httper, httper->savePath);
	//wl_httper_set_status (httper,WL_HTTPER_STATUS_START);
	wl_httper_set_resume_info(httper, length, total);
	httper->status = WL_HTTPER_STATUS_RESUME;
}

void wl_httper_load(WlHttper * httper, guint64 total_size, guint64 dl_size,
					guint status)
{
	g_return_if_fail(WL_IS_HTTPER(httper));

	wl_httper_set_status(httper, status);
	httper->dlTotal = total_size;
	httper->dlNow = dl_size;
	gsize length;
	gchar *contents = NULL;
	switch (status) {
	case WL_HTTPER_STATUS_COMPLETE:
		wl_httper_set_complete_info(httper);
		break;
	case WL_HTTPER_STATUS_NOT_START:
		wl_httper_set_default_info(httper);
		break;
	case WL_HTTPER_STATUS_PAUSE:
	case WL_HTTPER_STATUS_START:
	case WL_HTTPER_STATUS_RESUME:
		if (g_file_get_contents
			(httper->savePath, &contents, &length, NULL)) {
			/* 断点叙传 */
			g_free(contents);
			wl_httper_resume(httper, length, total_size);
		} else {
			wl_httper_set_default_info(httper);
			wl_httper_set_status(httper, WL_HTTPER_STATUS_NOT_START);
		}
		break;
	case WL_HTTPER_STATUS_ABORT:
	default:
		wl_httper_set_invalid_info(httper, "aborted");
		break;
	}
}
