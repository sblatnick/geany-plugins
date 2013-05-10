
static Tool *executed_tool;

static void output_init()
{
	switch(executed_tool->output) {
		case TOOL_OUTPUT_NONE:
			break;
		case TOOL_OUTPUT_MESSAGE_TEXT:
			break;
		case TOOL_OUTPUT_MESSAGE_TABLE:
			break;
		case TOOL_OUTPUT_REPLACE_SELECTED:
			break;
		case TOOL_OUTPUT_REPLACE_LINE:
			break;
		case TOOL_OUTPUT_REPLACE_WORD:
			break;
		case TOOL_OUTPUT_APPEND_CURRENT_DOCUMENT:
			break;
		case TOOL_OUTPUT_NEW_DOCUMENT:
			break;
	}
}

static gboolean output_out(GIOChannel *channel, GIOCondition cond, gpointer type)
{
	gchar *string;
	gchar *err;

	if(cond == G_IO_HUP)
	{
		g_io_channel_unref(channel);
		return FALSE;
	}
	
	GIOStatus st;
	while ((st = g_io_channel_read_line(channel, &string, NULL, NULL, NULL)) == G_IO_STATUS_NORMAL && string)
	{
		switch(executed_tool->output) {
			case TOOL_OUTPUT_NONE:
				break;
			case TOOL_OUTPUT_MESSAGE_TEXT:
				panel_print(string, GPOINTER_TO_UINT(type) == 1 ? "error" : NULL);
				break;
			case TOOL_OUTPUT_MESSAGE_TABLE:
				break;
			case TOOL_OUTPUT_REPLACE_SELECTED:
				break;
			case TOOL_OUTPUT_REPLACE_LINE:
				break;
			case TOOL_OUTPUT_REPLACE_WORD:
				break;
			case TOOL_OUTPUT_APPEND_CURRENT_DOCUMENT:
				break;
			case TOOL_OUTPUT_NEW_DOCUMENT:
				break;
		}
		g_free(string);	
	}

	return TRUE;
}

void execute(Tool *tool)
{
	executed_tool = tool;
	output_init();
	printf("TOOL EXECUTED: %s\n", tool->name);
	panel_prepare();

	GeanyDocument *doc = document_get_current();
	if(tool->save) {
		document_save_file(doc, TRUE);
	}

	const char *home = g_getenv("HOME");
	if (!home) {
		home = g_get_home_dir();
	}

	GError *error = NULL;
	gint std_in, std_out, std_err;
	GPid pid;

	GString *cmd_str = g_string_new("$script");
	utils_string_replace_all(cmd_str, "$script", g_build_path(G_DIR_SEPARATOR_S, tools, tool->name, NULL));
	gchar *cmd = g_string_free(cmd_str, FALSE);
	cmd = utils_get_locale_from_utf8(cmd);

	char geany_line_number[32];
	gint line = sci_get_current_line(doc->editor->sci);
	snprintf(geany_line_number, 32, "%d", line + 1);

	gchar **argv = utils_copy_environment(
		NULL,
		"GEANY_LINE_NUMBER", geany_line_number,
		"GEANY_SELECTION", sci_get_selection_contents(doc->editor->sci),
		"GEANY_SELECTED_LINE", sci_get_line(doc->editor->sci, line),
		"GEANY_FILE_PATH", doc->file_name,
		"GEANY_FILE_MIME_TYPE", doc->file_type->mime_type,
		"GEANY_FILE_TYPE_NAME", doc->file_type->name,
		NULL
	);

	if(g_spawn_async_with_pipes(
		home,
		&cmd,
		argv,
		0, NULL, NULL,
		&pid,
		&std_in,
		&std_out,
		&std_err,
		&error
	))
	{
		#ifdef G_OS_WIN32
			GIOChannel err_channel = g_io_channel_win32_new_fd(std_err);
			GIOChannel *out_channel = g_io_channel_win32_new_fd(std_out);
		#else
			GIOChannel *err_channel = g_io_channel_unix_new(std_err);
			GIOChannel *out_channel = g_io_channel_unix_new(std_out);
		#endif
		
		g_io_add_watch(out_channel, G_IO_IN | G_IO_HUP, (GIOFunc)output_out, GUINT_TO_POINTER(0));
		g_io_add_watch(err_channel, G_IO_IN | G_IO_HUP, (GIOFunc)output_out, GUINT_TO_POINTER(1));
	}
	else {
		printf("ERROR %s: %s (%d, %d, %d)", cmd, error->message, std_in, std_out, std_err);
		ui_set_statusbar(TRUE, _("ERROR %s: %s (%d, %d, %d)"), cmd, error->message, std_in, std_out, std_err);
		g_error_free(error);
	}
	g_free(cmd);
	g_free(argv);
}
