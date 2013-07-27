extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools;
extern const gchar *home;
extern GtkWidget *panel;

static Tool *executed_tool;

static void output_focus()
{
	//Focus the tab:
	gtk_notebook_set_current_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook), panel)
	);
}

static void output_prepare()
{
	if(executed_tool->output != TOOL_OUTPUT_NONE) {
		output_focus();
		panel_prepare();
		if(executed_tool->output == TOOL_OUTPUT_REPLACE_LINE) {
			GeanyDocument *doc = document_get_current();
			gint line = sci_get_current_line(doc->editor->sci);
			gint length = sci_get_lines_selected(doc->editor->sci) - 1;
			gint start = sci_get_pos_at_line_sel_start(doc->editor->sci, line);
			gint end = sci_get_pos_at_line_sel_end(doc->editor->sci, line + length);
			sci_set_selection_start(doc->editor->sci, start);
			sci_set_selection_end(doc->editor->sci, end);
		}
		else {
			switch(executed_tool->output) {
				case TOOL_OUTPUT_MESSAGE_TABLE:
					table_prepare();
					break;
				case TOOL_OUTPUT_NEW_DOCUMENT:
					document_new_file(NULL, NULL, NULL);
					break;
			}
		}
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

	GeanyDocument *doc = document_get_current();

	GIOStatus st;
	if((st = g_io_channel_read_line(channel, &string, NULL, NULL, NULL)) == G_IO_STATUS_NORMAL && string)
	{
		if(GPOINTER_TO_UINT(type) == 1) {
			output_focus();
			panel_print(string, "error");
		}
		else {
			switch(executed_tool->output) {
				case TOOL_OUTPUT_NONE:
					break;
				case TOOL_OUTPUT_MESSAGE_TEXT:
					panel_print(string, NULL);
					break;
				case TOOL_OUTPUT_MESSAGE_TABLE:
					table_print(string, NULL);
					break;
				case TOOL_OUTPUT_REPLACE_SELECTED:
				case TOOL_OUTPUT_REPLACE_LINE:
					sci_replace_sel(doc->editor->sci, string);
					break;
				case TOOL_OUTPUT_REPLACE_WORD:
					break;
				case TOOL_OUTPUT_APPEND_CURRENT_DOCUMENT:
				case TOOL_OUTPUT_NEW_DOCUMENT:
					sci_insert_text(doc->editor->sci, sci_get_length(doc->editor->sci), string);
					break;
			}
		}
		g_free(string);
	}

	return TRUE;
}

void execute(Tool *tool)
{
	executed_tool = tool;
	printf("TOOL EXECUTED: %s\n", tool->name);
	output_prepare();

	GeanyDocument *doc = document_get_current();
	if(tool->save) {
		document_save_file(doc, TRUE);
	}

	GError *error = NULL;
	gint std_out, std_err;

	gchar *cmd = tool->id;

	char geany_line_number[32];
	gint line = sci_get_current_line(doc->editor->sci);
	snprintf(geany_line_number, 32, "%d", line + 1);

	gchar *project_dir;
  GeanyProject *project = geany->app->project;
	if(project) {
		project_dir = project->base_path;
	}
	else {
		project_dir = geany->prefs->default_open_path;
	}

	gchar **env = utils_copy_environment(
		NULL,
		"GEANY_LINE_NUMBER", geany_line_number,
		"GEANY_SELECTION", sci_get_selection_contents(doc->editor->sci),
		"GEANY_SELECTED_LINE", sci_get_line(doc->editor->sci, line),
		"GEANY_FILE_PATH", doc->file_name,
		"GEANY_FILE_MIME_TYPE", doc->file_type->mime_type,
		"GEANY_FILE_TYPE_NAME", doc->file_type->name,
		"GEANY_PROJECT_DIRECTORY", project_dir,
		NULL
	);

	gchar **argv;
	if(!g_shell_parse_argv(cmd, NULL, &argv, &error))
	{
		ui_set_statusbar(TRUE, _("Tool failed: %s"), error->message);
		g_error_free(error);
		return;
	}

	if(g_spawn_async_with_pipes(
		home,
		argv,
		env,
		0, NULL, NULL, NULL, NULL,
		&std_out,
		&std_err,
		&error
	))
	{
		#ifdef G_OS_WIN32
			GIOChannel *err_channel = g_io_channel_win32_new_fd(std_err);
			GIOChannel *out_channel = g_io_channel_win32_new_fd(std_out);
		#else
			GIOChannel *err_channel = g_io_channel_unix_new(std_err);
			GIOChannel *out_channel = g_io_channel_unix_new(std_out);
		#endif

		g_io_add_watch(out_channel, G_IO_IN | G_IO_HUP, (GIOFunc)output_out, GUINT_TO_POINTER(0));
		g_io_add_watch(err_channel, G_IO_IN | G_IO_HUP, (GIOFunc)output_out, GUINT_TO_POINTER(1));
	}
	else {
		printf("ERROR %s: %s (%d, %d)\n", cmd, error->message, std_out, std_err);
		ui_set_statusbar(TRUE, _("ERROR %s: %s (%d, %d, %d)"), cmd, error->message, std_out, std_err);
		g_error_free(error);
	}
	g_free(env);
}
