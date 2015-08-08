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

#include "wlbter.h"
#include "wlbtermenu.h"
#include "icons.h"


enum {
	WL_BTER_PROPERTY_SESSION = 1,
	WL_BTER_PROPERTY_TORRENT,
};

#define WL_BTER_TIMEOUT (500)

G_DEFINE_TYPE(WlBter, wl_bter, GTK_TYPE_EVENT_BOX);

static void wl_bter_getter(GObject * object, guint property_id,
						   GValue * value, GParamSpec * ps);
static void wl_bter_setter(GObject * object, guint property_id,
						   const GValue * value, GParamSpec * ps);
static void wl_bter_finalize(GObject * object);

static inline void wl_bter_add_timeout(WlBter * bter);
static inline void wl_bter_remove_timeout(WlBter * bter);
static gboolean wl_bter_timeout(gpointer data);

static inline void wl_bter_set_dl_size(WlBter * bter, guint64 dlSize);
static inline void wl_bter_set_total_size(WlBter * bter,
										  guint64 totalSize);
static inline void wl_bter_set_dl_speed(WlBter * bter, gdouble Kbs);
static inline void wl_bter_set_rtime(WlBter * bter, gdouble Kbs,
									 guint64 left);
static inline void wl_bter_set_title(WlBter * bter, const gchar * title);

static inline void wl_bter_set_status(WlBter * bter, WlBterStatus status);
static inline void wl_bter_set_complete_info(WlBter * bter);
static inline void wl_bter_set_pause_info(WlBter * bter);
static inline void wl_bter_set_start_info(WlBter * bter);
static inline void wl_bter_set_error_info(WlBter * bter,
										  const gchar * error);
/* 将libtransmission定义的状态转化为wdl定义的状态 */
static inline gint wl_bter_convert_status(tr_torrent_activity activity);

static void wl_bter_init(WlBter * bter)
{
	GtkWidget *hBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_set_border_width(GTK_CONTAINER(hBox), 0);
	gtk_container_add(GTK_CONTAINER(bter), hBox);

	GtkIconSize iconSize = GTK_ICON_SIZE_DIALOG;
	GtkWidget *image = gtk_image_new_from_icon_name("folder",
													iconSize);
	gtk_box_pack_start(GTK_BOX(hBox), image, FALSE, FALSE, 0);

	GtkWidget *vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_set_border_width(GTK_CONTAINER(vBox), 8);
	gtk_box_pack_start(GTK_BOX(hBox), vBox, TRUE, TRUE, 0);

	GtkWidget *titleLabel = gtk_label_new("New BitTorrent Download");
	PangoAttrList *attrList = pango_attr_list_new();
	pango_attr_list_insert(attrList,
						   pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes(GTK_LABEL(titleLabel), attrList);
	gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
	gtk_label_set_ellipsize(GTK_LABEL(titleLabel), PANGO_ELLIPSIZE_END);
	gtk_label_set_single_line_mode(GTK_LABEL(titleLabel), TRUE);
	gtk_box_pack_start(GTK_BOX(vBox), titleLabel, TRUE, TRUE, 0);

	/* 进度条 */
	GtkWidget *progressBar = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vBox), progressBar, TRUE, TRUE, 0);

	/* 下载信息，速度等 */
	GtkWidget *iBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(vBox), iBox, TRUE, TRUE, 0);

	GtkWidget *dlLabel = gtk_label_new(" 0 KB");
	gtk_widget_set_halign(dlLabel, GTK_ALIGN_START);
	gtk_label_set_single_line_mode(GTK_LABEL(dlLabel), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), dlLabel, FALSE, FALSE, 0);

	GtkWidget *totalLabel = gtk_label_new("of 0 KB -");
	gtk_label_set_single_line_mode(GTK_LABEL(totalLabel), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), totalLabel, FALSE, FALSE, 0);

	GtkWidget *arrow =
		gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start(GTK_BOX(iBox), arrow, FALSE, FALSE, 0);
	GtkWidget *speedLabel = gtk_label_new(" 0.00 kB/s");
	gtk_label_set_single_line_mode(GTK_LABEL(speedLabel), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), speedLabel, FALSE, FALSE, 0);

	GtkWidget *timeLabel = gtk_label_new(" (unknown time remaining)");
	gtk_label_set_single_line_mode(GTK_LABEL(timeLabel), TRUE);
	gtk_box_pack_start(GTK_BOX(iBox), timeLabel, FALSE, FALSE, 0);


	bter->icon = image;
	bter->titleLabel = titleLabel;
	bter->dlLabel = dlLabel;
	bter->totalLabel = totalLabel;
	bter->speedLabel = speedLabel;
	bter->timeLabel = timeLabel;
	bter->progressBar = progressBar;
	bter->popMenu = NULL;
	bter->totalLast = -1;

	bter->session = NULL;
	bter->torrent = NULL;
	bter->timeout = -1;
	bter->statusCB = NULL;
	bter->statusCBData = NULL;
	bter->status = WL_BTER_STATUS_NOT_START;
	bter->userData = NULL;
}

