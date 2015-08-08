#include "wldownloadwindow.h"
#include "libtransmission/transmission.h"
#include "libtransmission/variant.h"

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	WlDownloadWindow *window = wl_download_window_new();

	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_main();
	return 0;
}
