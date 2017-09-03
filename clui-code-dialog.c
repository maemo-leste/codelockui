/*
 * This file is a part of codelockui
 *
 * Copyright (C) 2013 Jonathan Wilson(jfwfreo@tpgi.com.au), all rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include <hildon/hildon-banner.h>
#include <hildon/hildon-helper.h>
#include <libintl.h>

#include "codelockui.h"
#include "clui-code-dialog.h"

struct _CluiCodeDialogPrivate
{
  GtkWidget *code_entry;
  GtkWidget *buttons[12];
  GtkWidget *ok_button;
  GtkWidget *cancel_button;
  GtkWidget *emergency_call_button;
  GtkWidget *vbox;
  gboolean emergency;
  GtkIMContext *im_context;
  gboolean ok_button_disabled;
};

GtkDialogClass *clui_code_dialog_parent_dialog;
guint input_signal;

G_DEFINE_TYPE(CluiCodeDialog,clui_code_dialog,GTK_TYPE_DIALOG);

void
clui_code_dialog_set_cancel_button_with_label(CluiCodeDialog *dialog,
					      const gchar *text)
{
  CluiCodeDialogPrivate *priv;

  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  gtk_button_set_label(GTK_BUTTON(priv->cancel_button), text);

  if (text)
  {
    if (!GTK_WIDGET_MAPPED(priv->cancel_button))
    {
      gtk_box_pack_end(GTK_BOX(priv->vbox), priv->cancel_button, 0, 1, 0);
      gtk_box_reorder_child(GTK_BOX(priv->vbox), priv->cancel_button, 1);
    }
  }
  else
  {
    if (GTK_WIDGET_MAPPED(priv->cancel_button))
      gtk_container_remove(GTK_CONTAINER(priv->vbox), priv->cancel_button);
  }
}

void
clui_code_dialog_set_max_code_length(CluiCodeDialog *dialog,
				     guint max_code_length)
{
  CluiCodeDialogPrivate *priv;

  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  if (max_code_length > 20)
    max_code_length = 20;

  gtk_entry_set_max_length(GTK_ENTRY(priv->code_entry), max_code_length);
}

void
clui_code_dialog_set_emergency_mode(CluiCodeDialog *dialog,
				    gboolean setting)
{
  CluiCodeDialogPrivate *priv;

  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  if (!priv->emergency)
    return;

  if (setting)
  {
    if (!GTK_WIDGET_MAPPED(priv->emergency_call_button))
    {
      gtk_box_pack_start(GTK_BOX(priv->vbox),priv->emergency_call_button, 0, 1,
			 0);
      gtk_entry_set_visibility(GTK_ENTRY(priv->code_entry), 1);
    }
  }
  else
  {
    if (GTK_WIDGET_MAPPED(priv->emergency_call_button))
    {
      gtk_container_remove(GTK_CONTAINER(priv->vbox),
			   priv->emergency_call_button);
      gtk_entry_set_visibility(GTK_ENTRY(priv->code_entry), 0);
    }
  }
}

void
clui_code_dialog_set_ok_button_style(CluiCodeDialog *dialog)
{
  CluiCodeDialogPrivate *priv;

  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  if (!priv->ok_button)
    return;

  if (GTK_WIDGET_SENSITIVE(priv->ok_button) &&
      GTK_WIDGET_PARENT_SENSITIVE(priv->ok_button))
  {
    g_object_set(G_OBJECT(priv->ok_button),
                 "name", "hildon-dtmf-back-button",
		 NULL);
  }
  else
    g_object_set(G_OBJECT(priv->ok_button), "name", NULL, NULL);
}

void
clui_code_dialog_set_input_sensitive(CluiCodeDialog *dialog,
				     gboolean sensitive)
{
  CluiCodeDialogPrivate *priv;
  int i;

  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  for (i = 0; i < 12; i++)
  {
    if (i == 7)
      continue;

    gtk_widget_set_sensitive(GTK_WIDGET(priv->buttons[i]), sensitive);
  }

  gtk_widget_set_sensitive(priv->code_entry, sensitive);

  if (priv->cancel_button)
    gtk_widget_set_sensitive(priv->cancel_button, sensitive);

  if (priv->emergency_call_button)
    gtk_widget_set_sensitive(priv->emergency_call_button, sensitive);
}

void
clui_code_dialog_set_title(CluiCodeDialog *dialog, const gchar *text)
{
  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  gtk_window_set_title(GTK_WINDOW(dialog), text);
}

void
clui_code_dialog_clear_code(CluiCodeDialog *dialog)
{
  CluiCodeDialogPrivate *priv;

  g_return_if_fail(CLUI_IS_CODE_DIALOG(dialog));

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  gtk_entry_set_text(GTK_ENTRY(priv->code_entry), "");
  gtk_widget_set_sensitive(priv->ok_button, 0);
  clui_code_dialog_set_ok_button_style(dialog);
  clui_code_dialog_set_emergency_mode(dialog, 0);
}

gchar *
clui_code_dialog_get_code(CluiCodeDialog *dialog)
{
  CluiCodeDialogPrivate *priv;

  g_return_val_if_fail(CLUI_IS_CODE_DIALOG(dialog), NULL);

  priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);

  return g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->code_entry)));
}

GtkWidget *
clui_code_dialog_new(gboolean emergency_enabled)
{
  return (GtkWidget *)g_object_new(CLUI_TYPE_CODE_DIALOG,
                                   "emergency", emergency_enabled,
                                   NULL);
}

static void
clui_code_dialog_init(CluiCodeDialog *dialog)
{
  dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog, CLUI_TYPE_CODE_DIALOG,
					     CluiCodeDialogPrivate);
}

static void
clui_code_dialog_unrealize(GtkWidget *widget)
{
  GtkIMContext *im_context =
      GTK_IM_CONTEXT(CLUI_CODE_DIALOG(widget)->priv->im_context);

  gtk_im_context_set_client_window(im_context, NULL);

  if (GTK_WIDGET_CLASS(clui_code_dialog_parent_dialog)->unrealize)
    GTK_WIDGET_CLASS(clui_code_dialog_parent_dialog)->unrealize(widget);
}

static void
clui_code_dialog_realize(GtkWidget *widget)
{
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(widget)->priv;

  if (GTK_WIDGET_CLASS(clui_code_dialog_parent_dialog)->realize)
    GTK_WIDGET_CLASS(clui_code_dialog_parent_dialog)->realize(widget);

  gtk_im_context_set_client_window(GTK_IM_CONTEXT(priv->im_context),
				   GTK_WIDGET(widget)->window);
  gtk_im_context_focus_in(priv->im_context);
}

static void
clui_code_dialog_finalize(GObject *object)
{
  g_object_unref(CLUI_CODE_DIALOG(object)->priv->im_context);
  G_OBJECT_CLASS(clui_code_dialog_parent_dialog)->finalize(object);
}

static void
clui_code_dialog_destroy(GtkObject *object)
{
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(object)->priv;

  if (priv->emergency_call_button)
  {
    g_object_unref(priv->emergency_call_button);
    priv->emergency_call_button = 0;
  }

  if (priv->cancel_button)
  {
    g_object_unref(priv->cancel_button);
    priv->cancel_button = 0;
  }

  GTK_OBJECT_CLASS(clui_code_dialog_parent_dialog)->destroy(object);
}

static void
clui_code_dialog_set_property(GObject *object, guint property_id,
			      const GValue *value, GParamSpec *pspec)
{
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(object)->priv;

  if (property_id == 1)
    priv->emergency = g_value_get_boolean(value);
}

static void
clui_code_dialog_get_property(GObject *object, guint property_id, GValue *value,
			      GParamSpec *pspec)
{
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(object)->priv;

  if (property_id == 1)
    g_value_set_boolean(value,priv->emergency);
}

void
clui_code_dialog_emit_input_signal(CluiCodeDialog *dialog, char *string)
{
  g_signal_emit(dialog, input_signal, 0, string);
}

void
clui_code_dialog_insert_text(GtkEditable *editable, gchar *new_text,
			     gint new_text_length, gint *position,
			     CluiCodeDialog *user_data)
{
  CluiCodeDialog *dialog = CLUI_CODE_DIALOG(user_data);
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(user_data)->priv;
  gchar *code = clui_code_dialog_get_code(dialog);
  gint entrylen = gtk_entry_get_max_length(GTK_ENTRY(priv->code_entry));
  glong strlen = g_utf8_strlen(code, -1);

  g_free(code);

  if (strlen == entrylen)
  {
    hildon_banner_show_information(GTK_WIDGET(dialog), NULL,
				   dgettext("hildon-common-strings",
					    "ckdg_ib_maximum_characters_reached"));
  }
  else if (!strlen)
    gtk_widget_set_sensitive(priv->ok_button,1);

  if (priv->ok_button_disabled && new_text_length)
    clui_code_dialog_emit_input_signal(dialog, "BSP");
  else
  {
    gchar *text = 0;

    if (new_text)
      text = &new_text[new_text_length - 1];

    clui_code_dialog_emit_input_signal(dialog, text);
  }

  clui_code_dialog_set_ok_button_style(dialog);
}

static void
clui_code_dialog_im_commit(GtkIMContext *imcontext,
                           gchar *arg1, CluiCodeDialog *dialog)
{
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(dialog)->priv;
  g_assert(priv);
  if (g_ascii_isdigit(*arg1))
  {
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), TRUE);
    g_signal_emit_by_name(GTK_ENTRY(priv->code_entry)->im_context,
                          "commit", arg1);
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), FALSE);
    gtk_editable_set_position(GTK_EDITABLE(priv->code_entry), -1);
  }
}

static void clui_code_dialog_backspace(CluiCodeDialog *dialog)
{
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(dialog)->priv;
  gchar *code;
  glong len;

  g_assert(priv);

  code = clui_code_dialog_get_code(dialog);
  len = g_utf8_strlen(code, -1);

  if (len)
  {
    code[len - 1] = 0;
    gtk_editable_delete_text(GTK_EDITABLE(priv->code_entry), len - 1, -1);

    if (!*code || priv->ok_button_disabled)
    {
      gtk_widget_set_sensitive(priv->ok_button, FALSE);
      clui_code_dialog_set_ok_button_style(dialog);
    }

    clui_code_dialog_emit_input_signal(dialog, "BSP");
    gtk_editable_set_position(GTK_EDITABLE(priv->code_entry), -1);
  }

  g_free(code);
}

static void
clui_code_dialog_button_clicked(GtkButton *button, CluiCodeDialog *data)
{
  CluiCodeDialog *dialog = CLUI_CODE_DIALOG(data);
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(dialog)->priv;
  gchar *digit = g_object_get_data(G_OBJECT(button), "digit");
  GtkWidget *grab = gtk_grab_get_current();

  g_assert(priv);

  if (grab == GTK_WIDGET(button))
    gtk_grab_remove(grab);

  if (GTK_BUTTON(priv->ok_button) == button)
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  else if (GTK_BUTTON(priv->cancel_button) == button)
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
  else if (priv->emergency_call_button &&
           GTK_BUTTON(priv->emergency_call_button) == button)
  {
    gtk_dialog_response(GTK_DIALOG(dialog), 100);
  }
  else if (digit && *digit)
  {
    priv->ok_button_disabled = FALSE;
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), TRUE);
    g_signal_emit_by_name(GTK_ENTRY(priv->code_entry)->im_context,
                          "commit", digit);
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), FALSE);
    gtk_editable_set_position(GTK_EDITABLE(priv->code_entry), -1);
  }
  else
    clui_code_dialog_backspace(dialog);
}

static GtkWidget *
clui_code_dialog_create_number_button(const gchar *number, const gchar *letters,
				      CluiCodeDialog *data)
{
  GtkWidget *vbox;
  GtkWidget *align;
  GtkWidget *label_number;
  GtkWidget *label_letters;
  GtkWidget *button;
  PangoFontDescription *font;

  button = g_object_new(GTK_TYPE_BUTTON,
			"focus-on-click", FALSE,
			"name", "hildon-dtmf-landscape-dialpad-button",
			NULL);

  gtk_button_set_focus_on_click(GTK_BUTTON(button), FALSE);
  GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);

  if (number)
    g_object_set_data_full(G_OBJECT(button), "digit", g_strdup(number),
			   &g_free);

  vbox = gtk_vbox_new(0, 0);
  align = g_object_new(GTK_TYPE_ALIGNMENT, NULL);

  gtk_alignment_set_padding(GTK_ALIGNMENT(align), 4u, 4u, 8u, 8u);

  label_number = g_object_new(GTK_TYPE_LABEL,
			      "xalign", 0.5,
			      "yalign", 0.0,
			      "label", number,
			      "height-request", 40,
			      NULL);

  font = pango_font_description_from_string("Nokia Sans Bold 35px");
  gtk_widget_modify_font(label_number, font);
  pango_font_description_free(font);

  label_letters = g_object_new(GTK_TYPE_LABEL,
			       "xalign", 0.5,
			       "yalign", 1.0,
			       "label", letters,
			       "height-request", 30,
			       NULL);

  hildon_helper_set_logical_font(label_letters, "SystemFont");
  hildon_helper_set_logical_color(label_letters, GTK_RC_FG, 0,
				  "SecondaryTextColor");
  hildon_helper_set_logical_color(label_letters, GTK_RC_FG, GTK_STATE_PRELIGHT,
				  "SecondaryTextColor");
  g_signal_connect_data(G_OBJECT(button), "clicked",
			G_CALLBACK(clui_code_dialog_button_clicked),
			data, 0, 0);
  gtk_box_pack_start_defaults(GTK_BOX(vbox), label_number);
  gtk_box_pack_start_defaults(GTK_BOX(vbox), label_letters);
  gtk_container_add(GTK_CONTAINER(align), vbox);
  gtk_container_add(GTK_CONTAINER(button), align);
  gtk_widget_set_size_request(button, 130, 90);
  gtk_widget_show_all(button);

  return (GtkWidget *)button;
}

struct button_strings
{
  const gchar *number;
  const gchar *letters;
};

static struct button_strings button_string_array[10] =
{
  { "0", "" },
  { "1", "" },
  { "2", "abc" },
  { "3", "def" },
  { "4", "ghi" },
  { "5", "jkl" },
  { "6", "mno" },
  { "7", "pqrs" },
  { "8", "tuv" },
  { "9", "wxyz" }
};

static GObject *
clui_code_dialog_build(GType type, guint n_construct_properties,
                       GObjectConstructParam *construct_properties)
{
  GObjectClass *object_class;
  CluiCodeDialogPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *align;
  PangoFontDescription *font;
  guint right;
  guint bottom;
  GtkWidget **buttons;
  struct button_strings *v47;
  guint col;
  GtkWidget *child;
  GtkIconTheme *icon_theme;
  GtkIconInfo *bs_icon_info;
  GdkPixbuf *bs_icon;
  GtkWidget *bs_image;
  GtkWidget *v69;
  GtkWidget *em_call_label;
  GtkWidget *call_icon;
  GtkWidget *hbox;
  GtkWidget *align_1;
  CluiCodeDialog *dialog;
  GObject *object;
  GtkWidget *table;
  guint row;
  int i;

  object_class = G_OBJECT_CLASS(clui_code_dialog_parent_dialog);
  object = object_class->constructor(type, n_construct_properties, construct_properties);

  dialog = CLUI_CODE_DIALOG(object);
  priv = CLUI_CODE_DIALOG(dialog)->priv;

  g_assert(priv);

  priv->ok_button_disabled = FALSE;

  gtk_window_set_title(GTK_WINDOW(dialog),
                       dgettext("osso-system-lock", "secu_application_title"));
  gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_dialog_set_padding(GTK_DIALOG(dialog), 0, 0, 0, 0);
  gtk_dialog_set_inner_spacing(GTK_DIALOG(dialog), 0);
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog), 0);
  gtk_rc_parse_string("style \"fremantle-widget\" {\n  GtkWidget::hildon-mode = 1\n} widget \"*.fremantle-widget\" style \"fremantle-widget\"class \"GtkEntry\" style \"fremantle-widget\"widget_class \"*.GtkEntry.*\" style \"fremantle-widget\"");
  gtk_rc_parse_string("style \"clui-code-dialog\" {\n CluiCodeDialog::action-area-border = 0\n CluiCodeDialog::contents-area-border = 0\n} widget \"*.clui-code-dialog\" style \"clui-code-dialog\"class \"CluiCodeDialog\" style \"clui-code-dialog\"widget_class \"*.CluiCodeDialog.*\" style \"clui-code-dialog\"");

  table = gtk_table_new(3, 4, 0);
  row = 0;
  i = 0;

  vbox = gtk_vbox_new(0, 0);
  align = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
  gtk_alignment_set_padding(GTK_ALIGNMENT(align), 4u, 0xCu, 0xCu, 0);

  priv->code_entry = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(priv->code_entry), 10);

  gtk_rc_parse_string("style \"pincode-entry\" = \"osso-color-themeing\" {\t\n        xthickness = 16\t\n        ythickness = 0\t\n        GtkWidget::interior-focus = 1\t\n        GtkEntry::inner-border = { 0, 0, 16, 16 }\t\n        GtkEntry::state-hint = 1\t\n        # We need to change the logical colour, because otherwise HildonEntry\t\n        # will override the specified colour.\t\n        color[\"ReversedTextColor\"] = @DefaultTextColor\t\n        engine \"sapwood\" {\t\n                image {\t\n                        function = SHADOW\t\n                        file = \"../images/CallEntry.png\"\t\n                        border = { 16, 16, 16, 16 }\t\n                }\t\n                image {\t\n                        function = FLAT_BOX\t\n                        shadow = NONE\t\n                        file = \"../images/CallEntryCenter.png\"\t\n                        border = { 12, 12, 16, 16 }\t\n                }\t\n        }\t\n}\t\nwidget \"*.pincode-entry\" style \"pincode-entry\"\t\n");

  g_object_set(G_OBJECT(priv->code_entry), "can-focus", FALSE,
               "name", "pincode-entry", NULL);
  GTK_WIDGET_UNSET_FLAGS(priv->code_entry, GTK_CAN_FOCUS);

  gtk_entry_set_invisible_char(GTK_ENTRY(priv->code_entry),
                                         g_utf8_get_char("*"));
  gtk_entry_set_alignment(GTK_ENTRY(priv->code_entry), 0.0);
  gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), 0);
  gtk_entry_set_visibility(GTK_ENTRY(priv->code_entry), 0);
  gtk_widget_set_size_request(priv->code_entry, 520, 81);

  font = pango_font_description_from_string("Nokia Sans Bold 48px");
  gtk_widget_modify_font(priv->code_entry, font);
  pango_font_description_free(font);

  gtk_box_pack_start(GTK_BOX(vbox), priv->code_entry, 1, 1, 0);
  gtk_box_pack_start(GTK_BOX(vbox), table, 1, 1, 0);
  gtk_container_add(GTK_CONTAINER(align), vbox);

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), align, 1, 1, 0);

  do
  {
    right = 0;
    bottom = row + 1;
    buttons = &priv->buttons[4 * row];
    do
    {
      v47 = &button_string_array[i + right + 1];
      col = right++;
      *buttons = clui_code_dialog_create_number_button(v47->number, v47->letters, dialog);
      child = *buttons;
      ++buttons;
      gtk_table_attach(GTK_TABLE(table), child, col, right, row, bottom,
                       GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
    }
    while ( right != 3 );

    ++row;
    i += 3;
  }
  while ( bottom != 3 );

  priv->buttons[3] = g_object_new(GTK_TYPE_BUTTON, "can-focus", FALSE, NULL);

  gtk_button_set_focus_on_click(GTK_BUTTON(priv->buttons[3]), FALSE);
  GTK_WIDGET_UNSET_FLAGS(priv->buttons[3], GTK_CAN_FOCUS);

  gtk_widget_set_size_request(priv->buttons[3], 130, 90);

  gtk_table_attach(GTK_TABLE(table), priv->buttons[3], 3, 4, 0, 1,
                   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

  /* BS button */
  icon_theme = gtk_icon_theme_get_default();
  bs_icon_info = gtk_icon_theme_lookup_icon(icon_theme, "general_backspace", 1,
                                            GTK_ICON_LOOKUP_NO_SVG);
  bs_icon = gtk_icon_theme_load_icon(
        icon_theme, "general_backspace",
        gtk_icon_info_get_base_size(bs_icon_info), GTK_ICON_LOOKUP_NO_SVG, 0);
  gtk_icon_info_free(bs_icon_info);

  bs_image = gtk_image_new_from_pixbuf(bs_icon);
  g_object_unref(G_OBJECT(bs_icon));
  gtk_container_add(GTK_CONTAINER(priv->buttons[3]), bs_image);
  g_signal_connect(G_OBJECT(priv->buttons[3]), "clicked",
                   G_CALLBACK(clui_code_dialog_button_clicked), dialog);

  priv->buttons[7] = NULL;

  /* '0' button */
  priv->buttons[11] = clui_code_dialog_create_number_button("0", "", dialog);
  gtk_widget_set_size_request(priv->buttons[11], 130, 90);
  gtk_table_attach(GTK_TABLE(table), priv->buttons[11], 3, 4, 2, 3,
                   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(GTK_DIALOG(dialog)->action_area),
                            GTK_BUTTONBOX_END);
  /* WTF? */
  gtk_hbox_new(0, 0);

  v69 = gtk_alignment_new(1.0, 1.0, 0.0, 0.0);
  gtk_alignment_set_padding(GTK_ALIGNMENT(v69), 0, 12, 0, 12);

  priv->vbox = gtk_vbox_new(0, 0);

  gtk_box_set_spacing(GTK_BOX(priv->vbox), 0);

  priv->cancel_button = g_object_new(GTK_TYPE_BUTTON, "focus-on-click", FALSE,
                                     "name", "hildon-dtmf-back-button",
                                     "label", "", NULL);
  gtk_widget_set_size_request(priv->cancel_button, 240, 90);
  gtk_button_set_focus_on_click(GTK_BUTTON(priv->cancel_button), 0);
  GTK_WIDGET_UNSET_FLAGS(priv->cancel_button, GTK_CAN_FOCUS);
  g_object_ref(priv->cancel_button);

  priv->ok_button =
      g_object_new(GTK_TYPE_BUTTON,
                   "focus-on-click", FALSE,
                   "label", dgettext("hildon-libs", "wdgt_bd_done"),
                   NULL);

  gtk_widget_set_size_request(priv->ok_button, 240, 90);
  gtk_button_set_focus_on_click(GTK_BUTTON(priv->ok_button), FALSE);
  GTK_WIDGET_UNSET_FLAGS(priv->ok_button, GTK_CAN_FOCUS);
  gtk_widget_set_sensitive(priv->ok_button, 0);

  gtk_box_pack_end(GTK_BOX(priv->vbox), priv->ok_button, 0, 1, 0);

  if (priv->emergency)
  {

    em_call_label =
        g_object_new(GTK_TYPE_LABEL,
                     "label", dgettext("osso-system-lock",
                                       "secu_lock_code_dialog_emergency_call"),
                     "xalign", 0.0f,
                     "wrap", TRUE,
                     "wrap-mode", 0,
                     "width-request", 130,
                     NULL);

    call_icon = g_object_new(GTK_TYPE_IMAGE,
                             "icon-name", "general_call",
                             "pixel-size", 64,
                             "xalign", 0.5f,
                             NULL);

    priv->emergency_call_button =
        g_object_new(GTK_TYPE_BUTTON,
                     "can-focus", FALSE,
                     "focus-on-click", FALSE,
                     "width-request", 240,
                     "height-request", 90,
                     "name", "hildon-dtmf-back-button",
                     NULL);
    hbox = gtk_hbox_new(0, 16);
    align_1 = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);

    gtk_box_pack_start(GTK_BOX(hbox), call_icon, 1, 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox), em_call_label, 1, 1, 0);
    gtk_container_add(GTK_CONTAINER(align_1), hbox);
    gtk_container_add(GTK_CONTAINER(priv->emergency_call_button), align_1);
    g_object_ref(priv->emergency_call_button);
    gtk_box_pack_end(GTK_BOX(priv->vbox), priv->emergency_call_button, 0, 1, 0);
  }

  gtk_container_add(GTK_CONTAINER(v69), priv->vbox);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), v69, 0, 1, 0);

  priv->vbox = priv->vbox;
  priv->im_context = gtk_im_multicontext_new();

  g_object_set(G_OBJECT(priv->im_context), "hildon-input-mode", 2, NULL);
  g_signal_connect(G_OBJECT(priv->im_context), "commit",
                   G_CALLBACK(clui_code_dialog_im_commit), dialog);
  g_signal_connect(G_OBJECT(priv->code_entry), "insert-text",
                   G_CALLBACK(clui_code_dialog_insert_text), dialog);
  g_signal_connect(G_OBJECT(priv->ok_button), "clicked",
                   G_CALLBACK(clui_code_dialog_button_clicked), dialog);

  if (GTK_IS_BUTTON(priv->cancel_button))
    g_signal_connect(GTK_BUTTON(priv->cancel_button), "clicked",
                     G_CALLBACK(clui_code_dialog_button_clicked), dialog);


  if (GTK_IS_BUTTON(priv->emergency_call_button))
    g_signal_connect(GTK_BUTTON(priv->emergency_call_button), "clicked",
                     G_CALLBACK(clui_code_dialog_button_clicked), dialog);

  gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->vbox));
  gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->action_area));


  if (priv->emergency_call_button)
  {
    g_object_ref(priv->emergency_call_button);
    gtk_container_remove(GTK_CONTAINER(priv->vbox),
                         priv->emergency_call_button);
  }

  return object;
}

