#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>

GeanyPlugin		 *geany_plugin;
GeanyData			 *geany_data;
GeanyFunctions	*geany_functions;

static GtkWidget *main_menu_item = NULL;
static GtkWidget *dialog, *entry;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Search", "Do a case-insensitive search on the current document while highlighting all results", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
	KB_QUICK_SEARCH,
	KB_QUICK_SEARCH_NEXT,
	KB_QUICK_SEARCH_PREV,
	KB_GROUP
};

static void quick_search(G_GNUC_UNUSED guint key_id)
{
  gint ox, oy, x, y;
  gdk_window_get_origin(gtk_widget_get_window(geany->main_widgets->window), &ox, &oy);
  gtk_widget_translate_coordinates(geany->main_widgets->notebook, geany->main_widgets->window, 0, 0, &x, &y);
  gtk_window_move(GTK_WINDOW(dialog), ox + x, oy + y);

  gtk_widget_show_all(dialog);
}

static void quick_next(G_GNUC_UNUSED guint key_id)
{

}

static void quick_prev(G_GNUC_UNUSED guint key_id)
{

}

static gboolean onkey(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{

}

void plugin_init(GeanyData *data)
{
	dialog = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(geany->main_widgets->window));

  entry = gtk_entry_new();
  gtk_entry_set_icon_from_stock(GTK_ENTRY(entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);

	gtk_container_add(GTK_CONTAINER(dialog), entry);
	g_signal_connect(entry, "key-release-event", G_CALLBACK(onkey), NULL);

	GeanyKeyGroup *key_group;
	
	key_group = plugin_set_key_group(geany_plugin, "quick_search_keyboard_shortcut", KB_GROUP, NULL);
	keybindings_set_item(key_group, KB_QUICK_SEARCH, quick_search, 0, 0,
		"quick_search", _("Quick Search..."), NULL);
	keybindings_set_item(key_group, KB_QUICK_SEARCH_NEXT, quick_next, 0, 0,
		"quick_search_next", _("Go Next"), NULL);
	keybindings_set_item(key_group, KB_QUICK_SEARCH_PREV, quick_prev, 0, 0,
		"quick_search_prev", _("Go Previous"), NULL);
}

void plugin_cleanup(void)
{
  gtk_widget_destroy(dialog);
}

