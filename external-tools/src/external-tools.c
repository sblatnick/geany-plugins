#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

GeanyPlugin		 *geany_plugin;
GeanyData			 *geany_data;
GeanyFunctions	*geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("External Tools", "Allow external tools to be integrated into many common actions.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

static gchar *file;
static GKeyFile *config;

enum
{
	KB_GROUP
};

void dialog()
{
	//show the dialog created in plugin_init
}

void load_tool(gchar *name)
{
	//load the file of the script into geany
	//and hide the dialog?
}

void execute_tool(gchar *name)
{
	//execute the bash script for the tool, passing it all needed values
	//and handling the output
}

void plugin_init(GeanyData *data)
{
	config = g_key_file_new();
	file = g_strconcat(geany->app->configdir, G_DIR_SEPARATOR_S, "plugins", G_DIR_SEPARATOR_S,
		"external-tools", G_DIR_SEPARATOR_S, "external-tools.conf", NULL);
	g_key_file_load_from_file(config, file, G_KEY_FILE_NONE, NULL);

	//Load existing tools (in groups) 
	gsize len;
	gchar **groups = g_key_file_get_groups(config, &len);
	gsize i;
	
	for(i = 0; i < len; i++)
	{
		printf("loading data for group/tool %s", groups[i]);
		//get values
		gint type = g_key_file_get_integer(config, groups[i], "type", NULL);

		//populate actions in BOTH dialog and setup the shortcuts
		//callbacks will use: g_key_file_set_integer(config, "external-tools", "type", type);
	}
}

void plugin_cleanup(void)
{
	gchar *data = g_key_file_to_data(config, NULL, NULL);
	utils_write_file(file, data);
	g_free(data);
	g_key_file_free(config);
}
