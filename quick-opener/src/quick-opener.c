#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>

GeanyPlugin		 *geany_plugin;
GeanyData			 *geany_data;
GeanyFunctions	*geany_functions;

GtkWidget *dialog, *scrollable, *tree;
GtkTreeStore *list;
GtkTreeIter row;
gint row_pos;
gchar *base_directory;
gint MAX_LIST = 20;
GRegex *name_regex, *path_regex, *trim_path;
GtkAdjustment *adjustment;
GtkTreePath *first, *second;

//Config:
static gchar *conf;
static GKeyFile *config;
static GtkWidget *check_search_path;
static gboolean include_path = FALSE;


typedef struct
{
  const gchar *text;
  GtkWidget *entry;
  const gchar *DEFAULT;
} RegexSetting;
static RegexSetting path = {NULL, NULL, "^\\.|^build$"};
static RegexSetting name = {NULL, NULL, "^\\.|\\.(o|so|exe|class|pyc)$"};

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Opener", "Search filenames while typing", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
	KB_QUICK_OPEN,
	COUNT_KB
};

static GtkWidget *main_menu_item = NULL;

static void submit(
	GtkTreeView *treeview,
	GtkTreePath *path,
	GtkTreeViewColumn *col,
	gpointer user_data
)
{
	GtkTreeSelection *selected = gtk_tree_view_get_selection(treeview);
	GtkTreeIter iter;
	GtkTreeModel *model;	
	if(gtk_tree_selection_get_selected(selected, &model, &iter))
	{
		gchar *path, *name, *file;
		gtk_tree_model_get(model, &iter, 0, &path, 1, &name, -1);
		file = g_build_path(G_DIR_SEPARATOR_S, path, name, NULL);
		document_open_file(file, FALSE, NULL, NULL);
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
		g_free(path);
		g_free(name);
		g_free(file);
	}
}

static void list_files(gchar *base, const gchar *filter, gboolean usePath)
{	
	GDir *dir;
	gchar const *file_name;
	dir = g_dir_open(base, 0, NULL);
	foreach_dir(file_name, dir)
	{
		if(row_pos > MAX_LIST) {
			g_dir_close(dir);
			return;
		}
		gchar *path = g_build_path(G_DIR_SEPARATOR_S, base, file_name, NULL);

		if(g_file_test(path, G_FILE_TEST_IS_DIR)) {
			if(g_regex_match(path_regex, file_name, 0, NULL)) {
				g_free(path);
				continue;
			}
			list_files(path, filter, usePath);
		}
		else {
			if(g_regex_match(name_regex, file_name, 0, NULL)) {
				g_free(path);
				continue;
			}
			GRegex *regex = g_regex_new(filter, G_REGEX_CASELESS, 0, NULL);
			if(regex != NULL && g_regex_match(regex, usePath ? path : file_name, 0, NULL)) {
				gtk_tree_store_append(list, &row, NULL);
				gchar *r_base = g_regex_replace(trim_path, base, -1, 0, "", 0, NULL);
				gchar *b_path = g_strconcat(r_base, "/", NULL);
				gtk_tree_store_set(list, &row, 0, b_path, 1, file_name, -1);
				g_free(r_base);
				g_free(b_path);
				row_pos++;
			}
			g_regex_unref(regex);
		}
		g_free(path);
	}
	g_dir_close(dir);
}

static gboolean onkeypress(GtkEntry *entry, GdkEventKey *event, gpointer user_data)
{
	if(event->keyval == GDK_Down) {
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree), second, NULL, FALSE);
	}
	return FALSE;
}

static void onkeyrelease(GtkEntry *entry, GdkEventKey *event, gpointer user_data)
{
	if(event->keyval == GDK_Return) {
		submit(GTK_TREE_VIEW(tree), NULL, NULL, NULL);
	}
	else {
		row_pos = 0;
		gtk_tree_store_clear(list);
		list_files(base_directory, gtk_entry_get_text(entry), include_path);
		gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));

		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree), first, NULL, FALSE);
	}
}

static void quick_open()
{
	GtkWidget *entry, *label, *hbox;
	GtkTreeViewColumn *path_column, *name_column;
	GtkCellRenderer *renderLeft, *renderRight;

	GeanyProject	 *project	 = geany->app->project;
	if(project) {
		base_directory = project->base_path;
	}
	else {
		base_directory = geany->prefs->default_open_path;
	}

	dialog = gtk_dialog_new_with_buttons(_("Quick Open:"),
		GTK_WINDOW(geany->main_widgets->window),
		GTK_DIALOG_DESTROY_WITH_PARENT,NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 250);

	gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Open"), GTK_RESPONSE_OK);
	gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Cancel"), GTK_RESPONSE_CANCEL);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox, FALSE, FALSE, 0);

	label=gtk_label_new(_("File:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

	entry = gtk_entry_new();
	g_signal_connect(entry, "key-press-event", G_CALLBACK(onkeypress), NULL);
	g_signal_connect(entry, "key-release-event", G_CALLBACK(onkeyrelease), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);
	
	//Table:
	
	list = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));

	path_column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), path_column);
	name_column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), name_column);
	
	renderRight = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_alignment(renderRight, 1.0, 0.0);
	gtk_cell_renderer_set_padding(renderRight, 0, 1);
	g_object_set(renderRight, "foreground", "#777", "foreground-set", TRUE, NULL);
	
	renderLeft = gtk_cell_renderer_text_new();
	gtk_cell_renderer_set_padding(renderLeft, 0, 1);
	
	gtk_tree_view_column_pack_start(path_column, renderRight, TRUE);
	gtk_tree_view_column_add_attribute(path_column, renderRight, "text", 0);
	gtk_tree_view_column_pack_start(name_column, renderLeft, TRUE);
	gtk_tree_view_column_add_attribute(name_column, renderLeft, "text", 1);
		
	g_object_unref(list);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);

	//Scrollable:
	scrollable = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrollable), tree);
	g_signal_connect(tree, "row-activated", G_CALLBACK(submit), NULL);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), scrollable, TRUE, TRUE, 10);
	gtk_widget_show_all(dialog);
	
	adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrollable));
	first = gtk_tree_path_new_from_string("0");
	second = gtk_tree_path_new_from_string("1");

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
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

