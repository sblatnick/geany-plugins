#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Hide Menu", "Hide the main menu to reduce it's used space.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

static gulong inMenuHandler, outMenuHandler, inToolbarHandler, outToolbarHandler;
static gint height = 4;
static GtkWidget *menubar, *menu;

static gboolean on_in(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	printf("on_in\n");
	gtk_widget_set_size_request(menubar, -1, -1);
	return FALSE;
}

//~ static gboolean on_out(GtkWidget *widget, GdkEvent *event, gpointer user_data)
//~ {
	//~ printf("on_out\n");
	//~ gtk_widget_set_size_request(menubar, -1, height);
	//~ return FALSE;
//~ }

void plugin_init(GeanyData *data)
{
	menubar = ui_lookup_widget(main_widgets.window, "hbox_menubar");
	menu = ui_lookup_widget(main_widgets.window, "menubar1");
	gtk_widget_add_events(menu, GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(main_widgets.toolbar, GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_set_size_request(menubar, -1, height);
	inMenuHandler = g_signal_connect(menu, "motion-notify-event", G_CALLBACK(on_in), NULL);
	inToolbarHandler = g_signal_connect(main_widgets.toolbar, "motion-notify-event", G_CALLBACK(on_in), NULL);
	//~ outMenuHandler = g_signal_connect(menu, "leave-notify-event", G_CALLBACK(on_out), NULL);
	//~ outToolbarHandler = g_signal_connect(main_widgets.toolbar, "leave-notify-event", G_CALLBACK(on_out), NULL);
}

void plugin_cleanup(void)
{
	g_signal_handler_disconnect(menubar, inToolbarHandler);
	g_signal_handler_disconnect(menubar, inMenuHandler);
	g_signal_handler_disconnect(menubar, outToolbarHandler);
	g_signal_handler_disconnect(menubar, outMenuHandler);
	gtk_widget_set_size_request(menubar, -1, -1);
}

