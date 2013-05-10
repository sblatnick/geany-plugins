#include "tool.h"
#include "output.c"

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools, *conf;
static GKeyFile *config;
static GeanyKeyGroup *key_group;
static gint shortcutCount = 0;
static gint menuCount = 0;
static GtkWidget **menu_tools;

static void key_callback(G_GNUC_UNUSED guint key_id)
{
	plugin_show_configure(geany_plugin);
}

Tool* new_tool()
{
	gchar* orig = g_strdup(_("New Tool"));
	gchar* name = g_strdup(_("New Tool"));
	gint count = 2;
	gchar* new = g_build_path(G_DIR_SEPARATOR_S, tools, orig, NULL);
	while(g_file_test(new, G_FILE_TEST_EXISTS)) {
		char counter[32];
		snprintf(counter, 32, "%d", count);
		setptr(name, g_strconcat(orig, " ", counter, NULL));
		setptr(new, g_build_path(G_DIR_SEPARATOR_S, tools, name, NULL));
		count++;
	}
	g_creat(new, 0744);
	g_free(orig);
	g_free(new);

	Tool init = {name, -1, FALSE, FALSE, FALSE};
	Tool *tool = g_slice_new(Tool);
	*tool = init;
	return tool;
}

void free_tool(Tool* tool)
{
	g_key_file_remove_group(config, tool->name, NULL);
	g_free(tool->name);
	g_slice_free(Tool, tool);
}

static void tool_menu_callback(GtkToggleButton *cb, gpointer data)
{
	execute(data);
}

static void tool_shortcut_callback(G_GNUC_UNUSED guint key_id)
{
	execute(shortcut_tools[key_id]);
}

int count_tools(Tool* tool)
{
	if(tool->shortcut) {
		shortcutCount++;
	}
	if(tool->menu) {
		menuCount++;
	}
}

int setup_tools(Tool* tool)
{
	if(tool->shortcut) {
		shortcut_tools[shortcutCount] = tool;
		keybindings_set_item(key_group, shortcutCount, tool_shortcut_callback, 0, 0,
			tool->name, tool->name, NULL);
		shortcutCount++;
	}
	if(tool->menu) {
		GtkWidget *tool_menu_item = gtk_menu_item_new_with_mnemonic(tool->name);
		menu_tools[menuCount] = tool_menu_item;
		gtk_widget_show(tool_menu_item);
		gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), tool_menu_item);
		g_signal_connect(tool_menu_item, "activate", G_CALLBACK(tool_menu_callback), tool);
		menuCount++;
	}
}

void clean_tools()
{
	gchar *data = g_key_file_to_data(config, NULL, NULL);
	utils_write_file(conf, data);
	g_free(data);
	g_key_file_free(config);

	gint i = 0;
	while(i < menuCount) {
		gtk_container_remove(GTK_CONTAINER(geany->main_widgets->tools_menu), menu_tools[i]);
		i++;
	}

	shortcutCount = 0;
	menuCount = 0;
	g_free(shortcut_tools);
	g_free(menu_tools);
}

Tool* load_tool(gchar *name)
{
	Tool init = {
		name,
		g_key_file_get_integer(config, name, "output", NULL),
		g_key_file_get_boolean(config, name, "save", NULL),
		g_key_file_get_boolean(config, name, "menu", NULL),
		g_key_file_get_boolean(config, name, "shortcut", NULL)
	};
	Tool *tool = g_slice_new(Tool);
	*tool = init;
	return tool;
}

void load_tools(int (*callback)(Tool*))
{
	if(callback == NULL) {
		return;
	}

	//Load existing tools (in groups)
	gsize len;
	gchar **groups = g_key_file_get_groups(config, &len);
	gsize i;

	for(i = 0; i < len; i++)
	{
		Tool *tool = load_tool(groups[i]);
		callback(tool);
	}
}

void reload_tools()
{
	config = g_key_file_new();
	g_key_file_load_from_file(config, conf, G_KEY_FILE_NONE, NULL);
	load_tools(count_tools);
	key_group = plugin_set_key_group(geany_plugin, "external_tools_keyboard_shortcut", shortcutCount + 1, NULL);
	shortcut_tools = (Tool **) g_malloc(shortcutCount);
	menu_tools = (GtkWidget **) g_malloc(menuCount);
	shortcutCount = 0;
	menuCount = 0;
	load_tools(setup_tools);
	keybindings_set_item(key_group, shortcutCount, key_callback, 0, 0, "external_tools_keyboard_shortcut", _("External Tools..."), NULL);
	keybindings_load_keyfile();
}

int save_tool(Tool* tool)
{
	g_key_file_set_integer(config, tool->name, "output", tool->output);
	g_key_file_set_boolean(config, tool->name, "save", tool->save);
	g_key_file_set_boolean(config, tool->name, "menu", tool->menu);
	g_key_file_set_boolean(config, tool->name, "shortcut", tool->shortcut);
}
