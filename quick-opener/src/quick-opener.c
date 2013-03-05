#include <geanyplugin.h>

#define foreach_slist_free(node, list) for (node = list, list = NULL; g_slist_free_1(list), node != NULL; list = node, node = node->next)

GeanyPlugin     *geany_plugin;
GeanyData       *geany_data;
GeanyFunctions  *geany_functions;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Opener", "Search filenames while typing", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
	KB_QUICK_OPEN,
	COUNT_KB
};

static GtkWidget *main_menu_item = NULL;

static void search(gchar *value, gchar *dir)
{
	gchar *cmd, *locale_cmd, *c;
	GString *cmd_str = g_string_new("notify-send \"$str\" \"$dir\"");
	GError *error = NULL;
	
	utils_string_replace_all(cmd_str, "$str", value);
	utils_string_replace_all(cmd_str, "$dir", dir);

	cmd = g_string_free(cmd_str, FALSE);
	locale_cmd = utils_get_locale_from_utf8(cmd);
	if(!g_spawn_command_line_async(locale_cmd, &error))
	{
		c = strchr(cmd, ' ');
		if(c != NULL) {
			*c = '\0';
		}
		ui_set_statusbar(TRUE, _("Could not execute configured external command '%s' (%s)."), cmd, error->message);
		g_error_free(error);
	}
}

static void onkeypress(GtkEntry *entry)
{
	GSList *list, *node;
	gchar *name, *uri;
	gboolean is_dir;
  gchar* text = gtk_entry_get_text(GTK_ENTRY(entry));  

	list = utils_get_file_list_full("/home/steve/Desktop/", TRUE, TRUE, NULL);
	if(list != NULL) {
	  foreach_slist_free(node, list) {
			name = node->data;
			uri = g_strconcat("/home/steve/Desktop/", name, NULL);
			is_dir = g_file_test(uri, G_FILE_TEST_IS_DIR);
			search("pressed!", utils_get_utf8_from_locale(name));
		}
	}
}

static void quick_open()
{
	dialogs_show_msgbox(GTK_MESSAGE_INFO, "QuickSearch");
	return;
	GtkWidget *dialog, *entry, *label, *hbox;

	gchar *dir;
  GeanyProject 	*project 	= geany->app->project;
	if(project) {
		dir = project->base_path;
	}
	else {
		dir = geany->prefs->default_open_path;
	}

  dialog = gtk_dialog_new_with_buttons(
    _("Snap Open:"),
    GTK_WINDOW(geany->main_widgets->window),
    GTK_DIALOG_DESTROY_WITH_PARENT,NULL
  );
  gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Ok"), GTK_RESPONSE_OK);
	gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Cancel"), GTK_RESPONSE_CANCEL);
	
	hbox=gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("File:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label);

	entry = gtk_entry_new();
	g_signal_connect(entry, "changed", G_CALLBACK(onkeypress), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
	gtk_widget_show(entry);

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	
  if(response == GTK_RESPONSE_OK) {
	  document_open_file("/home/steve/geany-plugins/quicksearch/make", FALSE, NULL, NULL);
	}
  gtk_widget_destroy(dialog);
}

static void search_dialog_cb(GtkMenuItem *menuitem, gpointer gdata)
{
	quick_open();
}

static void quick_open_cb(G_GNUC_UNUSED guint key_id)
{
	quick_open();
}

void plugin_init(GeanyData *data)
{
	GeanyKeyGroup *key_group;
	key_group = plugin_set_key_group(geany_plugin, "quick_open_cb", COUNT_KB, NULL);
	keybindings_set_item( key_group, KB_QUICK_OPEN, quick_open_cb, 0, 0,
		"quick_open_cb", _("Quick Open..."), NULL);

	main_menu_item = gtk_menu_item_new_with_mnemonic("Quick Open...");
	gtk_widget_show(main_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), main_menu_item);
	g_signal_connect(main_menu_item, "activate", G_CALLBACK(search_dialog_cb), NULL);
}

void plugin_cleanup(void)
{
	gtk_widget_destroy(main_menu_item);
}

