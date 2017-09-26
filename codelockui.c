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
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <string.h>
#include <libintl.h>

#include <hildon/hildon-banner.h>

#include "codelockui.h"
#include "clui-code-dialog.h"

guint eph_len = 10;
osso_context_t *osso_context;
guint timeout_event_id;
GSList *eph_list;
gchar eph_num[11];

gboolean codelockui_init(osso_context_t *osso)
{
	if (osso)
	{
		osso_context = osso;
		return TRUE;
	}
	return FALSE;
}

void codelockui_deinit(void)
{
	osso_context = 0;
}

static gint codelock_compare_func(gint val1, gint val2, gint val3)
{
	gint result;
	result = val3 >= val1;
	if (val3 > val2)
	{
		result = 0;
	}
	return result;
}

static gboolean codelock_ephnumbers_timeout_cb(gpointer user_data)
{
	timeout_event_id = 0;
	return 0;
}

void eph_max_len(guint length)
{
	if (length > 10)
	{
		length = 10;
	}
	eph_len = length;
}

void codelock_set_max_code_length(CodeLockUI *ui, guint max_code_length)
{
	if (ui)
	{
		if (ui->dialog)
		{
			clui_code_dialog_set_max_code_length(CLUI_CODE_DIALOG(ui->dialog),max_code_length);
		}
	}
	eph_max_len(max_code_length);
}

void codelock_set_emergency_mode(CodeLockUI *ui, gboolean setting)
{
	if (ui)
	{
		if (ui->dialog)
		{
			clui_code_dialog_set_emergency_mode(CLUI_CODE_DIALOG(ui->dialog),setting);
		}
	}
}

void eph_reset()
{
	eph_num[10] = 0;
	eph_num[0] = 0;
	eph_num[1] = 0;
	eph_num[2] = 0;
	eph_num[3] = 0;
	eph_num[4] = 0;
	eph_num[5] = 0;
	eph_num[6] = 0;
	eph_num[7] = 0;
	eph_num[8] = 0;
	eph_num[9] = 0;
}

void codelock_clear_code(CodeLockUI *ui)
{
	if (ui)
	{
		if (ui->dialog)
		{
			clui_code_dialog_clear_code(CLUI_CODE_DIALOG(ui->dialog));
			eph_reset();
		}
	}
}

static gboolean code_entry_timeout_cb(CodeLockUI *ui)
{
  ui->entry_event_id = 0;
  clui_code_dialog_clear_code(CLUI_CODE_DIALOG(ui->dialog));
  eph_reset();
  return FALSE;
}

const gchar* codelock_get_code(CodeLockUI *ui)
{
  if (ui && ui->dialog)
    return clui_code_dialog_get_code(CLUI_CODE_DIALOG(ui->dialog));

  return NULL;
}

void codelock_disable_input(CodeLockUI *ui, gboolean disable)
{
  if (ui && ui->dialog)
    clui_code_dialog_set_input_sensitive(CLUI_CODE_DIALOG(ui->dialog),!disable);
}

static gboolean
codelock_verify_passwd(const gchar *parameter,
                       const gchar *old_code, const gchar *new_code)
{
  __pid_t pid;
  int status = 0;

  pid = fork();

  if (pid == -1)
    return FALSE;

  if (!pid)
  {
    /* child process */
    if (parameter)
      execl("/bin/devlocktool", "/bin/devlocktool", parameter, new_code,
            old_code, NULL);
    else
      execl("/bin/devlocktool", "/bin/devlocktool", old_code, NULL);

    exit(-1);
  }

  if (waitpid(pid, &status, 0) != -1 && !WTERMSIG(status))
    return WEXITSTATUS(status) == 1;

  return FALSE;
}

gboolean codelock_is_passwd_correct(const gchar *pw)
{
	return codelock_verify_passwd(NULL, pw, NULL);
}

gboolean codelock_change_passwd(const gchar *old_passwd, const gchar *new_passwd)
{
	return codelock_verify_passwd("-C", old_passwd, new_passwd);
}

static void codelock_reset_dialog(osso_context_t *osso, CodeLockUI *ui, const gchar *title)
{
	if (ui)
	{
		if (title)
		{
			clui_code_dialog_set_title(CLUI_CODE_DIALOG(ui->dialog), title);
		}
		clui_code_dialog_clear_code(CLUI_CODE_DIALOG(ui->dialog));
		eph_reset();
	}
}

