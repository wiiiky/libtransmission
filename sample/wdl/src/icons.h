/*
 * icons.[ch] written by Paolo Bacchilega, who writes:
 * "There is no problem for me, you can license
 * my code under whatever licence you wish :)"
 *
 * $Id: icons.h 13625 2012-12-05 17:29:46Z jordan $
 */

#ifndef GTR_ICONS_H
#define GTR_ICONS_H

#define DIRECTORY_MIME_TYPE "folder"
#define UNKNOWN_MIME_TYPE "unknown"

const char *gtr_get_mime_type_from_filename(const char *file);

GdkPixbuf *gtr_get_mime_type_icon(const char *mime_type,
								  GtkIconSize icon_size,
								  GtkWidget * for_widget);

/*
 * icons.[ch]是我从transmission的源代码中提取出来的，
 * 我自己加入了下面这个函数，只是对上面两个函数的简单封装
 */
GdkPixbuf *wdl_get_pixbuf_from_filename(const gchar * file,
										GtkIconSize size);

#endif
