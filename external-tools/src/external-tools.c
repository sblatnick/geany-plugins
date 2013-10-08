#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "panel.c"
#include "table.c"
#include "tool.c"
#include "dialog.c"

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("External Tools", "Allow external tools to be integrated into many common actions.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

const gchar *path, *conf, *tools, *home;
GRegex *name_regex, *path_regex, *match_regex;

static GtkWidget *tool_menu_item = NULL;

static void menu_callback(GtkMenuItem *menuitem, gpointer gdata)
{
	plugin_show_configure(geany_plugin);
}

void plugin_init(GeanyData *data)
{
	path_regex = g_regex_new("^\\.|^build$", G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
	name_regex = g_regex_new("^\\.|\\.(o|so|exe|class|pyc)$", G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
	match_regex = g_regex_new(g_strconcat("[^", G_DIR_SEPARATOR_S, "]+\\.[^\\s]+", NULL), G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);

	home = g_getenv("HOME");
	if (!home) {
		home = g_get_home_dir();
	}
	path = g_build_path(G_DIR_SEPARATOR_S, geany_data->app->configdir, "plugins", "external-tools", NULL);
	conf = g_build_path(G_DIR_SEPARATOR_S, path, "external-tools.conf", NULL);
	tools = g_build_path(G_DIR_SEPARATOR_S, path, "tools", NULL);
	g_mkdir_with_parents(tools, S_IRUSR | S_IWUSR | S_IXUSR);

	reload_tools();

	tool_menu_item = gtk_menu_item_new_with_mnemonic("External Tools...");
	gtk_widget_show(tool_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), tool_menu_item);
	g_signal_connect(tool_menu_item, "activate", G_CALLBACK(menu_callback), NULL);
	panel_init();
}

void plugin_cleanup(void)
{
	g_regex_unref(path_regex);
	g_regex_unref(name_regex);
	g_regex_unref(match_regex);
	clean_tools();
	gtk_container_remove(GTK_CONTAINER(geany->main_widgets->tools_menu), tool_menu_item);
	panel_cleanup();
}
