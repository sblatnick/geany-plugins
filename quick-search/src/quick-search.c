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

	gtk_window_set_title(GTK_WINDOW(dialog), _("Quick Search"));
	gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 200, -1);

  entry = gtk_entry_new();
  gtk_entry_set_icon_from_stock(GTK_ENTRY(entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);

	gtk_container_add(GTK_CONTAINER(dialog), entry);
	g_signal_connect(entry, "key-release-event", G_CALLBACK(onkey), NULL);

	GeanyKeyGroup *key_group;
	
	key_group = plugin_set_key_group(geany_plugin, "quick_search_keyboard_shortcut", KB_GROUP, NULL);
	keybindings_set_item(key_group, KB_QUICK_SEARCH, quick_search, 0, 0,
		"quick_search_keyboard_shortcut", _("Quick Search..."), NULL);
	keybindings_set_item(key_group, KB_QUICK_SEARCH_NEXT, quick_next, 0, 0,
		"quick_search_keyboard_shortcut", _("Go Next"), NULL);
	keybindings_set_item(key_group, KB_QUICK_SEARCH_PREV, quick_prev, 0, 0,
		"quick_search_keyboard_shortcut", _("Go Previous"), NULL);
}

void plugin_cleanup(void)
{
  gtk_widget_destroy(dialog);
}

