#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

GeanyPlugin		 *geany_plugin;
GeanyData			 *geany_data;
GeanyFunctions	*geany_functions;

static GtkWidget *entry, *panel, *label, *scrollable_table, *tree;
GtkTreeStore *list;
GtkTreeIter row;
gint row_pos;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Find", "Quickly search documents based on the treebrowser root or project root using ack-grep.", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
	KB_QUICK_FIND,
	KB_GROUP
};

static gboolean panel_focus_tab(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	GeanyKeyBinding *kb = keybindings_lookup_item(GEANY_KEY_GROUP_FOCUS, GEANY_KEYS_FOCUS_SIDEBAR);
	if (kb != NULL && event->key.keyval == kb->key && (event->key.state & gtk_accelerator_get_default_mod_mask()) == kb->mods) {
		gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook));
		gint tab = gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook), panel);
		if(current == tab) {
			gtk_widget_grab_focus(entry);
			//TODO: or grab table entries, whatever was last focused
		}
	}
	return FALSE;
}

static void quick_find()
{
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
  gchar *base_directory;
  
  GeanyProject *project	= geany->app->project;
	if(project) {
		base_directory = project->base_path;
	}
	else {
		base_directory = geany->prefs->default_open_path;
	}
  
  //search files
}

static void entry_focus(G_GNUC_UNUSED guint key_id)
{
  gtk_notebook_set_current_page(
		GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook), panel)
	);
	gtk_widget_grab_focus(entry);
}

static gboolean on_activate(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  quick_find();
	return FALSE;
}

static void on_click(GtkButton* button, gpointer data)
{
	quick_find();
}

static void selected_row(
	GtkTreeView *treeview,
	GtkTreePath *path,
	GtkTreeViewColumn *col,
	gpointer user_data
)
{
  printf("selected_row\n");
}

void plugin_init(GeanyData *data)
{
	label = gtk_label_new(_("Find"));
	panel = gtk_vbox_new(FALSE, 6);
	scrollable_table = gtk_scrolled_window_new(NULL, NULL);

	gtk_box_pack_start(GTK_BOX(panel), scrollable_table, TRUE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(geany->main_widgets->sidebar_notebook), panel, label);
	
	entry = gtk_entry_new();
	gtk_entry_set_icon_from_stock(GTK_ENTRY(entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);
	g_signal_connect(entry, "activate", G_CALLBACK(on_activate), NULL);
	
	GtkWidget *button_box = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(button_box), entry, TRUE, TRUE, 0);
	
	GtkWidget *button = gtk_button_new_with_label(_("Find"));
	g_signal_connect(button, "clicked", G_CALLBACK(on_click), NULL);
	gtk_box_pack_end(GTK_BOX(button_box), button, FALSE, TRUE, 0);
	
	gtk_box_pack_end(GTK_BOX(panel), button_box, FALSE, TRUE, 0);
	gtk_container_set_focus_child(GTK_CONTAINER(panel), entry);

	GtkTreeViewColumn *number_column, *line_column, *file_column;
	GtkCellRenderer *render;
  
  list = gtk_tree_store_new(3, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));

  number_column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), number_column);
	line_column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), line_column);
	file_column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), file_column);
	
	render = gtk_cell_renderer_text_new();
	
	gtk_tree_view_column_pack_start(number_column, render, TRUE);
	gtk_tree_view_column_pack_start(line_column, render, TRUE);
	gtk_tree_view_column_pack_start(file_column, render, TRUE);
	//gtk_tree_view_column_add_attribute(number_column, render, "text", 0);
		
	g_object_unref(GTK_TREE_MODEL(list));
	g_signal_connect(tree, "row-activated", G_CALLBACK(selected_row), NULL);
	
	gtk_container_add(GTK_CONTAINER(scrollable_table), tree);
  gtk_widget_show(label);
	gtk_widget_show_all(panel);
	
	g_signal_connect(geany->main_widgets->window, "key-release-event", G_CALLBACK(panel_focus_tab), NULL);

	GeanyKeyGroup *key_group;
	key_group = plugin_set_key_group(geany_plugin, "quick_find_keyboard_shortcut", KB_GROUP, NULL);
	keybindings_set_item(key_group, KB_QUICK_FIND, entry_focus, 0, 0, "quick_find", _("Quick Find..."), NULL);
}

void plugin_cleanup(void)
{

}

