/* afp_stat.c
 * Based on
 * smb_stat   2003 Ronnie Sahlberg
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include <epan/packet_info.h>
#include <epan/epan.h>
#include <epan/value_string.h>
#include <epan/tap.h>
#include <epan/dissectors/packet-afp.h>

#include "../file.h"
#include "../stat_menu.h"

#include "ui/simple_dialog.h"

#include "ui/gtk/gui_utils.h"
#include "ui/gtk/dlg_utils.h"
#include "ui/gtk/service_response_time_table.h"
#include "ui/gtk/tap_param_dlg.h"
#include "ui/gtk/main.h"

#include "ui/gtk/old-gtk-compat.h"

void register_tap_listener_gtkafpstat(void);

/* used to keep track of the statistics for an entire program interface */
typedef struct _afpstat_t {
	GtkWidget *win;
	srt_stat_table afp_srt_table;
} afpstat_t;

static void
afpstat_set_title(afpstat_t *ss)
{
	set_window_title(ss->win, "AFP Service Response Time statistics");
}

static void
afpstat_reset(void *pss)
{
	afpstat_t *ss=(afpstat_t *)pss;

	reset_srt_table_data(&ss->afp_srt_table);
	afpstat_set_title(ss);
}

static int
afpstat_packet(void *pss, packet_info *pinfo, epan_dissect_t *edt _U_, const void *prv)
{
	afpstat_t *ss=(afpstat_t *)pss;
	const afp_request_val *request_val=(const afp_request_val *)prv;

	/* if we havnt seen the request, just ignore it */
	if(!request_val){
		return 0;
	}

	add_srt_table_data(&ss->afp_srt_table, request_val->command, &request_val->req_time, pinfo);

	return 1;
}



static void
afpstat_draw(void *pss)
{
	afpstat_t *ss=(afpstat_t *)pss;

	draw_srt_table_data(&ss->afp_srt_table);
}


static void
win_destroy_cb(GtkWindow *win _U_, gpointer data)
{
	afpstat_t *ss=(afpstat_t *)data;

	remove_tap_listener(ss);

	free_srt_table_data(&ss->afp_srt_table);
	g_free(ss);
}


static void
gtk_afpstat_init(const char *opt_arg, void *userdata _U_)
{
	afpstat_t *ss;
	const char *filter=NULL;
	GtkWidget *label;
	char *filter_string;
	GString *error_string;
	int i;
	GtkWidget *vbox;
	GtkWidget *bbox;
	GtkWidget *close_bt;

	if(!strncmp(opt_arg,"afp,srt,",8)){
		filter=opt_arg+8;
	} else {
		filter=NULL;
	}

	ss=(afpstat_t *)g_malloc(sizeof(afpstat_t));

	ss->win=dlg_window_new("afp-stat");  /* transient_for top_level */
	gtk_window_set_destroy_with_parent (GTK_WINDOW(ss->win), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(ss->win), 550, 600);
	afpstat_set_title(ss);

	vbox=ws_gtk_box_new(GTK_ORIENTATION_VERTICAL, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(ss->win), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

	label=gtk_label_new("AFP Service Response Time statistics");
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	filter_string = g_strdup_printf("Filter: %s", filter ? filter : "");
	label=gtk_label_new(filter_string);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	g_free(filter_string);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	label=gtk_label_new("AFP Commands");
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	/* We must display TOP LEVEL Widget before calling init_srt_table() */
	gtk_widget_show_all(ss->win);

	init_srt_table(&ss->afp_srt_table, 256, vbox, "afp.command");
	for(i=0;i<256;i++){
		init_srt_table_row(&ss->afp_srt_table, i, val_to_str_ext(i, &CommandCode_vals_ext, "Unknown(%u)"));
	}


	error_string=register_tap_listener("afp", ss, filter, 0, afpstat_reset, afpstat_packet, afpstat_draw);
	if(error_string){
		simple_dialog(ESD_TYPE_ERROR, ESD_BTN_OK, "%s", error_string->str);
		g_string_free(error_string, TRUE);
		g_free(ss);
		return;
	}

	/* Button row. */
	bbox = dlg_button_row_new(GTK_STOCK_CLOSE, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

	close_bt = (GtkWidget *)g_object_get_data(G_OBJECT(bbox), GTK_STOCK_CLOSE);
	window_set_cancel_button(ss->win, close_bt, window_cancel_button_cb);

	g_signal_connect(ss->win, "delete_event", G_CALLBACK(window_delete_event_cb), NULL);
	g_signal_connect(ss->win, "destroy", G_CALLBACK(win_destroy_cb), ss);

	gtk_widget_show_all(ss->win);
	window_present(ss->win);

	cf_retap_packets(&cfile);
	gdk_window_raise(gtk_widget_get_window(ss->win));
}

static tap_param afp_stat_params[] = {
	{ PARAM_FILTER, "Filter", NULL }
};

static tap_param_dlg afp_stat_dlg = {
	"AFP SRT Statistics",
	"afp,srt",
	gtk_afpstat_init,
	-1,
	G_N_ELEMENTS(afp_stat_params),
	afp_stat_params
};

void
register_tap_listener_gtkafpstat(void)
{
	register_param_stat(&afp_stat_dlg, "AFP",
	    REGISTER_STAT_GROUP_RESPONSE_TIME);
}