static void _codelock_reset_dialog(osso_context_t *osso, CodeLockUI *ui,
                                   const gchar *title)
{
  if (!ui)
    return;

  if (title)
    clui_code_dialog_set_title(CLUI_CODE_DIALOG(ui->dialog), title);

  clui_code_dialog_clear_code(CLUI_CODE_DIALOG(ui->dialog));
  eph_reset();
}

static void codelock_response_signal(GtkDialog *dialog, gint response_id,
                                     CodeLockUI *ui)
{
  if (!ui)
          return;

  if (ui->passwd_idx >= 4)
          return;

  ui->passwd_idx = 0;

  if (response_id == GTK_RESPONSE_CANCEL ||
      response_id == GTK_RESPONSE_DELETE_EVENT)
  {
    hildon_banner_show_information(
          GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(ui->dialog))),
          NULL,
          dgettext("osso-system-lock", "secu_ib_lockcodenotchanged"));

    ui->passwd_idx = 4;

    if (ui->changefunc)
    {
      g_signal_handler_disconnect(G_OBJECT(ui->dialog),
                                  ui->response_signal_handle);
      ui->response_signal_handle = 0;
      ui->changefunc(ui, 0);
    }
  }
  else
  {
    if (!*clui_code_dialog_get_code(CLUI_CODE_DIALOG(ui->dialog)))
      return;

    if (codelock_compare_func(0, 2, ui->passwd_idx) == 1)
    {
      if (ui->passwd[ui->passwd_idx])
      {
        g_free(ui->passwd[ui->passwd_idx]);
        ui->passwd[ui->passwd_idx] = NULL;
      }

      ui->passwd[ui->passwd_idx] =
                      g_strdup(clui_code_dialog_get_code(CLUI_CODE_DIALOG(ui->dialog)));
    }

    if (ui->passwd_idx == 1)
    {
      const char *msgid;

      if (codelock_compare_func(0, 2, 1) != 1 || strlen(ui->passwd[1]) > 4)
      {
        msgid = "secu_ti_changelock_3";
        ui->passwd_idx = 2;
      }
      else
      {
        hildon_banner_show_information(
              ui->dialog,
              NULL,
              dgettext("osso-system-lock", "secu_ib_lockcodetooshort"));

        msgid = "secu_ti_changelock_2";
      }

      _codelock_reset_dialog(osso_context, ui,
                             dgettext("osso-system-lock", msgid));
    }
    else
    {
      if (ui->passwd_idx != 2)
      {
        if (ui->passwd_idx)
          return;

        if (!ui->passwd[0] || !codelock_is_passwd_correct(ui->passwd[0]))
        {
          if (codelock_compare_func(0, 2, ui->passwd_idx) == 1 &&
              ui->passwd[ui->passwd_idx])
          {
            hildon_banner_show_information(
                  GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(ui->dialog))),
                  NULL,
                  dgettext("osso-system-lock", "secu_info_incorrectcode"));

            g_free(ui->passwd[0]);
            g_free(ui->passwd[1]);
            g_free(ui->passwd[2]);

            ui->passwd[0] = 0;
            ui->passwd[1] = 0;
            ui->passwd[2] = 0;
            ui->passwd_idx = 3;

            if (ui->changefunc)
            {
              g_signal_handler_disconnect(G_OBJECT(ui->dialog),
                                          ui->response_signal_handle);
              ui->response_signal_handle = 0;
              ui->changefunc(ui, 0);
            }
          }

          return;
        }

        hildon_banner_show_information(
              ui->dialog,
              NULL,
              dgettext("osso-system-lock", "secu_info_codeaccepted"));
        ++ui->passwd_idx;

reset:
        _codelock_reset_dialog(osso_context, ui,
                               dgettext("osso-system-lock",
                                        "secu_ti_changelock_2"));

        return;
      }

      if (codelock_compare_func(0, 2, 2) == 1)
      {
        if ( strcmp(ui->passwd[2], ui->passwd[1]) )
        {
          hildon_banner_show_information(ui->dialog, NULL,
                                         dgettext("osso-system-lock",
                                                  "secu_ib_codedonotmatch"));
          --ui->passwd_idx;
          goto reset;
        }
      }

      if ( !codelock_change_passwd(ui->passwd[0], ui->passwd[2]))
      {
        hildon_banner_show_information(ui->dialog, NULL,
                                       dgettext("osso-system-lock",
                                                "secu_ib_lockcodenotchanged"));

        ui->passwd_idx = 1;
        g_free(ui->passwd[1]);
        g_free(ui->passwd[2]);

        ui->passwd[2] = 0;
        ui->passwd[1] = 0;

        goto reset;
      }

      hildon_banner_show_information(
            GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(ui->dialog))),
            NULL,
            dgettext("osso-system-lock", "secu_ib_lockcodechanged"));

      ui->passwd_idx = 4;

      if (ui->changefunc)
      {
        g_signal_handler_disconnect(G_OBJECT(ui->dialog), ui->response_signal_handle);
        ui->response_signal_handle = 0;
        ui->changefunc(ui, 1);
      }
    }
  }
}

