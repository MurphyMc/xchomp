
#define EXTERN
#include "xchomp.h"

int main(argc, argv)
int argc;
char *argv[];
{
   XEvent         event;
   int            dummy;
   XCharStruct    chars;
   unsigned long  event_mask;
   int            fdelay = 0;

   /* User may set FRAME_DELAY from the command line */
   if (argc >= 2) {
      fdelay = atoi(argv[1]);
   }

   /* open the display */
   display = XOpenDisplay(NULL);
   screen  = DefaultScreen(display);
   root    = DefaultRootWindow(display);
   depth   = DefaultDepth(display, screen);
   black   = BlackPixel(display, screen);
   white   = WhitePixel(display, screen);

   /* get a font */
   font = XLoadFont(display, "fixed");
   XQueryTextExtents(display, font, "000000", 6, &dummy,
      &ascent, &descent, &chars);

   /*
    * We want to suspend the game in case the window is iconified.
    * This is more difficult than it sounds.  On the Sun, iconification
    * seems to produce an UnmapNotify event -- very nice.  DECwindows,
    * however, informs the application by generating a PropertyNotify
    * event on a DEC-specific property -- very nasty.  The atom is
    * not defined in any of the DECwindows headers, so we will try
    * to get its value from the server, and use it later.  We are
    * hoping here that all non-DECwindows servers will return None
    * for this atom.
    */
   DEC_icon_atom = XInternAtom(display, "DEC_WM_ICON_STATE", True);

   /* assemble resources */
   create_ghost();
   create_pac();
   create_fruit();
   create_maze_symbols();
   create_demo_images();
   create_GCs();
   create_window(argc, argv);
   create_maps();
   create_regions();

   /* select the event mask for the window */
   event_mask = ExposureMask | KeyPressMask;
   if (DEC_icon_atom == None)
      event_mask |= StructureNotifyMask;
   else event_mask |= PropertyChangeMask;
   XSelectInput(display, window, event_mask);

   /* display the window */
   XMapWindow(display, window);
   while (True) {
      XNextEvent(display, &event);
      if (event.xany.window != window) continue;
      if (event.type == Expose) break;
   }

   /* go to it */
   play_game(fdelay);

   /* exit */
   do_exit();
   return 0;
}


/*
 * The following function contains the main game loop.
 */
