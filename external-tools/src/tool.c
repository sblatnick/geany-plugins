#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools;
extern GKeyFile *config;

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
	GError *error = NULL;
	gint status;
	gchar *std_out, *std_err;

	GString *cmd_str = g_string_new("echo $script");
	utils_string_replace_all(cmd_str, "$script",
		g_build_path(G_DIR_SEPARATOR_S, tools, tool->name, NULL));
	gchar *cmd = g_string_free(cmd_str, FALSE);
	cmd = utils_get_locale_from_utf8(cmd);

	if(g_spawn_command_line_sync(
		cmd,
		&std_out,
		&std_err,
		&status,
		&error
	))
	{
		printf("std_out: %s", std_out);
	}
	else {
		printf("std_err: %s", std_err);
		ui_set_statusbar(TRUE, _("ERROR %s: %s (%s)"), cmd, std_err, error->message);
		g_error_free(error);
	}
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
  //Cleanup tools

	//Load existing tools (in groups)
	gsize len;
	gchar **groups = g_key_file_get_groups(config, &len);
	gsize i;

	for(i = 0; i < len; i++)
	{
		printf("loading data for group/tool %s\n", groups[i]);
		Tool *tool = load_tool(groups[i]);
		if(callback != NULL) {
		  callback(tool);
		}
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
