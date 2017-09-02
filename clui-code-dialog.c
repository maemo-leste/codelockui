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
    g_object_set(G_OBJECT(priv->ok_button), "name", "hildon-dtmf-back-button",
		 NULL);
  }
  else
    g_object_set(G_OBJECT(priv->ok_button), "name", NULL, NULL);
}

void
clui_code_dialog_set_input_sensitive(CluiCodeDialog *dialog,
				     gboolean sensitive)
{
  //todo
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
  return (GtkWidget *)g_object_new(CLUI_TYPE_CODE_DIALOG, "emergency",
				   emergency_enabled, NULL);
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
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), 1);
    g_signal_emit_by_name(GTK_ENTRY(priv->code_entry),"commit", arg1);
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), 0);
    gtk_editable_set_position(GTK_EDITABLE(priv->code_entry), -1);
  }
}

static void
clui_code_dialog_button_clicked(GtkButton *button, CluiCodeDialog *data)
{
  GtkWidget *grab = gtk_grab_get_current();
  CluiCodeDialog *dialog = CLUI_CODE_DIALOG(data);
  CluiCodeDialogPrivate *priv = CLUI_CODE_DIALOG(dialog)->priv;
  gchar *digit = g_object_get_data(G_OBJECT(button),"digit");

  g_assert(priv);

  if (grab == GTK_WIDGET(button))
    gtk_grab_remove(grab);

  if (GTK_BUTTON(priv->ok_button) == button)
  {
    gtk_dialog_response(GTK_DIALOG(dialog), -6);
    return;
  }

  if (priv->emergency_call_button &&
      GTK_BUTTON(priv->emergency_call_button) == button)
  {
    gtk_dialog_response(GTK_DIALOG(dialog), 100);
  }
  else if (digit && *digit)
  {
    priv->ok_button_disabled = FALSE;
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), 1);
    g_signal_emit_by_name(GTK_ENTRY(priv->code_entry), "commit", digit);
    gtk_editable_set_editable(GTK_EDITABLE(priv->code_entry), 0);
    gtk_editable_set_position(GTK_EDITABLE(priv->code_entry), -1);
  }
  else
  {
    gchar *code = clui_code_dialog_get_code(dialog);
    glong len = g_utf8_strlen(code,-1);

    if (len)
    {
      code[len - 1] = 0;
      gtk_editable_delete_text(GTK_EDITABLE(priv->code_entry), len - 1, -1);

      if (!*code || (priv->ok_button_disabled = TRUE, !*code))
      {
	gtk_widget_set_sensitive(priv->ok_button, 0);
	clui_code_dialog_set_ok_button_style(dialog);
      }

      clui_code_dialog_emit_input_signal(dialog, "BSP");
      gtk_editable_set_position(GTK_EDITABLE(priv->code_entry), -1);
    }

    g_free(code);
  }
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

static GObject *
clui_code_dialog_build(GType type, guint n_construct_properties,
		       GObjectConstructParam *construct_properties)
{
  //todo
  return NULL;
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
