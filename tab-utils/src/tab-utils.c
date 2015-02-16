#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Tab Utils", "Various tab utilities", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

static void tab_utils_close_before(GtkButton *button, GtkWidget *child)
{
	gint pos = gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->notebook), child);
	printf("close before tab %d\n", pos);
	gint i;
	for(i = pos-1; i >= 0; i--) {
		printf("closing %d\n", i);
		GeanyDocument *doc = document_get_from_page(i);
		document_close(doc);
	}
}

static void tab_utils_close_after(GtkButton *button, GtkWidget *child)
{
	gint pos = gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->notebook), child);
	printf("close after tab %d\n", pos);
	gint i;
	for(i = pos+1; i < GEANY(documents_array)->len; i++) {
		printf("closing %d\n", i);
		if(documents[i]->is_valid)
		{
			document_close(documents[i]);
		}
	}
}

static gboolean tab_util_click(GtkWidget *widget, GdkEventButton *event, GtkWidget *child)
{
	if(event->button == 3)
	{		
		GtkWidget *menu = gtk_menu_new();		
		GtkWidget *menu_item = ui_image_menu_item_new(GTK_STOCK_CLOSE, _("Close before"));
		gtk_widget_show(menu_item);
		gtk_container_add(GTK_CONTAINER(menu), menu_item);
		g_signal_connect(menu_item, "activate", G_CALLBACK(tab_utils_close_before), child);

		menu_item = ui_image_menu_item_new(GTK_STOCK_CLOSE, _("Close after"));
		gtk_widget_show(menu_item);
		gtk_container_add(GTK_CONTAINER(menu), menu_item);
		g_signal_connect(menu_item, "activate", G_CALLBACK(tab_utils_close_after), child);

		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);		
		return TRUE;
	}
	return FALSE;
}

void tab_util_new_tab(
	GtkNotebook *notebook,
	GtkWidget   *child,
	guint        page_num,
	gpointer     user_data
)
{
	GtkWidget *label = gtk_notebook_get_tab_label(notebook, child);
	GeanyDocument *doc = document_get_from_page(page_num);
	gulong popup_id = g_signal_handler_find(label, G_SIGNAL_MATCH_ID, g_signal_lookup("button-press-event", GTK_TYPE_WIDGET), 0, NULL, NULL, doc);
	if(popup_id) {
		g_signal_handler_disconnect(label, popup_id);
		g_signal_connect(label, "button-press-event", G_CALLBACK(tab_util_click), child);
	}
}

void plugin_init(GeanyData *data)
{
	g_signal_connect_after(geany->main_widgets->notebook, "page-added", G_CALLBACK(tab_util_new_tab), NULL);
}
