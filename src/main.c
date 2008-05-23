/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include "osm-gps-map.h"

//1=google, 2=oam, 3=osm
#define MAP_PROVIDER 3

gboolean
on_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	OsmGpsMap *map = OSM_GPS_MAP(widget);

	if ( (event->button == 1) && (event->type == GDK_2BUTTON_PRESS) )
	{
		coord_t coord;
		g_debug("Double clicked %f %f", event->x, event->y);
		coord = osm_gps_map_get_co_ordinates(map, (int)event->x, (int)event->y);
		osm_gps_map_draw_gps (map, coord.lat,coord.lon, 0);
	}
	return FALSE;
}

gboolean
on_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	float lat,lon;
	GtkEntry *entry = GTK_ENTRY(user_data);
	OsmGpsMap *map = OSM_GPS_MAP(widget);

	g_object_get(map, "latitude", &lat, "longitude", &lon, NULL);
	gchar *msg = g_strdup_printf("%f,%f",lat,lon);
	gtk_entry_set_text(entry, msg);
	g_free(msg);

	return FALSE;
}

gboolean 
on_zoom_in_clicked_event (GtkWidget *widget, gpointer user_data)
{
	int zoom;
	OsmGpsMap *map = OSM_GPS_MAP(user_data);
	g_object_get(map, "zoom", &zoom, NULL);
	osm_gps_map_set_zoom(map, zoom+1);
	return FALSE;
}

gboolean 
on_zoom_out_clicked_event (GtkWidget *widget, gpointer user_data)
{
	int zoom;
	OsmGpsMap *map = OSM_GPS_MAP(user_data);
	g_object_get(map, "zoom", &zoom, NULL);
	osm_gps_map_set_zoom(map, zoom-1);
	return FALSE;
}

gboolean 
on_home_clicked_event (GtkWidget *widget, gpointer user_data)
{
	OsmGpsMap *map = OSM_GPS_MAP(user_data);
	osm_gps_map_set_mapcenter(map, -43.5326,172.6362,12);
	return FALSE;
}

int
main (int argc, char **argv)
{
	GtkWidget *vbox;
	GtkWidget *bbox;
	GtkWidget *entry;
	GtkWidget *window;
	GtkWidget *zoomInbutton;
	GtkWidget *zoomOutbutton;
	GtkWidget *homeButton;

	GtkWidget *map;

	g_thread_init(NULL);
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
	
#if MAP_PROVIDER == 1
	//According to 
	//http://www.mgmaps.com/cache/MapTileCacher.perl
	//the v string means:
	//  w2.99		Maps
	//  w2t.99		Hybrid
	//  w2p.99		Photo
	map = g_object_new (OSM_TYPE_GPS_MAP,
						"repo-uri","http://mt.google.com/mt?n=404&v=w2p.99&x=%d&y=%d&zoom=%d",
						"tile-cache","/tmp/Maps/Google",
						"invert-zoom",TRUE,
						NULL);
#elif MAP_PROVIDER == 2
	map = g_object_new (OSM_TYPE_GPS_MAP,
						"repo-uri","http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/%d/%d/%d.jpg",
						"tile-cache","/tmp/Maps/OAM",
						NULL);
#elif MAP_PROVIDER == 3
	map = osm_gps_map_new ();
#else
	#error select map provider
#endif
    vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	//Add the map to the box
	gtk_box_pack_start (GTK_BOX(vbox), map, TRUE, TRUE, 0);
	//And add a box for the buttons
    bbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	//And add the lat/long entry
	entry = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(vbox), entry, FALSE, TRUE, 0);	

	//Add buttons to the bbox
	zoomInbutton = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
    g_signal_connect (G_OBJECT (zoomInbutton), "clicked",
		      G_CALLBACK (on_zoom_in_clicked_event), (gpointer) map);
	gtk_box_pack_start (GTK_BOX(bbox), zoomInbutton, FALSE, TRUE, 0);

	zoomOutbutton = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
    g_signal_connect (G_OBJECT (zoomOutbutton), "clicked",
		      G_CALLBACK (on_zoom_out_clicked_event), (gpointer) map);
	gtk_box_pack_start (GTK_BOX(bbox), zoomOutbutton, FALSE, TRUE, 0);

	homeButton = gtk_button_new_from_stock (GTK_STOCK_HOME);
    g_signal_connect (G_OBJECT (homeButton), "clicked",
		      G_CALLBACK (on_home_clicked_event), (gpointer) map);
	gtk_box_pack_start (GTK_BOX(bbox), homeButton, FALSE, TRUE, 0);

	//Connect to map events
  	g_signal_connect (map, "button-press-event",
    		G_CALLBACK (on_button_press_event), (gpointer) entry);
	g_signal_connect (map, "button-release-event",
            G_CALLBACK (on_button_release_event), (gpointer) entry);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show_all (window);

	gtk_main ();
	return 0;
}
