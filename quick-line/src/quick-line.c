#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

GeanyPlugin		 *geany_plugin;
GeanyData			 *geany_data;
GeanyFunctions	*geany_functions;

static GtkWidget *dialog, *entry;
static gulong handler;
static const gchar *text = "";

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Line", "Quickly go to the line entered.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
	KB_QUICK_LINE,
	KB_GROUP
};

static void quick_line(G_GNUC_UNUSED guint key_id)
{
	gint ox, oy, x, y;
	gdk_window_get_origin(gtk_widget_get_window(geany->main_widgets->window), &ox, &oy);
	gtk_widget_translate_coordinates(geany->main_widgets->notebook, geany->main_widgets->window, 0, 0, &x, &y);
	gtk_window_move(GTK_WINDOW(dialog), ox + x, oy + y);

	gtk_entry_set_text(GTK_ENTRY(entry), "");
	gtk_widget_show_all(dialog);
}

static gboolean on_out(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	g_signal_handler_disconnect(entry, handler);
	gtk_widget_hide(dialog);
	return FALSE;
}

static gboolean on_activate(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	text = gtk_entry_get_text(GTK_ENTRY(entry));
	GeanyDocument *doc = document_get_current();
	editor_goto_line(doc->editor, atoi(text) - 1, 0);
	on_out(NULL, NULL, NULL);
	return FALSE;
}

static void on_in(GtkWidget *widget, gpointer user_data) {
	handler = g_signal_connect(entry, "grab-notify", G_CALLBACK(on_out), NULL);
	gdk_keyboard_grab(widget->window, TRUE, GDK_CURRENT_TIME);
	gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
}

static gboolean on_key(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if(event->keyval == GDK_Escape) {
		on_out(NULL, NULL, NULL);
	}
	else {
		text = gtk_entry_get_text(GTK_ENTRY(entry));
		//
	}
}

void plugin_init(GeanyData *data)
{
	dialog = gtk_window_new(GTK_WINDOW_POPUP);
	g_signal_connect(G_OBJECT(dialog), "show", G_CALLBACK(on_in), NULL);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(geany->main_widgets->window));

	entry = gtk_entry_new();
	gtk_entry_set_icon_from_stock(GTK_ENTRY(entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_JUMP_TO);
	gtk_widget_grab_focus(GTK_WIDGET(entry));

	gtk_container_add(GTK_CONTAINER(dialog), entry);
	g_signal_connect(entry, "key-release-event", G_CALLBACK(on_key), NULL);
	g_signal_connect(entry, "activate", G_CALLBACK(on_activate), NULL);

	GeanyKeyGroup *key_group;
	
	key_group = plugin_set_key_group(geany_plugin, "quick_line_keyboard_shortcut", KB_GROUP, NULL);
	keybindings_set_item(key_group, KB_QUICK_LINE, quick_line, 0, 0,
		"quick_line", _("Quick Line..."), NULL);
}

void plugin_cleanup(void)
{
	gtk_widget_destroy(dialog);
}

