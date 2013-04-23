#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools;
static GtkWidget *panel;
static GtkWidget *label;
static GtkWidget *text_view;
static GtkWidget *scrollable;

void panel_init()
{
	label = gtk_label_new(_("Tools"));
	panel = gtk_vbox_new(FALSE, 6);
	text_view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
	scrollable = gtk_scrolled_window_new(NULL, NULL);
	
	gtk_container_add(GTK_CONTAINER(scrollable), text_view);
	gtk_box_pack_start(GTK_BOX(panel), scrollable, TRUE, TRUE, 2);
	
	gtk_notebook_append_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		panel,
		label
	);
	
	gtk_widget_show(label);
	gtk_widget_show_all(panel);
}

void panel_cleanup()
{
	gtk_notebook_remove_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook), panel)
	);
}

static GtkTextBuffer* buffer()
{
	return gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
}

void panel_prepare()
{
	gtk_text_buffer_set_text(buffer(), "", 0);
	
	//Focus the tab:
	gtk_notebook_set_current_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook), panel)
	);
}

void panel_print(gchar *text)
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(buffer(), &iter);
	gtk_text_buffer_insert(buffer(), &iter, text, -1);
	
	//Scroll to bottom:
	GtkTextMark *mark;
	mark = gtk_text_buffer_get_insert(buffer());
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view), mark, 0.0, FALSE, 0.0, 0.0);
}