void play_game(int fdelay) {
   register int    i;
   char            c_buf;
   XComposeStatus  status;
   XRectangle      xrect;
   XEvent          event;

   static int      flash_ticks[] = {
                      13, 8, 4, 1, 13, 8, 4, 1,
                      8, 4, 1, 4, 1, 8, 4, 1,
                      0, 0, 8, 4, 0, 0, 1, 0 };
   static int      off_ticks[] = {
                      19, 14, 10, 7, 19, 14, 10, 7,
                      14, 10, 7, 10, 7, 14, 10, 7,
                      1, 1, 14, 10, 1, 1, 7, 1 };
   static int      screens[] = {
                      1, 1, 1, 1, 2, 2, 2, 2,
                      3, 3, 3, 4, 4, 5, 5, 5,
                      1, 2, 6, 6, 3, 4, 6, 5 };

   high_score = 0L;

   setjmp(jb_start);

   demo:

   /* run the demo screen */
   demo_seq();

   /* initialize the game */
   lives = 3;
   level = (-1);
   score = 0L;
   display_title();
   print_score(0L);

   new_screen:

   /* advance the level */
   plevel = (++level > 13) ? 13 : level;
   flash_tick = flash_ticks[level % 24];
   off_tick = off_ticks[level % 24];
   display_level(True);

   /* initialize dynamic parameters */
   completed = False;
   fruit_times = 0;

   /* build the maze */
   clear_maps();
   read_maze(screens[level % 24] - 1);

   new_life:

   /* initialize more dynamic parameters */
   last_key = XK_Left;
   dead = False;
   eat_mode = False;
   count = (-1);
   fruit_count = (-1);
   fruit_shown = False;
   position_players();

   /* create the clipping regions */
   XDestroyRegion(region[0]);      XDestroyRegion(region[1]);
   region[0] = XCreateRegion();    region[1] = XCreateRegion();
   cr = 0;

   /* display the number of lives */
   (void)set_lives(lives);

   /* copy the maze to the map and the screen */
   XCopyArea(display, save, map, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);
   XCopyArea(display, map, window, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);

   /* display the ready message */
   get_ready();


   /*-- The Animation Loop ----------------------------------------*/

   while (True) {

      /*-- Xlib Event Section -------------------------------------*/

      while (QLength(display) > 0) {
         XNextEvent(display, &event);
         if (event.xany.window != window) continue;
         switch (event.type) {
            case KeyPress:
               XLookupString((XKeyEvent *) &event, &c_buf, 1, &last_key, &status);
               if (last_key == XK_space)
                  if (!pause_seq())
                     goto demo;
               break;
            case UnmapNotify:
               while (True) {
                  XNextEvent(display, &event);
                  if (event.xany.window != window) continue;
                  if (event.type == MapNotify) break;
               }
               if (!pause_seq())
                  goto demo;
               break;
            case PropertyNotify:
               if (event.xproperty.atom != DEC_icon_atom) break;
               while (True) {
                  XNextEvent(display, &event);
                  if (event.xany.window != window) continue;
                  if (event.type != PropertyNotify) continue;
                  if (event.xproperty.atom == DEC_icon_atom) break;
               }
               if (!pause_seq())
                  goto demo;
               break;
            case Expose:
               XCopyArea(display, map, window, fullcopyGC, 0, 0,
                  WIN_WIDTH, WIN_HEIGHT, 0, 0);
               restore_status();
               display_title();
               break;
            default: break;
         }
      }

      /*-- Adjust Frame Counter -----------------------------------*/

      count = (count + 1) & 0x0f;

      /*-- Clear Clipping Region ----------------------------------*/

      XSubtractRegion(region[cr], full_region, region[cr ^ 1]);
      cr ^= 1;

      /*-- Flashing Power-Dot And Fruit Section -------------------*/

      if (count == 0) {

         /* it's time to flash the power-dots */
         XCopyArea(display, powermap, save, powerGC, 0, 0,
            WIN_WIDTH, WIN_HEIGHT, 0, 0);
         XUnionRegion(region[cr], power_region, region[cr ^ 1]);
         cr ^= 1;

         /* see if it's time to display or erase the fruit */
         if (fruit_times < 2) {
            if (++fruit_count == 30) {
               XCopyPlane(display, fruit_pix[plevel], save, fullcopyGC,
                  0, 0, GHOST_SIZE, GHOST_SIZE, fruit_x, fruit_y, 1);
               XUnionRegion(region[cr], fruit_region, region[cr ^ 1]);
               cr ^= 1;
               dd[fruit_y >> 4][fruit_x >> 4] = 'F';
               fruit_shown = True;
            }
            else if (fruit_count == 50) {
               XFillRectangle(display, save, clearGC, fruit_x - 2,
                  fruit_y, FRUIT_WIDTH, FRUIT_HEIGHT);
               XUnionRegion(region[cr], fruit_region, region[cr ^ 1]);
               cr ^= 1;
               dd[fruit_y >> 4][fruit_x >> 4] = '\0';
               fruit_count = 0;
               ++fruit_times;
               fruit_shown = False;
            }
         }
      }

      /*-- Set Clipping Information -------------------------------*/

      for (i = 0; i < num_ghosts; i++, cr ^= 1)
         XUnionRegion(region[cr], ghost_region[i], region[cr ^ 1]);
      XUnionRegion(region[cr], pac_region, region[cr ^ 1]), cr ^= 1;
      XSetRegion(display, copyGC, region[cr]);

      /*-- Restore Background Image -------------------------------*/

      XCopyArea(display, save, map, copyGC, 0, 0, WIN_WIDTH,
         WIN_HEIGHT, 0, 0);

      /*-- Motion Control Section ---------------------------------*/

      control_pac();
      if (dead || completed) break;
      for (i = 0; i < num_ghosts; i++)
         if (!(ghost_x[i] & 0x0f) && !(ghost_y[i] & 0x0f))
            (*drive[i])(i);
      for (i = 0; i < num_ghosts; i++) {
         ghost_x[i] += ghost_ix[i];  ghost_y[i] += ghost_iy[i];
         XOffsetRegion(ghost_region[i], ghost_ix[i], ghost_iy[i]);
      }

      /*-- Flashing Ghost Section ---------------------------------*/

      /*
       * If we're in the middle of a ghost-eating period, this section
       * handles the timing and changes ghost states when necessary.
       */
      if (eat_mode)
         if (count == count_sync) {
            ++grey_tick;
            if (grey_tick == flash_tick) {
               for (i = 0; i < num_ghosts; i++)
                  if (ghost[i] == gghost)
                     ghost[i] = fghost;
            }
            else if (grey_tick == off_tick) {
               eat_mode = False;
               for (i = 0; i < num_ghosts; i++)
                  if (drive[i] == run) {
                     ghost[i] = bghost;
                     contact[i] = die;
                     drive[i] = follow;
                     ghost_ix[i] *= 2;  ghost_iy[i] *= 2;
                     XClipBox(ghost_region[i], &xrect);
                     XOffsetRegion(ghost_region[i],
                        (ghost_x[i] &= ~1) - 2 - xrect.x,
                        (ghost_y[i] &= ~1) - 2 - xrect.y);
                  }
                  else if (drive[i] == hover2) {
                     ghost[i] = bghost;
                     contact[i] = die;
                     drive[i] = hover;
                     ghost_ix[i] *= 2;  ghost_iy[i] *= 2;
                     XClipBox(ghost_region[i], &xrect);
                     XOffsetRegion(ghost_region[i],
                        (ghost_x[i] &= ~1) - 2 - xrect.x,
                        (ghost_y[i] &= ~1) - 2 - xrect.y);
                  }
            }
         }

      /*-- Offscreen Figure Overlay -------------------------------*/

      for (i = 0; i < num_ghosts; i++)
         XCopyPlane(display, ghost[i][count], map, orGC, 0, 0,
            GHOST_SIZE, GHOST_SIZE, ghost_x[i], ghost_y[i], 1);
      XCopyPlane(display, pac[count], map, orGC, 0, 0,
         GHOST_SIZE, GHOST_SIZE, pac_x, pac_y, 1);

      /*-- Screen Update ------------------------------------------*/

      XCopyArea(display, map, window, copyGC, 0, 0, WIN_WIDTH,
         WIN_HEIGHT, 0, 0);

      /*-- Synchronization And Delay ------------------------------*/

      XSync(display, False);

#ifdef FRAME_DELAY
      if (fdelay > 0) {
         usleep(fdelay);
      } else {
         usleep(FRAME_DELAY);
      }
#endif

   } /* while */

   /*-- End of Animation Loop -------------------------------------*/

   if (dead) {
      do_sleep(2);
      if (set_lives(lives - 1))
         goto new_life;
      game_over();
      goto demo;
   }

   if (completed) {
      finish();
      goto new_screen;
   }
}


void do_exit()
{
   destroy_regions();
   XUnmapWindow(display, window);
   XUnloadFont(display, font);
   XFlush(display);
   XCloseDisplay(display);
   exit(1);
}
