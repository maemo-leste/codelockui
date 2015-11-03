/*
 * This file is a part of codelockui
 *
 * Copyright © 2008-2009 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CLUI_CODE_DIALOG_H__
#define __CLUI_CODE_DIALOG_H__

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CLUI_TYPE_CODE_DIALOG \
    (clui_code_dialog_get_type())

#define CLUI_CODE_DIALOG(obj) \
    (GTK_CHECK_CAST (obj, CLUI_TYPE_CODE_DIALOG, CluiCodeDialog))

#define CLUI_CODE_DIALOG_CLASS(klass) \
    (GTK_CHECK_CLASS_CAST ((klass),\
    CLUI_TYPE_CODE_DIALOG, CluiCodeDialogClass))

#define CLUI_IS_CODE_DIALOG(obj) \
    (GTK_CHECK_TYPE (obj, CLUI_TYPE_CODE_DIALOG))

#define CLUI_IS_CODE_DIALOG_CLASS(klass) \
    (GTK_CHECK_CLASS_TYPE ((klass), CLUI_TYPE_CODE_DIALOG))

#define CLUI_CODE_DIALOG_GET_CLASS(obj) \
    ((CluiCodeDialogClass *) G_OBJECT_GET_CLASS(obj))

typedef struct _CluiCodeDialog CluiCodeDialog;

typedef struct _CluiCodeDialogClass CluiCodeDialogClass;

typedef struct _CluiCodeDialogPrivate CluiCodeDialogPrivate;

struct _CluiCodeDialog
{
    GtkDialog parent;
    CluiCodeDialogPrivate *priv;
};

struct _CluiCodeDialogClass
{
    GtkDialogClass parent_class;
    /* void (*input) (CluiCodeDialog *dialog); */
};

GType G_GNUC_CONST clui_code_dialog_get_type(void);

GtkWidget* clui_code_dialog_new(gboolean emergency_enabled);

gchar* clui_code_dialog_get_code(CluiCodeDialog *dialog);

void clui_code_dialog_clear_code(CluiCodeDialog *dialog);

void clui_code_dialog_set_title(CluiCodeDialog *dialog, const gchar *text);

void clui_code_dialog_set_input_sensitive(CluiCodeDialog *dialog,
					  gboolean sensitive);

void clui_code_dialog_set_ok_button_style(CluiCodeDialog *dialog);

void clui_code_dialog_set_emergency_mode(CluiCodeDialog *dialog, gboolean setting);

void clui_code_dialog_set_max_code_length(CluiCodeDialog *dialog, guint max_code_length);

void clui_code_dialog_set_cancel_button_with_label(CluiCodeDialog *dialog, const gchar *text);

#define CLUI_RESPONSE_EMCALL (100)

G_END_DECLS

#endif /* __CLUI_CODE_DIALOG_H__ */