static void wl_bter_class_init(WlBterClass * klass)
{
	GObjectClass *objClass = G_OBJECT_CLASS(klass);
	objClass->get_property = wl_bter_getter;
	objClass->set_property = wl_bter_setter;
	objClass->finalize = wl_bter_finalize;

	GParamSpec *ps;
	ps = g_param_spec_pointer("session",
							  "libtransmission session",
							  "Libtransmission Session",
							  G_PARAM_READABLE | G_PARAM_WRITABLE |
							  G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(objClass, WL_BTER_PROPERTY_SESSION,
									ps);

	ps = g_param_spec_pointer("torrent",
							  "libtransmission torrent",
							  "Libtransmission Torrent",
							  G_PARAM_READABLE | G_PARAM_WRITABLE |
							  G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(objClass, WL_BTER_PROPERTY_TORRENT,
									ps);
}

static void wl_bter_finalize(GObject * object)
{
	WlBter *bter = (WlBter *) object;
	g_source_remove(bter->timeout);
	if (bter->popMenu)
		gtk_widget_destroy(bter->popMenu);
}

static void wl_bter_getter(GObject * object, guint property_id,
						   GValue * value, GParamSpec * ps)
{
	WlBter *bter = WL_BTER(object);
	switch (property_id) {
	case WL_BTER_PROPERTY_SESSION:
		g_value_set_pointer(value, bter->session);
		break;
	case WL_BTER_PROPERTY_TORRENT:
		g_value_set_pointer(value, bter->torrent);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static void wl_bter_setter(GObject * object, guint property_id,
						   const GValue * value, GParamSpec * ps)
{
	WlBter *bter = WL_BTER(object);
	switch (property_id) {
	case WL_BTER_PROPERTY_SESSION:
		bter->session = g_value_get_pointer(value);
		break;
	case WL_BTER_PROPERTY_TORRENT:
		bter->torrent = g_value_get_pointer(value);
		const tr_info *info = tr_torrentInfo(bter->torrent);
		wl_bter_set_title(bter, info->originalName);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, ps);
		break;
	}
}

static inline gint wl_bter_convert_status(tr_torrent_activity activity)
{
	switch (activity) {
	case TR_STATUS_STOPPED:
		return WL_BTER_STATUS_PAUSE;
		break;
	case TR_STATUS_SEED:
	case TR_STATUS_SEED_WAIT:
		return WL_BTER_STATUS_COMPLETE;
		break;
	case TR_STATUS_CHECK:
	case TR_STATUS_CHECK_WAIT:
	case TR_STATUS_DOWNLOAD:
	case TR_STATUS_DOWNLOAD_WAIT:
		return WL_BTER_STATUS_START;
		break;
	default:
		return WL_BTER_STATUS_NOT_START;
		break;
	}
	return 0;
}


static inline void wl_bter_set_title(WlBter * bter, const gchar * title)
{
	gtk_label_set_text(GTK_LABEL(bter->titleLabel), title);
}

static inline void wl_bter_set_status(WlBter * bter, WlBterStatus status)
{
	bter->status = status;
	if (status == WL_BTER_STATUS_COMPLETE)
		wl_bter_set_complete_info(bter);
	else if (status == WL_BTER_STATUS_PAUSE)
		wl_bter_set_pause_info(bter);
	else if (status == WL_BTER_STATUS_START)
		wl_bter_set_start_info(bter);
	else if (status == WL_BTER_STATUS_ABORT) {
		const tr_stat *stat = tr_torrentStatCached(bter->torrent);
		wl_bter_set_error_info(bter, stat->errorString);
	}
	if (bter->statusCB)
		bter->statusCB(bter, bter->statusCBData);
	if (bter->popMenu)
		wl_bter_menu_set_sensitive(WL_BTER_MENU(bter->popMenu), bter);
}

static inline void wl_bter_set_error_info(WlBter * bter,
										  const gchar * error)
{
	gchar *label = g_strdup_printf(" %s", error);
	gtk_label_set_text(GTK_LABEL(bter->speedLabel), label);
	gtk_label_set_text(GTK_LABEL(bter->timeLabel), "");
	//gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(httper->progressBar),
	//                            0.0);
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(-1, 0, 0));
	}
	gtk_label_set_attributes(GTK_LABEL(bter->speedLabel), attrList);
	g_free(label);
	//wl_httper_set_status(httper, WL_HTTPER_STATUS_ABORT);
}

static inline void wl_bter_set_complete_info(WlBter * bter)
{
	gtk_label_set_text(GTK_LABEL(bter->speedLabel), "complete");
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(0, 65535 / 3,
														 65535 / 2));
	}
	gtk_label_set_attributes(GTK_LABEL(bter->speedLabel), attrList);
	gtk_label_set_text(GTK_LABEL(bter->timeLabel), "");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bter->progressBar),
								  1.0);
	//wl_bter_set_status(bter, WL_BTER_STATUS_COMPLETE);
}