gboolean codelock_password_change(CodeLockUI *ui, CodeLockChangeFunc func)
{
  if (ui)
  {
    ui->response_signal_handle =
        g_signal_connect(G_OBJECT(ui->dialog), "response",
                         G_CALLBACK(codelock_response_signal), ui);
    codelock_reset_dialog(osso_context, ui, dgettext("osso-system-lock",
                                                     "secu_ti_changelock_1"));
    ui->passwd_idx = 0;
    ui->changefunc = func;

    if (func)
    {
      gint idx = 0;
      gint response = -1;

      while (idx <= 3 || response == -4)
      {
        response = gtk_dialog_run(GTK_DIALOG(ui->dialog));

        if (response != -6 && response != -4)
        {
          idx = ui->passwd_idx;

          if (idx != 3)
            continue;
        }

        g_signal_handler_disconnect(G_OBJECT(ui->dialog),
                                    ui->response_signal_handle);
        ui->response_signal_handle = 0;
        return FALSE;
      }

      g_signal_handler_disconnect(G_OBJECT(ui->dialog),
                                  ui->response_signal_handle);
      ui->response_signal_handle = 0;
      return TRUE;
    }
  }
  return FALSE;
}

static gboolean key_press_event_cb(GtkDialog *dialog, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval == 0xFF1B)
	{
		gtk_dialog_response(dialog, -6);
		return TRUE;
	}
	return FALSE;
}

static void codelock_ephnumbers_free(GPtrArray *array)
{
	if (array)
	{
		for (int i = 0;i < array->len;i++)
		{
			g_free(array->pdata[i]);
			array->pdata[i] = 0;
		}
		g_ptr_array_set_size(array, 0);
	}
}

void eph_destroy()
{
	if (eph_list)
	{
		for (int i = 0;i < g_slist_length(eph_list);i++)
		{
			g_free(g_slist_nth_data(eph_list,i));
		}
		g_slist_free(eph_list);
		eph_list = 0;
	}
}

void codelock_destroy_dialog(CodeLockUI *ui)
{
	if (!ui)
	{
		return;
	}
	if (ui->entry_event_id)
	{
		g_source_remove(ui->entry_event_id);
		ui->entry_event_id = 0;
	}
	if (timeout_event_id)
	{
		g_source_remove(timeout_event_id);
		timeout_event_id = 0;	
	}
	if (ui->dialog)
	{
		gtk_dialog_response(GTK_DIALOG(ui->dialog), -1);
		gtk_widget_destroy(ui->dialog);
		ui->dialog = 0;
	}
	for (int i = 0;i < 3;i++)
	{
		if (ui->passwd[i])
		{
			g_free(ui->passwd[i]);
			ui->passwd[i] = 0;
		}
	}
	if (ui->ephnumbers)
	{
		codelock_ephnumbers_free(ui->ephnumbers);
		g_ptr_array_free(ui->ephnumbers, 1);
		ui->ephnumbers = 0;
		eph_destroy();
	}
	ui->dialog = 0;
	ui->entry_event_id = 0;
	ui->code_entry_timeout = 0;
	ui->passwd[0] = 0;
	ui->passwd[1] = 0;
	ui->passwd[2] = 0;
	ui->passwd_idx = 0;
	ui->changefunc = 0;
	ui->response_signal_handle = 0;
	ui->ephnumbers = 0;
	ui->input_state = 0;
}

