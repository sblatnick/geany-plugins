#include <geanyplugin.h>

GeanyPlugin     *geany_plugin;
GeanyData       *geany_data;
GeanyFunctions  *geany_functions;

GtkWidget *dialog;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Opener", "Search filenames while typing", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
	KB_QUICK_OPEN,
	COUNT_KB
};

static GtkWidget *main_menu_item = NULL;

static void onkeypress(GtkEntry *entry)
{
	printf("keypress\n");
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}

static void quick_open()
{
	GtkWidget *entry, *label, *hbox;

	gchar *dir;
  GeanyProject 	*project 	= geany->app->project;
	if(project) {
		dir = project->base_path;
	}
	else {
		dir = geany->prefs->default_open_path;
	}

  dialog = gtk_dialog_new_with_buttons(_("Quick Open:"),
  	GTK_WINDOW(geany->main_widgets->window),
  	GTK_DIALOG_DESTROY_WITH_PARENT,NULL);

  gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Open"), GTK_RESPONSE_OK);
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

static void quick_open_menu_callback(GtkMenuItem *menuitem, gpointer gdata)
{
	quick_open();
}

static void quick_open_keyboard_shortcut(G_GNUC_UNUSED guint key_id)
{
	quick_open();
}

void plugin_init(GeanyData *data)
{
	GeanyKeyGroup *key_group;
	key_group = plugin_set_key_group(geany_plugin, "quick_open_keyboard_shortcut", COUNT_KB, NULL);
	keybindings_set_item( key_group, KB_QUICK_OPEN, quick_open_keyboard_shortcut, 0, 0,
		"quick_open_keyboard_shortcut", _("Quick Open..."), NULL);

	main_menu_item = gtk_menu_item_new_with_mnemonic("Quick Open...");
	gtk_widget_show(main_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), main_menu_item);
	g_signal_connect(main_menu_item, "activate", G_CALLBACK(quick_open_menu_callback), NULL);
}

void plugin_cleanup(void)
{
	gtk_widget_destroy(main_menu_item);
}