static inline void wl_bter_set_start_info(WlBter * bter)
{
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(0, 65535 / 2,
														 65535 / 2));
	}
	gtk_label_set_attributes(GTK_LABEL(bter->speedLabel), attrList);
}

static inline void wl_bter_set_pause_info(WlBter * bter)
{
	gtk_label_set_text(GTK_LABEL(bter->speedLabel), "paused");
	static PangoAttrList *attrList = NULL;
	if (attrList == NULL) {
		attrList = pango_attr_list_new();
		pango_attr_list_insert(attrList,
							   pango_attr_foreground_new(65535 / 2,
														 65535 / 2, 0));
	}
	gtk_label_set_attributes(GTK_LABEL(bter->speedLabel), attrList);
}

/*
 * @description 添加定时器
 */
static inline void wl_bter_add_timeout(WlBter * bter)
{
	wl_bter_remove_timeout(bter);
	bter->timeout = g_timeout_add(WL_BTER_TIMEOUT, wl_bter_timeout, bter);
}

/*
 * @description 删除定时器
 */
static inline void wl_bter_remove_timeout(WlBter * bter)
{
	if (bter->timeout < 0)
		return;
	g_source_remove(bter->timeout);
	bter->timeout = -1;
}

/*
 * @description 周期性调用
 */
