#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;
const gchar *path, *conf;
static GKeyFile *config;
static const GdkColor blue = {0, 0, 0, 65535};

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
	//Tab context menu:
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
	G_GNUC_UNUSED GObject *obj,
	G_GNUC_UNUSED GeanyDocument *doc,
	G_GNUC_UNUSED gpointer user_data
)
{
	GtkNotebook *notebook = GTK_NOTEBOOK(geany->main_widgets->notebook);
	guint page_num = document_get_notebook_page(doc);
	GtkWidget *child = gtk_notebook_get_nth_page(notebook, page_num);

	//Replace the tab context menu:
	GtkWidget *label = gtk_notebook_get_tab_label(notebook, child);
	gulong popup_id = g_signal_handler_find(label, G_SIGNAL_MATCH_ID, g_signal_lookup("button-press-event", GTK_TYPE_WIDGET), 0, NULL, NULL, NULL);
	if(popup_id) {
		g_signal_handler_disconnect(label, popup_id);
		g_signal_connect(label, "button-press-event", G_CALLBACK(tab_util_click), child);
	}

	//Restore group to tab from previous session:
	gchar *group = g_key_file_get_string(config, "tab", doc->real_path, NULL);
	if(group) {
		//TODO:
		gtk_widget_modify_bg(gtk_widget_get_parent(label), GTK_STATE_ACTIVE, &blue);
		//gtk_widget_modify_bg(gtk_widget_get_parent(label), GTK_STATE_NORMAL, &blue);
	}

	printf("tab path: %s\n", doc->real_path);
}

void plugin_init(GeanyData *data)
{
	plugin_signal_connect(geany_plugin, NULL, "document-open", TRUE, G_CALLBACK(tab_util_new_tab), NULL);

	//Load config:
	config = g_key_file_new();
	path = g_build_path(G_DIR_SEPARATOR_S, geany_data->app->configdir, "plugins", "tab-utils", NULL);
	g_mkdir_with_parents(path, S_IRUSR | S_IWUSR | S_IXUSR);
	conf = g_build_path(G_DIR_SEPARATOR_S, path, "tab-utils.conf", NULL);
	g_key_file_load_from_file(config, conf, G_KEY_FILE_NONE, NULL);

	/* TODO?
	//Load tab groups from previous session:
	gchar **keys = g_key_file_get_keys(config, "tab", NULL, NULL);
	gchar **ptr = keys;

	if(ptr) {
		while (1)
		{
			const gchar *key = *ptr;
			if(!key) {
				break;
			}
			gchar *value = g_key_file_get_string(config, "tab", key, NULL);
			printf("%s => %s\n", key, value);
			//TODO
			ptr++;
		}
		g_strfreev(keys);
	}
	*/
}

void plugin_cleanup(void)
{
	g_key_file_free(config);
}
