#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *path, *file;

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
  Tool init = {g_strdup(_("New Tool")), -1, FALSE, FALSE, FALSE, FALSE};
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
		g_strconcat(path, G_DIR_SEPARATOR, tool->name, NULL));
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


Tool* load_tool(gchar *name, GKeyFile *config)
{
	Tool *tool = new_tool();
	tool->name = name;
	tool->output = g_key_file_get_integer(config, name, "output", NULL);
	tool->context = g_key_file_get_boolean(config, name, "context", NULL);
	tool->menu = g_key_file_get_boolean(config, name, "menu", NULL);
	tool->save = g_key_file_get_boolean(config, name, "save", NULL);
	return tool;
}

void load_script(Tool *tool)
{
	//load the file of the script into geany
	//and hide the dialog?
}