static gboolean wl_bter_timeout(gpointer data)
{
	WlBter *bter = WL_BTER(data);
	tr_torrent *torrent = bter->torrent;
	const tr_stat *stat = tr_torrentStatCached(torrent);

	if (stat->error != 0) {
		g_message("bt error");
		wl_bter_set_status(bter, WL_BTER_STATUS_ABORT);
		return FALSE;
	}

	/* 下载百分比 */
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bter->progressBar),
								  stat->percentDone);
	/* 下载速度 */
	wl_bter_set_dl_speed(bter, stat->pieceDownloadSpeed_KBps);
	/* 已下载数据和总数据 */
	wl_bter_set_total_size(bter, stat->sizeWhenDone);
	wl_bter_set_dl_size(bter, stat->haveValid + stat->haveUnchecked);
	/* 剩余时间 */
	wl_bter_set_rtime(bter, stat->pieceDownloadSpeed_KBps,
					  stat->leftUntilDone);
	if (stat->percentDone == 1.0) {
		g_message("complete");
		wl_bter_set_status(bter, WL_BTER_STATUS_COMPLETE);
		return FALSE;
	}
	return TRUE;
}

static inline void wl_bter_set_dl_size(WlBter * bter, guint64 dlSize)
{
	gchar label[20];
	if (dlSize > 1000 * 1000 * 1000) {	/* GB */
		g_snprintf(label, 20, " %.2f GB",
				   ((gdouble) dlSize) / (1000.0 * 1000.0 * 1000.0));
	} else if (dlSize > 1000 * 1000) {	/* MB */
		g_snprintf(label, 20, " %.2f MB",
				   ((gdouble) dlSize) / (1000.0 * 1000.0));
	} else if (dlSize > 1000) {	/* KB */
		g_snprintf(label, 20, " %.2f KB", ((gdouble) dlSize) / 1000.0);
	} else {					/* B */
		g_snprintf(label, 20, " %lu B", dlSize);
	}
	gtk_label_set_text(GTK_LABEL(bter->dlLabel), label);
}

static inline void wl_bter_set_total_size(WlBter * bter, guint64 totalSize)
{
	gchar label[25];
	if (bter->totalLast == totalSize)
		return;
	if (totalSize > 1000 * 1000 * 1000) {	/* GB */
		g_snprintf(label, 25, "of %.2f GB -",
				   ((gdouble) totalSize) / (1000.0 * 1000.0 * 1000.0));
	} else if (totalSize > 1000 * 1000) {	/* MB */
		g_snprintf(label, 25, "of %.2f MB -",
				   ((gdouble) totalSize) / (1000.0 * 1000.0));
	} else if (totalSize > 1000) {	/* KB */
		g_snprintf(label, 25, "of %.2f KB -",
				   ((gdouble) totalSize) / 1000.0);
	} else {					/* B */
		g_snprintf(label, 25, "of %lu B -", totalSize);
	}
	gtk_label_set_text(GTK_LABEL(bter->totalLabel), label);
}

static inline void wl_bter_set_dl_speed(WlBter * bter, gdouble Kbs)
{
	gchar label[20];
	if (Kbs > 1000) {			/* mB/s */
		g_snprintf(label, 20, " %.2f mB/s", Kbs / 1000);
	} else if (Kbs < 1) {
		g_snprintf(label, 20, " %.0f B/s", Kbs * 1000);
	} else {
		g_snprintf(label, 20, " %.2f kB/s", Kbs);
	}
	gtk_label_set_text(GTK_LABEL(bter->speedLabel), label);
}

static inline void wl_bter_set_rtime(WlBter * bter, gdouble Kbs,
									 guint64 left)
{
	gchar label[30];
	if (Kbs <= 0) {
		gtk_label_set_text(GTK_LABEL(bter->timeLabel),
						   " (unknown time remaining)");
		return;
	}
	guint64 time = left / (Kbs * 1000);
	if (time >= 24 * 60 * 60)
		g_snprintf(label, 30, " (%lu d %lu h remaining)",
				   time / (24 * 60 * 60),
				   time % (24 * 60 * 60) / (60 * 60));
	else if (time >= 60 * 60)
		g_snprintf(label, 30, " (%lu h %lu m remaining)",
				   time / (60 * 60), time % (60 * 60) / 60);
	else if (time > 60)
		g_snprintf(label, 30, " (%lu m %lu s remaining)",
				   time / 60, time % 60);
	else
		g_snprintf(label, 30, " (%lu s remaining)", time);
	gtk_label_set_text(GTK_LABEL(bter->timeLabel), label);
}