void
clui_code_dialog_class_init(CluiCodeDialogClass *clui)
{
  GObjectClass *object = G_OBJECT_CLASS(clui);
  GtkWidgetClass *widget = GTK_WIDGET_CLASS(clui);
  clui_code_dialog_parent_dialog =
      GTK_DIALOG_CLASS(g_type_class_peek_parent(clui));

  g_type_class_add_private(clui, sizeof(CluiCodeDialogPrivate));
  object->constructor = clui_code_dialog_build;
  object->get_property = clui_code_dialog_get_property;
  object->set_property = clui_code_dialog_set_property;
  object->finalize = clui_code_dialog_finalize;
  widget->realize = clui_code_dialog_realize;
  widget->unrealize = clui_code_dialog_unrealize;
  GTK_OBJECT_CLASS(clui)->destroy = clui_code_dialog_destroy;
  input_signal = g_signal_new("input", CLUI_TYPE_CODE_DIALOG, G_SIGNAL_RUN_LAST,
			      0, NULL, NULL,
			      (GSignalCMarshaller)&g_cclosure_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1, G_TYPE_POINTER);
  g_object_class_install_property(object, 1,
				  g_param_spec_boolean("emergency", "", "", 0,
						       G_PARAM_STATIC_BLURB|
						       G_PARAM_STATIC_NICK|
						       G_PARAM_PRIVATE|
						       G_PARAM_CONSTRUCT_ONLY|
						       G_PARAM_WRITABLE|
						       G_PARAM_READABLE));
}
