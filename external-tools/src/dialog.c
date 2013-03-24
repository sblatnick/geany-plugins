#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *path, *file;

GtkWidget *scrollable, *tree;
GtkTreeStore *list;
GtkTreeIter row;

void selected(GtkTreeView *view, gpointer data)
{

}

void add_tool(char *title)
{
	gtk_tree_store_append(list, &row, NULL);
	gtk_tree_store_set(list, &row, 0, title, -1);
}

void delete_tool(GtkButton *button, gpointer data)
{
	GtkTreeSelection *selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	GtkTreeIter 		iter;
	GtkTreeModel 		*model;	
	if(gtk_tree_selection_get_selected(selected, &model, &iter))
  {
    gchar *name;
    Tool *tool;
    gtk_tree_model_get(model, &iter, 0, &name, 1, &tool, -1);
    g_free(name);
  }
}

void new_tool(GtkButton *button, gpointer data)
{
	add_tool("New Tool");
}

GtkWidget *plugin_configure(GtkDialog *dialog)
{
	GtkCellRenderer *render;
	GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
	GtkWidget *hbox = gtk_hbox_new(FALSE, 6);
	GtkWidget *buttonBox = gtk_hbox_new(FALSE, 6);

	list = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
	render = gtk_cell_renderer_text_new();

	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Tool Name", render, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	gtk_tree_view_column_pack_start(column, render, TRUE);
	gtk_tree_view_column_add_attribute(column, render, "text", 0);
	g_object_unref(GTK_TREE_MODEL(list));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);

	scrollable = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrollable), tree);
	g_signal_connect(tree, "changed", G_CALLBACK(selected), NULL);

	GtkWidget *delete = gtk_button_new_with_label("Delete");
	GtkWidget *new = gtk_button_new_with_label("New");
	g_signal_connect(delete, "clicked", G_CALLBACK(delete_tool), NULL);
	g_signal_connect(new, "clicked", G_CALLBACK(new_tool), NULL);

	gtk_box_pack_start(GTK_BOX(buttonBox), delete, FALSE, FALSE, 10);
	gtk_box_pack_end(GTK_BOX(buttonBox), new, FALSE, FALSE, 10);

	gtk_box_pack_start(GTK_BOX(vbox), scrollable, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), buttonBox, FALSE, FALSE, 10);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 10);
	gtk_widget_show_all(hbox);

	return hbox;
}