static void updateIcon(WlBter * bter)
{
	if (bter->torrent == NULL)
		return;
	tr_info *torInfo = tr_torrentInfo(bter->torrent);
	if (torInfo->isMultifile) {
		gtk_image_set_from_icon_name(GTK_IMAGE(bter->icon), "folder",
									 GTK_ICON_SIZE_DIALOG);
	} else {
		gtk_image_set_from_pixbuf(GTK_IMAGE(bter->icon),
								  wdl_get_pixbuf_from_filename(torInfo->
															   files[0].
															   name,
															   GTK_ICON_SIZE_DIALOG));
	}
}

/**********************************************************
 * PUBLIC
 *******************************************************/
WlBter *wl_bter_new(tr_session * session, tr_torrent * torrent)
{
	g_return_val_if_fail(session != NULL && torrent != NULL, NULL);
	WlBter *bter = (WlBter *) g_object_new(WL_TYPE_BTER,
										   "session", session,
										   "torrent", torrent,
										   NULL);

	const tr_stat *stat = tr_torrentStatCached(torrent);

	if (stat->error != 0) {
		g_message("bt error");
		wl_bter_set_status(bter, WL_BTER_STATUS_ABORT);
		return FALSE;
	}

	/* 下载百分比 */
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bter->progressBar),
								  stat->percentDone);
	/* 下载速度 */
	//wl_bter_set_dl_speed(bter, stat->pieceDownloadSpeed_KBps);
	/* 已下载数据和总数据 */
	wl_bter_set_total_size(bter, stat->sizeWhenDone);
	wl_bter_set_dl_size(bter, stat->haveValid + stat->haveUnchecked);
	updateIcon(bter);
	/* 剩余时间 */
	//wl_bter_set_rtime(bter, stat->pieceDownloadSpeed_KBps,
	//                stat->leftUntilDone);
	if (stat->percentDone == 1.0) {
		g_message("complete");
		wl_bter_set_status(bter, WL_BTER_STATUS_COMPLETE);
	}
	return bter;
}

WlBter *wl_bter_new_from_file(tr_session * session, const gchar * path)
{
	g_return_val_if_fail(session != NULL && path != NULL, NULL);
	tr_torrent *torrent;
	tr_ctor *ctor = tr_ctorNew(session);
	if (ctor) {
		tr_ctorSetMetainfoFromFile(ctor, path);
		torrent = tr_torrentNew(ctor, NULL, NULL);
		tr_ctorFree(ctor);
	} else
		return NULL;
	/* 路径不对? */
	if (torrent == NULL)
		return NULL;
	WlBter *bter = (WlBter *) g_object_new(WL_TYPE_BTER,
										   "session", session,
										   "torrent", torrent,
										   NULL);
	return bter;
}

WlBter *wl_bter_new_from_magnetlink(tr_session * session,
									const gchar * link)
{
	g_return_val_if_fail(session != NULL && link != NULL, NULL);
	tr_torrent *torrent;
	tr_ctor *ctor = tr_ctorNew(session);
	if (ctor) {
		tr_ctorSetMetainfoFromMagnetLink(ctor, link);
		torrent = tr_torrentNew(ctor, NULL, NULL);
		tr_ctorFree(ctor);
	} else
		return NULL;
	/* MagnetLink 无效? */
	if (torrent == NULL)
		return NULL;
	WlBter *bter = (WlBter *) g_object_new(WL_TYPE_BTER,
										   "session", session,
										   "torrent", torrent,
										   NULL);
	return bter;
}

