#include <geanyplugin.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#define foreach_slist_free(node, list) for (node = list, list = NULL; g_slist_free_1(list), node != NULL; list = node, node = node->next)

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

extern gchar *tools;

static GtkWidget *panel; //All contents of the panel
static GtkWidget *label;
static GtkWidget *text_view;
static GtkWidget *scrollable_text, *scrollable_table;

typedef struct
{
	const gchar *text;
	GRegex *regex;
} RegexSetting;
static RegexSetting pathRegexSetting = {"^\\.|^build$", NULL};
static RegexSetting nameRegexSetting = {"^\\.|\\.(o|so|exe|class|pyc)$", NULL};
static RegexSetting fileRegexSetting = {"[\\w./-]+\\.[\\w./-]+:?(\\d+)?", NULL};

static void goto_link(gchar *url) {
	printf("url: %s\n", url);
}

static gchar* get_link_at_iter(GtkTextIter in)
{
	GtkTextIter iter = in;
	gchar *text = "";
	GSList *list, *node;
	list = gtk_text_iter_get_tags(&iter);
	if(list != NULL) {
	  foreach_slist_free(node, list) {
			GtkTextTag *tag = node->data;
			gchar *name;
			g_object_get(G_OBJECT(tag), "name", &name, NULL);
			if(strcmp(name, "error") == 0) {
				GtkTextIter end = iter;
				gtk_text_iter_backward_to_tag_toggle(&iter, tag);
				gtk_text_iter_forward_to_tag_toggle(&end, tag);
				text = gtk_text_iter_get_text(&iter, &end);
				g_free(name);
				break;
			}
			g_free(name);
		}
	}
	return text;
}

static gboolean on_click(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GtkTextIter iter;
	gint x, y;
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), GTK_TEXT_WINDOW_TEXT, event->x, event->y, &x, &y);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(text_view), &iter, x, y);
	
	gchar *text = get_link_at_iter(iter);
	goto_link(text);
	return FALSE;
}

static gboolean on_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if(event->keyval == GDK_Return) {
		printf("button!\n");
		GtkTextIter iter;
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
		GtkTextMark *cursor = gtk_text_buffer_get_mark(buffer, "insert");
		gtk_text_buffer_get_iter_at_mark(buffer, &iter, cursor);
		gchar *link = get_link_at_iter(iter);
		goto_link(link);
	}
	return FALSE;
}

static gboolean panel_focus_tab(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	GeanyKeyBinding *kb = keybindings_lookup_item(GEANY_KEY_GROUP_FOCUS, GEANY_KEYS_FOCUS_MESSAGE_WINDOW);
	if (kb != NULL && event->key.keyval == kb->key && (event->key.state & gtk_accelerator_get_default_mod_mask()) == kb->mods) {
		gint current = gtk_notebook_get_current_page(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook));
		gint tab = gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook), panel);
		if(current == tab) {
			gtk_widget_grab_focus(scrollable_text);
		}
	}
	return FALSE;
}

void panel_init()
{
	label = gtk_label_new(_("Tools"));
	panel = gtk_vbox_new(FALSE, 6);
	text_view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
	scrollable_text = gtk_scrolled_window_new(NULL, NULL);
	scrollable_table = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable_text), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollable_table), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(scrollable_text), text_view);
	gtk_box_pack_start(GTK_BOX(panel), scrollable_table, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(panel), scrollable_text, TRUE, TRUE, 2);

	gtk_notebook_append_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		panel,
		label
	);

	gtk_widget_show(label);
	gtk_widget_show(panel);
	gtk_widget_show_all(scrollable_text);
	gtk_container_set_focus_child(GTK_CONTAINER(panel), scrollable_text);

	g_signal_connect(geany->main_widgets->window, "key-release-event", G_CALLBACK(panel_focus_tab), NULL);
	g_signal_connect(text_view, "button-press-event", G_CALLBACK(on_click), NULL);
	g_signal_connect(text_view, "key-press-event", G_CALLBACK(on_keypress), NULL);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	gtk_text_buffer_create_tag(buffer, "error", "foreground", "#ff0000", NULL);
	gtk_text_buffer_create_tag(buffer, "link", "foreground", "#0000ff", "underline", PANGO_UNDERLINE_SINGLE, NULL);
	
  pathRegexSetting.regex = g_regex_new(pathRegexSetting.text, G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
	nameRegexSetting.regex = g_regex_new(nameRegexSetting.text, G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
	fileRegexSetting.regex = g_regex_new(fileRegexSetting.text, G_REGEX_OPTIMIZE | G_REGEX_CASELESS, 0, NULL);
}

void panel_cleanup()
{
	gtk_notebook_remove_page(
		GTK_NOTEBOOK(geany->main_widgets->message_window_notebook),
		gtk_notebook_page_num(GTK_NOTEBOOK(geany->main_widgets->message_window_notebook), panel)
	);

	g_regex_unref(pathRegexSetting.regex);
	g_regex_unref(nameRegexSetting.regex);
	g_regex_unref(fileRegexSetting.regex);
}

void panel_prepare()
{
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)), "", 0);
	gtk_widget_hide(scrollable_table);
}

void panel_print(gchar *text, const gchar *tag)
{
	GtkTextIter iter, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	if(tag == NULL) {
		gtk_text_buffer_insert(buffer, &iter, text, -1);
	}
	else {
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, tag, NULL);
	}
	gtk_text_iter_backward_char(&iter);
	end = iter;
	gtk_text_iter_backward_line(&iter);

  gchar *line_text = gtk_text_iter_get_text(&iter, &end);
	printf("line: \"%s\"\n", text);
	
	GMatchInfo *info;
	if(g_regex_match(fileRegexSetting.regex, line_text, 0, &info)) {
    while(g_match_info_matches(info))
    {
      gchar *word = g_match_info_fetch(info, 0);
      g_print("Found: %s\n", word);
      g_free (word);
      g_match_info_next(info, NULL);
    }
	}
	
	g_match_info_free(info);
	g_free(line_text);

  //fileRegexSetting
	//strstr - find first substr
	//strchr - find first character

	
	//gtk_text_buffer_apply_tag_by_name

	//Scroll to bottom:
	GtkTextMark *mark;
	mark = gtk_text_buffer_get_insert(buffer);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view), mark, 0.0, FALSE, 0.0, 0.0);
}
