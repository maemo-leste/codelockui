/**
 * @file codelockui.h
 * <p>
 * Copyright © 2004-2009 Nokia Corporation.
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

#ifndef _CODELOCKUI_H_
#define _CODELOCKUI_H_

#define PLUGIN_TEXTDOMAIN GETTEXT_PACKAGE

#include <gtk/gtk.h>

#include <libosso.h>

/*
#define ENTRY_TIMEOUT		"/system/systemui/devlock/text_entry_timeout"
#define PASSWD_SET		"/system/systemui/devlock/passwd_set"
*/
#define DEFAULT_CODE_ENTRY_TIMEOUT	(5*1000)
#define TIMEOUT_FOOBAR			-1

#define MAX_PINCODE_LEN                 10
#define MIN_PINCODE_LEN                 5

/**
	Callback, which is called, when codelock changing process has been
	finished.

	@param ui	The CodeLockUI notifying us. CodeLockUI instance can
			be destroyed in this callback.
	@param changed  TRUE if password was successfully changed
*/
typedef void (CodeLockChangeFunc) (gpointer ui, gboolean changed);

typedef struct {
	GtkWidget *dialog;

	/* Internal fields, don't mess with these! */
	guint entry_event_id;
	guint code_entry_timeout;

	gchar* passwd[3];
	gint passwd_idx;

	CodeLockChangeFunc *changefunc;
	gulong response_signal_handle;

	/* XXX: to provide emergency capability */
	GPtrArray *ephnumbers;
	gint input_state;

} CodeLockUI;

/**
	Initializes osso context to be used with the library functions.

        @param osso Osso context parameter
        @returns TRUE if osso was not NULL, otherwise FALSE
*/
gboolean codelockui_init(osso_context_t *osso);

/**
        Nulls osso context.
*/
void codelockui_deinit(void);

/**
	Function for API stability (calls codelock_create_dialog_help with osso set to NULL).

        @param ui	Pointer to allocated DeviceLockUi-struct, which is
			initialized with codelock_create_dialog.
	@param osso     pointer to osso structure.
	@param func	Callback, which is called when the changing is
			finished. If the func is NULL, then the function
			blocks until code has been changed (or cancelled).

	@return TRUE, if code was changed (always FALSE, if
                CodeLockChangeFunc is provided).
*/
GtkWidget* codelock_create_dialog_help(CodeLockUI *ui,
				       osso_context_t *osso,
				       gint timeout,
				       gboolean emergency_enabled);

/**
	Creates code query dialog.

	@param ui	Pointer to allocated DeviceLockUi-struct, which will be
			filled in by this function.
	@param timeout	The wanted timeout for the dialog or
			TIMEOUT_FOOBAR, if default is wanted.

	@return returns created dialog.
*/
GtkWidget* codelock_create_dialog(CodeLockUI *ui, gint timeout, gboolean emergency_enabled);

/**
	Initiates password changing for a given CodeLockUI.

	@param ui	Pointer to allocated DeviceLockUi-struct, which is
			initialized with codelock_create_dialog.
	@param func	Callback, which is called when the changing is
			finished. If the func is NULL, then the function
			blocks until code has been changed (or cancelled).

	@return TRUE, if code was changed (always FALSE, if
		CodeLockChangeFunc is provided).
*/
gboolean codelock_password_change(CodeLockUI *ui, CodeLockChangeFunc func);

/**
	Destroy code query dialog.

	@param ui	Pointer to allocated DeviceLockUi-struct. The struct itself
			will not be freed.
*/
void codelock_destroy_dialog(CodeLockUI *ui);

/**
        Disables the inputs of codelockui.
	@param ui	Pointer to allocated DeviceLockUI-struct.
	@param disable  TRUE if the inputs should be disabled.
*/
void codelock_disable_input(CodeLockUI *ui, gboolean disable);

/**
	Gets the code from the dialog.
	@param ui	Pointer to allocated DeviceLockUI-struct.
*/
const gchar* codelock_get_code(CodeLockUI *ui);

/**
	Clears the code from the dialog.
	@param ui	Pointer to allocated DeviceLockUI-struct.
*/
void codelock_clear_code(CodeLockUI *ui);

/**
	sets the emergency mode for the dialog.
	@param ui	Pointer to allocated DeviceLockUI-struct.
	@param setting  TRUE/FALSE  enables/disables the emergency mode
*/
void codelock_set_emergency_mode(CodeLockUI *ui, gboolean setting);

/**
	Sets the maximum code length for the dialog.
	@param ui	Pointer to allocated DeviceLockUI-struct.
	@param setting  Maximum length of code
*/
void codelock_set_max_code_length(CodeLockUI *ui, guint max_code_length);

/**
   Function to check password

   @param pw password to be tested
   @return TRUE, if the given password (pin code) is correct or
	   FALSE, if the password is incorrect, or error occured.
*/
gboolean codelock_is_passwd_correct(const gchar *pw);

/**
  function to change current password.

  @param old_passwd	Current password.
  @param new_passwd	The new password.

  @return TRUE, if password was succesfully changed.
*/
gboolean codelock_change_passwd(const gchar *old_passwd,
				const gchar *new_passwd);

#endif /* CODELOCKUI_H */
