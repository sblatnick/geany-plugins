#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools;
static GtkWidget *panel;
static GtkWidget *label;

void panel_init() {
	label = gtk_label_new(_("Tools"));
	panel = gtk_vbox_new(FALSE, 6);
	gtk_notebook_append_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		panel,
		label
	);
	gtk_widget_show(label);
	gtk_widget_show(panel);
}

void panel_cleanup() {
	gtk_notebook_remove_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook), panel)
	);
}