static void setup_regex()
{
	path_regex = g_regex_new(path.text, G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
	name_regex = g_regex_new(name.text, G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
}

void plugin_init(GeanyData *data)
{
	conf = g_build_path(G_DIR_SEPARATOR_S, geany_data->app->configdir, "plugins", "quick-opener.conf", NULL);
	config = g_key_file_new();
	g_key_file_load_from_file(config, conf, G_KEY_FILE_NONE, NULL);
	path.text = utils_get_setting_string(config, "main", "path-regex", path.DEFAULT);
	name.text = utils_get_setting_string(config, "main", "name-regex", name.DEFAULT);

	setup_regex();
	trim_path = g_regex_new(g_strconcat(G_DIR_SEPARATOR_S, "$", NULL), G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);

	GeanyKeyGroup *key_group;
	key_group = plugin_set_key_group(geany_plugin, "quick_open_keyboard_shortcut", COUNT_KB, NULL);
	keybindings_set_item( key_group, KB_QUICK_OPEN, quick_open_keyboard_shortcut, 0, 0,
		"quick_open_keyboard_shortcut", _("Quick Open..."), NULL);

	main_menu_item = gtk_menu_item_new_with_mnemonic("Quick Open...");
	gtk_widget_show(main_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), main_menu_item);
	g_signal_connect(main_menu_item, "activate", G_CALLBACK(quick_open_menu_callback), NULL);
}

static void dialog_response(GtkDialog *configure, gint response, gpointer user_data)
{
	if(response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY) {
		include_path = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_search_path));
		path.text = gtk_entry_get_text(GTK_ENTRY(path.entry));
		name.text = gtk_entry_get_text(GTK_ENTRY(name.entry));
		g_key_file_set_string(config, "main", "path-regex", path.text);
		g_key_file_set_string(config, "main", "name-regex", name.text);
		
		g_regex_unref(path_regex);
		g_regex_unref(name_regex);
		setup_regex();
	}
}

static void set_default(GtkButton* button, gpointer data)
{
  RegexSetting *setting = data;
	gtk_entry_set_text(GTK_ENTRY(setting->entry), setting->DEFAULT);
}

GtkWidget* plugin_configure(GtkDialog *configure)
{
	g_signal_connect(configure, "response", G_CALLBACK(dialog_response), NULL);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

	check_search_path = gtk_check_button_new_with_label(_("Include path in search?"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_search_path), include_path);
	ui_widget_set_tooltip_text(check_search_path, _("When filtering for files, should the search include the path when finding matches?"));
	gtk_box_pack_start(GTK_BOX(vbox), check_search_path, FALSE, FALSE, 10);
	
	GtkWidget *label_path = gtk_label_new(_("Exclude paths matching this regular expression:"));
	gtk_misc_set_alignment(GTK_MISC(label_path), 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_path, FALSE, FALSE, 2);

	path.entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(path.entry), path.text);
	GtkWidget *hbox_path = gtk_hbox_new(FALSE, 6);
	GtkWidget *button_default_path = gtk_button_new_with_label(_("Default"));
	g_signal_connect(button_default_path, "clicked", G_CALLBACK(set_default), &path);
	gtk_box_pack_start(GTK_BOX(hbox_path), path.entry, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox_path), button_default_path, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_path, FALSE, FALSE, 2);

	GtkWidget *label_name = gtk_label_new(_("Exclude file names matching this regular expression:"));
	gtk_misc_set_alignment(GTK_MISC(label_name), 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_name, FALSE, FALSE, 2);
	
	name.entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(name.entry), name.text);
	GtkWidget *hbox_name = gtk_hbox_new(FALSE, 6);
	GtkWidget *button_default_name = gtk_button_new_with_label(_("Default"));
	g_signal_connect(button_default_name, "clicked", G_CALLBACK(set_default), &name);
	gtk_box_pack_start(GTK_BOX(hbox_name), name.entry, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox_name), button_default_name, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_name, FALSE, FALSE, 2);
	
	gtk_widget_show_all(vbox);
	
	return vbox;
}

void plugin_cleanup(void)
{
	gchar *data = g_key_file_to_data(config, NULL, NULL);
	utils_write_file(conf, data);
	g_free(data);
	g_key_file_free(config);
	
	g_free(conf);
	g_regex_unref(path_regex);
	g_regex_unref(name_regex);

	gtk_widget_destroy(main_menu_item);
}