gboolean utils_dbus_send(const char *bus_name, const char *path, const char *interface, const char *method, gboolean (*callback)(DBusMessage *, CodeLockUI *), CodeLockUI *ui, DBusPendingCallNotifyFunction pending_call_notify, int first_arg_type, ...)
{
	va_list va;
	DBusError error;
	gboolean res;
	DBusPendingCall *pending_return;
	va_start(va,first_arg_type);
	if (!bus_name)
	{
		return FALSE;
	}
	dbus_error_init(&error);
	DBusConnection *connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
	if (dbus_error_is_set(&error))
	{
		dbus_error_free(&error);
		return FALSE;
	}
	DBusMessage *message = dbus_message_new_method_call(bus_name,path,interface,method);
	if (message)
	{
		if (first_arg_type && !dbus_message_append_args_valist(message,first_arg_type,va))
		{
			res = FALSE;
			goto label1;
		}
		if (callback)
		{
			dbus_error_init(&error);
			DBusMessage *message2 = dbus_connection_send_with_reply_and_block(connection,message,-1,&error);
			if (dbus_error_is_set(&error))
			{
				dbus_error_free(&error);
				res = FALSE;
				goto label1;
			}
			if (message2)
			{
				res = callback(message2,ui);
				dbus_message_unref(message2);
label1:
				dbus_message_unref(message);
				goto label2;
			}
label3:
			res = 0;
			goto label1;
		}
		if (pending_call_notify)
		{
			if (!dbus_connection_send_with_reply(connection,message,&pending_return,-1))
			{
				goto label3;
			}
			if (!pending_return)
			{
				goto label3;
			}
			dbus_connection_flush(connection);
			if (!dbus_pending_call_set_notify(pending_return,pending_call_notify,NULL,NULL))
			{
				goto label3;
			}
		}
		else
		{
			if (!dbus_connection_send(connection,message,NULL))
			{
				res = FALSE;
				goto label1;
			}
			dbus_connection_flush(connection);
		}
		res = TRUE;
		goto label1;
	}
	res = FALSE;
label2:
	if (dbus_error_is_set(&error))
	{
		dbus_error_free(&error);
	}
	dbus_connection_close(connection);
	dbus_connection_unref(connection);
	return res;
}

static void response_cb(GtkDialog *dialog, gint response_id, CodeLockUI *ui)
{
	if (ui)
	{
		if (ui->entry_event_id)
		{
			g_source_remove(ui->entry_event_id);
			ui->entry_event_id = 0;
		}
		if (response_id != 100)
		{
			return;
		}
		char *string = "urn:service:sos";
		utils_dbus_send("com.nokia.csd.Call","/com/nokia/csd/call","com.nokia.csd.Call","Create",NULL,NULL,NULL,'s',&string,0);
	}
}

static gboolean _ephnumbers_dbus_cb(DBusMessage *message, CodeLockUI *ui)
{
	DBusMessageIter iter;
	DBusMessageIter sub;
	const gchar *value;
	if (dbus_message_get_type(message) != 2 || (dbus_message_iter_init(message, &iter), dbus_message_iter_get_arg_type(&iter) != 'a') || dbus_message_iter_get_element_type(&iter) != 's')
	{
		return FALSE;
	}
	else
	{
		dbus_message_iter_recurse(&iter,&sub);
		codelock_ephnumbers_free(ui->ephnumbers);
		do
		{
			dbus_message_iter_get_basic(&sub,&value);
			g_ptr_array_add(ui->ephnumbers,g_strdup(value));
		} while (dbus_message_iter_next(&sub));
		return TRUE;
	}
}

void _eph_add(const gchar *num,gpointer user_data)
{
	g_return_if_fail((num != NULL) || (strlen(num) < 1));
	eph_list = g_slist_append(eph_list,g_strdup(num));
}

void eph_init(GPtrArray *array)
{
	eph_destroy();
	if (array)
	{
		g_ptr_array_foreach(array,(GFunc)_eph_add,NULL);
	}
}

static gint _ephnumbers_sort(const gchar **num1, const gchar **num2)
{
	return g_ascii_strcasecmp(*num1, *num2);
}

static gboolean _get_ephnumbers(CodeLockUI *ui)
{
	g_assert(ui);
	if (ui->response_signal_handle)
	{
		return FALSE;
	}
	if (timeout_event_id)
	{
		return TRUE;
	}
	if (!ui->ephnumbers)
	{
		return FALSE;
	}
	while (gtk_events_pending())
	{
		gtk_main_iteration();
	}
	if (!utils_dbus_send("com.nokia.csd.Call","/com/nokia/csd/call","com.nokia.csd.Call","GetEmergencyNumbers",_ephnumbers_dbus_cb,ui,NULL,0))
	{
		if (!ui->ephnumbers->len)
		{
			g_ptr_array_add(ui->ephnumbers,g_strdup("112"));
			g_ptr_array_add(ui->ephnumbers,g_strdup("911"));
		}
	}
	g_ptr_array_sort(ui->ephnumbers,(GCompareFunc)_ephnumbers_sort);
	eph_init(ui->ephnumbers);
	timeout_event_id = g_timeout_add(120000,codelock_ephnumbers_timeout_cb,NULL);
	return TRUE;
}

