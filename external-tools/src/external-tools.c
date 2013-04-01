#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>
#include "tool.c"
#include "dialog.c"

GeanyPlugin *geany_plugin;
GeanyData *geany_data;
GeanyFunctions *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("External Tools", "Allow external tools to be integrated into many common actions.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

gchar *path, *file;
static GKeyFile *config;
static GtkWidget *tool_menu_item = NULL;

enum
{
	KB_GROUP
};

static void menu_callback(GtkMenuItem *menuitem, gpointer gdata)
{
	plugin_show_configure(geany_plugin);
}

void plugin_init(GeanyData *data)
{
	config = g_key_file_new();
	
	path = g_strconcat(geany->app->configdir, G_DIR_SEPARATOR_S,
		"plugins", G_DIR_SEPARATOR_S, "external-tools", G_DIR_SEPARATOR_S, NULL);
	file = g_strconcat(path, "external-tools.conf", NULL);
	g_key_file_load_from_file(config, file, G_KEY_FILE_NONE, NULL);

	//Load existing tools (in groups)
	gsize len;
	gchar **groups = g_key_file_get_groups(config, &len);
	gsize i;

	for(i = 0; i < len; i++)
	{
		printf("loading data for group/tool %s", groups[i]);
		Tool *tool = load_tool(groups[i], config);

		//populate actions in BOTH dialog and setup the shortcuts
		//callbacks will use: g_key_file_set_integer(config, "external-tools", "type", type);
	}

	tool_menu_item = gtk_menu_item_new_with_mnemonic("External Tools...");
	gtk_widget_show(tool_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), tool_menu_item);
	g_signal_connect(tool_menu_item, "activate", G_CALLBACK(menu_callback), NULL);
}

void plugin_cleanup(void)
{
	gchar *data = g_key_file_to_data(config, NULL, NULL);
	utils_write_file(file, data);
	g_free(data);
	g_key_file_free(config);
}
