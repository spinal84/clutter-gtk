#include <gtk/gtk.h>
#include <clutter/clutter.h>

#include <clutter-gtk/clutter-gtk.h>

typedef struct {

  GtkWidget    *window;
  GtkWidget    *popup;
  GtkWidget    *gtk_entry;

  ClutterActor *stage;
  ClutterActor *hand;
  ClutterActor *clutter_entry;

} EventApp;

static void
on_gtk_entry_changed (GtkEditable *editable, EventApp *app)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (editable));

  clutter_entry_set_text (CLUTTER_ENTRY (app->clutter_entry), text);
}

static void
on_x_changed (GtkSpinButton *button, EventApp *app)
{
  clutter_actor_rotate_x (app->hand, 
                          gtk_spin_button_get_value (button),
                          clutter_actor_get_height (app->hand),
                          0);
}

static void
on_y_changed (GtkSpinButton *button, EventApp *app)
{
  clutter_actor_rotate_y (app->hand, 
                          gtk_spin_button_get_value (button),
                          clutter_actor_get_width (app->hand) /2,
                          0);
}

static void
on_z_changed (GtkSpinButton *button, EventApp *app)
{
  clutter_actor_rotate_z (app->hand, 
                          gtk_spin_button_get_value (button),
                          clutter_actor_get_width (app->hand) /2,
                          clutter_actor_get_height (app->hand)/2);
}

static void
on_opacity_changed (GtkSpinButton *button, EventApp *app)
{
  clutter_actor_set_opacity (app->hand, gtk_spin_button_get_value (button)); 
}

/* Set the clutter colors form the current gtk theme */
static void
create_colors (EventApp *app, ClutterColor *stage, ClutterColor *text)
{
  GtkStyle *style = gtk_widget_get_style (app->window);
  GdkColor color;

  /* Set the stage color to base[NORMAL] */
  color = style->bg[GTK_STATE_NORMAL];
  stage->red = (guint8) ((color.red/65535.0) * 255);
  stage->green = (guint8) ((color.green/65535.0) * 255);
  stage->blue  = (guint8) ((color.blue/65535.0) * 255);
  
  /* Now the text color */
  color = style->text[GTK_STATE_NORMAL];
  text->red =(guint8) ((color.red/65535.0) * 255);
  text->green = (guint8) ((color.green/65535.0) * 255);
  text->blue = (guint8) ((color.blue/65535.0) * 255);
}
  
gint
main (gint argc, gchar **argv)
{
  EventApp      *app = g_new0 (EventApp, 1);
  GtkWidget     *widget, *vbox, *hbox, *button, *label, *box;
  ClutterActor  *actor;
  GdkPixbuf     *pixbuf = NULL;
  guint          width, height;
  ClutterColor   stage_color = {255, 255, 255, 255};
  ClutterColor   text_color = {0, 0, 0, 255};

  clutter_init (&argc, &argv);
  
  gtk_init (&argc, &argv);

  /* Create the inital gtk window and widgets, just like normal */
  widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  app->window = widget;
  gtk_window_set_title (GTK_WINDOW (widget), "Gtk-Clutter Interaction demo");
  gtk_window_set_default_size (GTK_WINDOW (widget), 800, 600);
  gtk_window_set_resizable (GTK_WINDOW (widget), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (widget), 12);
  g_signal_connect (widget, "destroy", G_CALLBACK (gtk_main_quit), NULL);
 
  /* Create our layout box */
  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (app->window), vbox);

  widget = gtk_entry_new ();
  app->gtk_entry = widget;
  gtk_entry_set_text (GTK_ENTRY (widget), "Enter some text");
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
  g_signal_connect (widget, "changed", G_CALLBACK (on_gtk_entry_changed), app);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  
  /* Set up clutter & create our stage */
  create_colors (app, &stage_color, &text_color);
  widget = gtk_clutter_new ();
  gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);
  app->stage = gtk_clutter_get_stage (GTK_CLUTTER (widget));
  clutter_stage_set_color (CLUTTER_STAGE (app->stage), &stage_color);

  /* Create the main texture that the spin buttons manipulate */
  pixbuf = gdk_pixbuf_new_from_file ("redhand.png", NULL);
  if (pixbuf == NULL)
    g_error ("Unable to load pixbuf\n");

  actor = clutter_texture_new_from_pixbuf (pixbuf);
  app->hand = actor;
  clutter_group_add (CLUTTER_GROUP (app->stage), actor);
  clutter_actor_get_size (actor, &width, &height);
  clutter_actor_set_position (actor,
                              (CLUTTER_STAGE_WIDTH ()/2) - (width/2),
                              (CLUTTER_STAGE_HEIGHT ()/2) - (height/2));

  /* Setup the clutter entry */
  actor = clutter_entry_new_full ("Sans 10", "", &text_color);
  app->clutter_entry = actor;
  clutter_group_add (CLUTTER_GROUP (app->stage), actor);
  clutter_actor_set_position (actor, 0, 0);
  clutter_actor_set_size (actor, 500, 20);

  /* Create our adjustment widgets */
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  box = gtk_hbox_new (TRUE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, TRUE, 0);
  label = gtk_label_new ("Rotate x-axis");
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
  button = gtk_spin_button_new_with_range (0, 360, 1);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect (button, "value-changed", G_CALLBACK (on_x_changed), app);
  
  box = gtk_hbox_new (TRUE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, TRUE, 0);
  label = gtk_label_new ("Rotate y-axis");
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
  button = gtk_spin_button_new_with_range (0, 360, 1);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect (button, "value-changed", G_CALLBACK (on_y_changed), app);

  box = gtk_hbox_new (TRUE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, TRUE, 0);
  label = gtk_label_new ("Rotate z-axis");
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
  button = gtk_spin_button_new_with_range (0, 360, 1);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect (button, "value-changed", G_CALLBACK (on_z_changed), app);

  box = gtk_hbox_new (TRUE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, TRUE, 0);
  label = gtk_label_new ("Adjust opacity");
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
  button = gtk_spin_button_new_with_range (0, 255, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (button), 255);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
  g_signal_connect (button, "value-changed", G_CALLBACK (on_opacity_changed), app);

  clutter_actor_show_all (app->stage);
  gtk_widget_show_all (app->window);

  gtk_main ();
  
  return 0;
}