void input_cb(CodeLockUI *ui, char *pointer, gpointer user_data)
{
	if (ui->entry_event_id)
	{
		g_source_remove(ui->entry_event_id);
		ui->entry_event_id = 0;
	}
	ui->entry_event_id = g_timeout_add(ui->code_entry_timeout,(GSourceFunc)code_entry_timeout_cb,ui);
	_get_ephnumbers(ui);
}

gint _eph_input_state(const gchar *num_str)
{
	g_return_val_if_fail(num_str != NULL,0);
	g_return_val_if_fail(strlen(num_str) > 0,0);
	int count = 0;
	const gchar *str;
	do
	{
		guint len = g_slist_length(eph_list);
		int i1 = len >= count;
		int i2 = len == count++;
		if (i2 || !i1)
		{
			return 1;
		}
		str = (const gchar *)g_slist_nth_data(eph_list, count);
		if (g_ascii_strcasecmp(str,num_str))
		{
			return 3;
		}
	} while (g_str_has_prefix(str, num_str) != 1);
	return 2;
}

void eph_input_cb(CodeLockUI *ui, char *pointer, gpointer user_data)
{
	size_t len = strlen(eph_num);
	if (pointer && *pointer != 0)
	{
		if (*pointer == 'B')
		{
			if (len <= 0)
			{
				eph_reset();
			}
			else
			{
				eph_num[len - 1] = 0;
			}
		}
		else if (len < eph_len)
		{
			eph_num[len] = *pointer;
			eph_num[len + 1] = 0;
		}
		ui->input_state = _eph_input_state(eph_num);
		if (ui->input_state != 3)
		{
			goto label1;
		}
	}
	else
	{
		ui->input_state = 0;
		eph_reset();
		if (ui->input_state != 3)
		{
label1:
			clui_code_dialog_set_emergency_mode(CLUI_CODE_DIALOG(ui->dialog),FALSE);
			return;
		}
	}
	clui_code_dialog_set_emergency_mode(CLUI_CODE_DIALOG(ui->dialog),TRUE);
}

GtkWidget* codelock_create_dialog_help(CodeLockUI *ui,osso_context_t *osso,gint timeout,gboolean emergency_enabled)
{
	g_return_val_if_fail(ui,0);
	ui->dialog = 0;
	ui->entry_event_id = 0;
	ui->code_entry_timeout = 0;
	ui->passwd[0] = 0;
	ui->passwd[1] = 0;
	ui->passwd[2] = 0;
	ui->passwd_idx = 0;
	ui->changefunc = 0;
	ui->response_signal_handle = 0;
	ui->ephnumbers = 0;
	ui->input_state = 0;
	if (timeout != -1)
	{
		ui->code_entry_timeout = timeout;
	}
	else
	{
		ui->code_entry_timeout = 5000;
	}
	if (emergency_enabled)
	{
		ui->ephnumbers = g_ptr_array_new();
		_get_ephnumbers(ui);
	}
	ui->dialog = clui_code_dialog_new(emergency_enabled);
	gtk_window_set_type_hint(GTK_WINDOW(ui->dialog),GDK_WINDOW_TYPE_HINT_DIALOG);
	codelock_reset_dialog(NULL,ui,NULL);
	g_signal_connect_data(G_OBJECT(ui->dialog),"key-press-event",G_CALLBACK(key_press_event_cb),ui,NULL,0);
	g_signal_connect_data(G_OBJECT(ui->dialog),"input",G_CALLBACK(input_cb),ui,NULL,G_CONNECT_SWAPPED);
	if (emergency_enabled)
	{
		if (ui->ephnumbers->len)
		{
			ui->input_state = 0;
			eph_reset();
			g_signal_connect_data(G_OBJECT(ui->dialog),"input",G_CALLBACK(eph_input_cb),ui,NULL,G_CONNECT_SWAPPED);
		}
	}
	g_signal_connect_data(G_OBJECT(ui->dialog),"response",G_CALLBACK(response_cb),ui,NULL,0);
	return ui->dialog;
}

GtkWidget* codelock_create_dialog(CodeLockUI *ui, gint timeout, gboolean emergency_enabled)
{
	return codelock_create_dialog_help(ui,NULL,timeout,emergency_enabled);
}
