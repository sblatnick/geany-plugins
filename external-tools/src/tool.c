#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools;
extern GKeyFile *config;
extern gint shortcutCount;
extern GeanyKeyGroup *key_group;

enum OUTPUT
{
  NOTHING = 0,
	MESSAGE_PANEL,
	TABLE_PANEL,
	REPLACE_SELECTED,
	REPLACE_LINE,
	REPLACE_WORD,
	APPEND_CURRENT,
	NEW_DOCUMENT
} output;

typedef struct
{
	gchar *name;
	gint output;

	gboolean save;
	gboolean context;
	gboolean menu;
	gboolean shortcut;
} Tool;

enum
{
  SAVE = 0,
	CONTEXT,
	MENU,
	SHORTCUT
};

Tool* new_tool()
{
  gchar* orig = g_strdup(_("New Tool"));
  gchar* name = g_strdup(_("New Tool"));
  gint count = 2;
  gchar* new = g_build_path(G_DIR_SEPARATOR_S, tools, orig, NULL);
  while(g_file_test(new, G_FILE_TEST_EXISTS)) {
		printf("new: %s\n", new);
		char counter[32];
    snprintf(counter, 32, "%d", count);
    setptr(name, g_strconcat(orig, " ", counter, NULL));
    printf("try: %d\n", count);
		setptr(new, g_build_path(G_DIR_SEPARATOR_S, tools, name, NULL));
    printf("iterate\n");
    count++;
	}
	g_creat(new, 0744);
	g_free(orig);
	g_free(new);

  Tool init = {name, -1, FALSE, FALSE, FALSE, FALSE};
  Tool *tool = g_slice_new(Tool);
  *tool = init;
  return tool;
}

void free_tool(Tool* tool)
{
  g_free(tool->name);
  g_slice_free(Tool, tool);
}

void execute(Tool *tool)
{
  printf("TOOL EXECUTED: %s\n", tool->name);
  
  const char *home = g_getenv("HOME");
  if (!home) {
    home = g_get_home_dir();
  }
  
	GError *error;
	gint std_in, std_out, std_err;
	GPid pid;

	GString *cmd_str = g_string_new("$script");
	utils_string_replace_all(cmd_str, "$script", g_build_path(G_DIR_SEPARATOR_S, tools, tool->name, NULL));
	gchar *cmd = g_string_free(cmd_str, FALSE);
	cmd = utils_get_locale_from_utf8(cmd);

	if(g_spawn_async_with_pipes(
    home,
    &cmd,
    NULL,
    0, NULL, NULL,
    &pid,
    &std_in,
    &std_out,
    &std_err,
    &error
	))
	{
		FILE *fp = fdopen(std_out, "r");
		gchar line[256];
    while(fgets(line, sizeof line, fp) != NULL )
    {
      printf(line, stdout);
      printf("line: %s", line);
    }
    fclose(fp);
	}
	else {
		printf("ERROR %s: %s (%d, %d, %d)", cmd, error->message, std_in, std_out, std_err);
		ui_set_statusbar(TRUE, _("ERROR %s: %s (%d, %d, %d)"), cmd, error->message, std_in, std_out, std_err);
		g_error_free(error);
	}
}

static void tool_menu_callback(GtkToggleButton *cb, gpointer data)
{
  execute(data);
}

int setup_tool(Tool* tool)
{
  if(tool->shortcut) {
    //Store tool in an array
    shortcutCount++;
  }
  if(tool->context) {
    //TODO
  }
  if(tool->menu) {
    GtkWidget *tool_menu_item = gtk_menu_item_new_with_mnemonic(tool->name);
    gtk_widget_show(tool_menu_item);
    gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), tool_menu_item);
    g_signal_connect(tool_menu_item, "activate", G_CALLBACK(tool_menu_callback), tool);
  }
}

void clean_tools()
{

}

Tool* load_tool(gchar *name)
{
  Tool init = {
    name,
    g_key_file_get_integer(config, name, "output", NULL),
    g_key_file_get_boolean(config, name, "save", NULL),
    g_key_file_get_boolean(config, name, "context", NULL),
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
		printf("loading data for group/tool %s\n", groups[i]);
		Tool *tool = load_tool(groups[i]);
		callback(tool);
	}
}

void save_tool(Tool* tool)
{
	g_key_file_set_integer(config, tool->name, "output", tool->output);
	g_key_file_set_boolean(config, tool->name, "save", tool->save);
	g_key_file_set_boolean(config, tool->name, "context", tool->context);
	g_key_file_set_boolean(config, tool->name, "menu", tool->menu);
	g_key_file_set_boolean(config, tool->name, "shortcut", tool->shortcut);
}

void load_script(Tool *tool)
{
	//load the file of the script into geany
	//and hide the dialog?
}
