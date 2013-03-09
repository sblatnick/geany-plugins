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
GRegex *name_regex, *path_regex;

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
	GtkTreeIter 		iter;
	GtkTreeModel 		*model;	
	if(gtk_tree_selection_get_selected(selected, &model, &iter))
  {
    gchar *path, *name, *file;
    gtk_tree_model_get(model, &iter, 0, &path, 1, &name, -1);
    file = g_build_path(G_DIR_SEPARATOR_S, path, name, NULL);
		document_open_file(file, FALSE, NULL, NULL);
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    g_free(path);
    g_free(name);
  }
}

static void list_files(gchar *base, const gchar *filter)
{	
	GDir *dir;
	gchar const *file_name;
	gchar *path;
	dir = g_dir_open(base, 0, NULL);
	foreach_dir(file_name, dir)
	{
		if(row_pos > MAX_LIST) {
			return;
		}
		path = g_build_path(G_DIR_SEPARATOR_S, base, file_name, NULL);

		if(g_file_test(path, G_FILE_TEST_IS_DIR)) {
			if(g_regex_match(path_regex, file_name, 0, NULL)) {
				continue;
			}
			list_files(path, filter);
		}
		else {
			if(g_regex_match(name_regex, file_name, 0, NULL)) {
				continue;
			}
			if(g_strrstr(file_name, filter) > 0) {
				gtk_tree_store_append(list, &row, NULL);
				gtk_tree_store_set(list, &row, 0, base, 1, file_name, -1);
				row_pos++;
			}
		}
	}
	g_dir_close(dir);
}

static gboolean onkeypress(GtkEntry *entry, GdkEventKey *event, gpointer user_data)
{
	if(event->keyval == 65364) {
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree), gtk_tree_path_new_from_string("1"), NULL, FALSE);
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
		list_files(base_directory, gtk_entry_get_text(entry));
		GtkAdjustment *adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrollable));
		gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));

		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree), gtk_tree_path_new_from_string("0"), NULL, FALSE);
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
	gtk_widget_set_size_request(dialog, 500, 250);

	gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Open"), GTK_RESPONSE_OK);
	gtk_dialog_add_button(GTK_DIALOG(dialog),_("_Cancel"), GTK_RESPONSE_CANCEL);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("File:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label);

	entry = gtk_entry_new();
	g_signal_connect(entry, "key-press-event", G_CALLBACK(onkeypress), NULL);
	g_signal_connect(entry, "key-release-event", G_CALLBACK(onkeyrelease), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);
	gtk_widget_show(entry);
	
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
		
	g_object_unref(GTK_TREE_MODEL(list));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);

	//Scrollable:
	scrollable = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrollable), tree);
	g_signal_connect(tree, "row-activated", G_CALLBACK(submit), NULL);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), scrollable, TRUE, TRUE, 10);
	gtk_widget_show_all(dialog);

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

void plugin_init(GeanyData *data)
{
	GeanyKeyGroup *key_group;
	path_regex = g_regex_new("^\\.|^build$", G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
	name_regex = g_regex_new("^\\.|\\.(o|so|exe|class|pyc)$", G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
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

