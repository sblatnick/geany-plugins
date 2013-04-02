#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *path, *file;

static GtkWidget *scrollable, *tree, *saveCheckbox, *contextCheckbox, *menuCheckbox, *shortcutCheckbox, *title;
GtkTreeStore *list;
GtkTreeIter row;

Tool* get_active_tool()
{
	GtkTreeSelection *selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(selected, &model, &iter)) {
    Tool *tool;
    gtk_tree_model_get(model, &iter, 0, &tool, -1);
    return tool;
  }
  else {
    return NULL;
  }
}

void selected_changed(GtkTreeView *view, gpointer data)
{
  Tool* tool = get_active_tool();
  printf("Tool name: %s save: %s\n", tool->name, tool->save ? "true" : "false");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(saveCheckbox), tool->save);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(contextCheckbox), tool->context);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(menuCheckbox), tool->menu);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shortcutCheckbox), tool->shortcut);
  gtk_entry_set_text(GTK_ENTRY(title), tool->name);
}

void add_tool(Tool *tool)
{
	gtk_tree_store_append(list, &row, NULL);
	gtk_tree_store_set(list, &row, 0, tool, -1);
}

void delete_tool(GtkButton *button, gpointer data)
{
	GtkTreeSelection *selected = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(selected, &model, &iter))
  {
    Tool *tool;
    gtk_tree_model_get(model, &iter, 0, &tool, -1);
    g_free(tool);
  }
}

void new_tool_entry(GtkButton *button, gpointer data)
{
	Tool *tool = new_tool();
	tool->name = _("New Tool");
	tool->save = FALSE;
	add_tool(tool);
}

void cell_data(GtkTreeViewColumn *tree_column, GtkCellRenderer *render,
  GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
  Tool *tool;
  gtk_tree_model_get(model, iter, 0, &tool, -1);
  g_object_set(render, "text", tool->name, NULL);
}

void toggle_checkbox(GtkToggleButton *checkbox, gpointer data)
{
  Tool* tool = get_active_tool();
  if(tool != NULL) {
    gboolean value = gtk_toggle_button_get_active(checkbox);
    gint property = GPOINTER_TO_INT(data);
    switch(property) {
      case SAVE:
        tool->save = value;
        break;
      case CONTEXT:
        tool->context = value;
        break;
      case MENU:
        tool->menu = value;
        break;
      case SHORTCUT:
        tool->shortcut = value;
        break;
    }
    printf("Toggle: %d: %s\n", property, value ? "true" : "false");
  }
}

GtkWidget* checkbox(gchar *label, gchar *tooltip, gchar *key)
{
	GtkWidget *save_check = gtk_check_button_new_with_label(_(label));
	ui_widget_set_tooltip_text(save_check, _(tooltip));
	g_signal_connect(G_OBJECT(save_check), "toggled", G_CALLBACK(toggle_checkbox), key);
	return save_check;
}

void dialog_response(GtkDialog *dialog, gint response, gpointer user_data)
{
  if(! (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY)) {
		return;
	}
}

static gboolean on_change(GtkWidget *entry, GdkEventKey *event, gpointer user_data)
{
  Tool* tool = get_active_tool();
  if(tool != NULL) {
    tool->name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
  }
}

GtkWidget* plugin_configure(GtkDialog *dialog)
{
	GtkCellRenderer *render;
	GtkWidget* vbox = gtk_vbox_new(FALSE, 6);
	GtkWidget* hbox = gtk_hbox_new(FALSE, 6);
	GtkWidget* settingsBox = gtk_vbox_new(FALSE, 6);
	GtkWidget* buttonBox = gtk_hbox_new(FALSE, 6);

	list = gtk_tree_store_new(1, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
	render = gtk_cell_renderer_text_new();

	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Tool Name", render, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	gtk_tree_view_column_add_attribute(column, render, "text", 0);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);

  gtk_tree_view_column_set_cell_data_func(column, render,
    (GtkTreeCellDataFunc)cell_data, NULL, NULL);

	scrollable = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable),
	  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrollable), tree);
	
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
  gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
  g_signal_connect(G_OBJECT(select), "changed",
    G_CALLBACK(selected_changed), NULL);

	GtkWidget* delete = gtk_button_new_with_label("Delete");
	GtkWidget* new = gtk_button_new_with_label("New");
	g_signal_connect(delete, "clicked", G_CALLBACK(delete_tool), NULL);
	g_signal_connect(new, "clicked", G_CALLBACK(new_tool_entry), NULL);

	gtk_box_pack_start(GTK_BOX(buttonBox), delete, FALSE, FALSE, 10);
	gtk_box_pack_end(GTK_BOX(buttonBox), new, FALSE, FALSE, 10);

	gtk_box_pack_start(GTK_BOX(vbox), scrollable, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), buttonBox, FALSE, FALSE, 10);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 10);

  saveCheckbox = checkbox("Save", "Should the active document be saved when this tool is run?", GINT_TO_POINTER(SAVE));
  contextCheckbox = checkbox("Context Menu", "Should there be a context menu entry for this tool?", GINT_TO_POINTER(CONTEXT));
  menuCheckbox = checkbox("Menu", "Should there be a menu for this tool?", GINT_TO_POINTER(MENU));
  shortcutCheckbox = checkbox("Shortcut", "Should there be a keyboard shortcut for this tool?", GINT_TO_POINTER(SHORTCUT));
  
  gtk_box_pack_start(GTK_BOX(settingsBox), saveCheckbox, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(settingsBox), contextCheckbox, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(settingsBox), menuCheckbox, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(settingsBox), shortcutCheckbox, FALSE, FALSE, 10);

  title = gtk_entry_new();
	g_signal_connect(title, "changed", G_CALLBACK(on_change), NULL);
	gtk_box_pack_start(GTK_BOX(settingsBox), title, TRUE, TRUE, 2);

	gtk_box_pack_start(GTK_BOX(hbox), settingsBox, TRUE, TRUE, 10);
	gtk_widget_show_all(hbox);
	
	//g_signal_connect(dialog, "response", G_CALLBACK(dialog_response), NULL);

	return hbox;
}
