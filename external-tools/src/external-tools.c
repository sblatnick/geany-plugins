#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "tool.c"
#include "dialog.c"

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("External Tools", "Allow external tools to be integrated into many common actions.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

gchar *path, *conf, *tools;
GKeyFile *config;
static GtkWidget *tool_menu_item = NULL;

static void menu_callback(GtkMenuItem *menuitem, gpointer gdata)
{
	plugin_show_configure(geany_plugin);
}

void plugin_init(GeanyData *data)
{
	config = g_key_file_new();
	
	path = g_build_path(G_DIR_SEPARATOR_S, geany_data->app->configdir, "plugins", "external-tools", NULL);
	conf = g_build_path(G_DIR_SEPARATOR_S, path, "external-tools.conf", NULL);
	tools = g_build_path(G_DIR_SEPARATOR_S, path, "tools", NULL);
	g_key_file_load_from_file(config, conf, G_KEY_FILE_NONE, NULL);
	g_mkdir_with_parents(tools, S_IRUSR | S_IWUSR | S_IXUSR);

	reload_tools();

	tool_menu_item = gtk_menu_item_new_with_mnemonic("External Tools...");
	gtk_widget_show(tool_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), tool_menu_item);
	g_signal_connect(tool_menu_item, "activate", G_CALLBACK(menu_callback), NULL);
}

void plugin_cleanup(void)
{
	gchar *data = g_key_file_to_data(config, NULL, NULL);
	utils_write_file(conf, data);
	g_free(data);
	g_free(path);
	g_free(conf);
	g_free(tools);
	g_key_file_free(config);
	clean_tools();
}
