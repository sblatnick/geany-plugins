#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

enum OUTPUT
{
	MESSAGE_PANEL = 0,
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
	gboolean context;
	gboolean menu;
	gboolean save;
} Tool;

Tool* load_tool(gchar *name, GKeyFile *config)
{
	Tool *tool;
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

