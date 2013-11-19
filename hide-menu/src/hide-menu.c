#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Hide Menu", "Hide the main menu to reduce it's used space.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

static gulong inMenuHandler;
static gint height = 4;
static GtkWidget *menubar, *menu;
static gboolean recent = TRUE;
static gboolean added = FALSE;

static gboolean on_out(G_GNUC_UNUSED gpointer data)
{
	if(recent) {
		recent = FALSE;
		return TRUE;
	}
	else {
		added = FALSE;
		gtk_widget_set_size_request(menubar, -1, height);
		return FALSE;
	}
}

static gboolean on_in(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	recent = TRUE;
	if(!added) {
		gtk_widget_set_size_request(menubar, -1, -1);
		g_timeout_add(500, on_out, NULL);
		added = TRUE;
	}
	return FALSE;
}

void plugin_init(GeanyData *data)
{
	menubar = ui_lookup_widget(main_widgets.window, "hbox_menubar");
	menu = ui_lookup_widget(main_widgets.window, "menubar1");
	gtk_widget_add_events(menu, GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(main_widgets.toolbar, GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_set_size_request(menubar, -1, height);
	inMenuHandler = g_signal_connect(menubar, "motion-notify-event", G_CALLBACK(on_in), NULL);
}

void plugin_cleanup(void)
{
	g_signal_handler_disconnect(menubar, inMenuHandler);
	gtk_widget_set_size_request(menubar, -1, -1);
}