/*
 * @description 开始和继续
 */
void wl_bter_start(WlBter * bter)
{
	g_return_if_fail(WL_IS_BTER(bter) && bter->torrent != NULL);
	wl_bter_set_status(bter, WL_BTER_STATUS_START);
	tr_torrentStart(bter->torrent);
	wl_bter_add_timeout(bter);
}

void wl_bter_pause(WlBter * bter)
{
	g_return_if_fail(WL_IS_BTER(bter) && bter->torrent != NULL);
	tr_torrentStop(bter->torrent);
	wl_bter_set_status(bter, WL_BTER_STATUS_PAUSE);
	wl_bter_remove_timeout(bter);
}

gint wl_bter_get_status(WlBter * bter)
{
	g_return_val_if_fail(WL_IS_BTER(bter), 0);
	//const tr_stat *stat = tr_torrentStat(bter->torrent);
	//return wl_bter_convert_status(stat->activity);
	return bter->status;
}

tr_torrent *wl_bter_get_torrent(WlBter * bter)
{
	g_return_val_if_fail(WL_IS_BTER(bter), NULL);
	return bter->torrent;
}

const gchar *wl_bter_get_path(WlBter * bter)
{
	g_return_val_if_fail(WL_IS_BTER(bter) && bter->torrent != NULL, NULL);
	return tr_torrentGetDownloadDir(bter->torrent);
}

const gchar *wl_bter_get_magnet(WlBter * bter)
{
	g_return_val_if_fail(WL_IS_BTER(bter) && bter->torrent != NULL, NULL);
	return tr_torrentGetMagnetLink(bter->torrent);
}

void wl_bter_highlight(WlBter * bter)
{
	g_return_if_fail(WL_IS_BTER(bter));
	static GtkCssProvider *css = NULL;
	if (css == NULL) {
		gchar *str = "GtkEventBox{border-radius:0px;"
			"background-color:rgba(130,160,220,200);}";
		css = gtk_css_provider_new();
		gtk_css_provider_load_from_data(css, str, -1, NULL);
	}
	gtk_style_context_add_provider(gtk_widget_get_style_context
								   (GTK_WIDGET(bter)),
								   GTK_STYLE_PROVIDER(css), G_MAXUINT);
}

void wl_bter_clear_highlight(WlBter * bter)
{
	g_return_if_fail(WL_IS_BTER(bter));
	static GtkCssProvider *css = NULL;
	if (css == NULL) {
		gchar *str = "GtkEventBox{border-radius:0px;"
			"background-color:rgba(255,255,255,0);}";
		css = gtk_css_provider_new();
		gtk_css_provider_load_from_data(css, str, -1, NULL);
	}
	gtk_style_context_add_provider(gtk_widget_get_style_context
								   (GTK_WIDGET(bter)),
								   GTK_STYLE_PROVIDER(css), G_MAXUINT);
}

void wl_bter_set_status_callback(WlBter * bter,
								 WlBterStatusCallback callback,
								 gpointer data)
{
	g_return_if_fail(WL_IS_BTER(bter));
	bter->statusCB = callback;
	bter->statusCBData = data;
}

void wl_bter_set_popmenu(WlBter * bter, GtkWidget * menu)
{
	g_return_if_fail(WL_IS_BTER(bter));
	bter->popMenu = menu;
}

GtkWidget *wl_bter_get_popmenu(WlBter * bter)
{
	g_return_val_if_fail(WL_IS_BTER(bter), NULL);
	return bter->popMenu;
}

void wl_bter_set_user_data(WlBter * bter, gpointer data)
{
	g_return_if_fail(WL_IS_BTER(bter));
	bter->userData = data;
}

gpointer wl_bter_get_user_data(WlBter * bter)
{
	g_return_val_if_fail(WL_IS_BTER(bter), NULL);
	return bter->userData;
}
