#include <geanyplugin.h>

GeanyPlugin     *geany_plugin;
GeanyData       *geany_data;
GeanyFunctions  *geany_functions;

GtkWidget *dialog, *scrollable;
GtkTreeStore *list;
GtkTreeIter row;
gint row_pos;
gchar *base_directory;
gint MAX_LIST = 50;

PLUGIN_VERSION_CHECK(211)
PLUGIN_SET_INFO("Quick Opener", "Search filenames while typing", "0.1", "Steven Blatnick <steve8track@yahoo.com>");

enum
{
  KB_QUICK_OPEN,
  COUNT_KB
};

static GtkWidget *main_menu_item = NULL;

static void list_files(gchar *base, const gchar *filter)
{
  GSList *found, *file;
  found = utils_get_file_list_full(base, TRUE, TRUE, NULL);
  for(file = found; file && row_pos < MAX_LIST; file = file->next) {
    gchar *path = file->data;
    if(g_file_test(path, G_FILE_TEST_IS_DIR)) {
      list_files(path, filter);
    }
    else {
      gchar *last;
      last = g_strrstr(path,"/");
      gchar *name, *directory;
      GString *dir = g_string_new(path);
      
      name = last + 1;
      utils_string_replace_first(dir, name, "");
      utils_string_replace_first(dir, base_directory, ""); //Hide common directory
      directory = g_string_free(dir, FALSE);

      if(g_strrstr(path, filter) > 0) {
        gtk_tree_store_append(list, &row, NULL);
        gtk_tree_store_set(list, &row, 0, directory, 1, name, -1);
        row_pos++;
      }
    }
  }
}

static void onkeypress(GtkEntry *entry)
{
  row_pos = 0;
  gtk_tree_store_clear(list);
  list_files(base_directory, gtk_entry_get_text(entry));
  GtkAdjustment *adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrollable));
  gtk_adjustment_set_value(adjustment, gtk_adjustment_get_upper(adjustment));
}

static void quick_open()
{
  GtkWidget *entry, *label, *hbox, *tree;
  GtkTreeViewColumn *path_column, *name_column;
  GtkCellRenderer *renderLeft, *renderRight;

  GeanyProject   *project   = geany->app->project;
  if(project) {
    base_directory = project->base_path;
  }
  else {
    base_directory = geany->prefs->default_open_path;
  }
  
  printf("base directory: %s\n", base_directory);

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
  g_signal_connect(entry, "changed", G_CALLBACK(onkeypress), NULL);

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

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), scrollable, TRUE, TRUE, 10);
  gtk_widget_show_all(dialog);

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

