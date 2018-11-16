
#include "xchomp.h"


/*
 * This file contains code which implements several special
 * sequences in the game.
 */


/* the get-ready sequence */
void get_ready()
{
   int          xx, yy, i;
   int          direction, ascent, descent;
   XCharStruct  chars;
   char         *string = "READY!";

   XQueryTextExtents(display, font, string, 6, &direction, &ascent,
      &descent, &chars);

   xx = (WIN_WIDTH - chars.width) / 2;
   yy = pac_sy + 1 + ascent;

   XDrawImageString(display, map, fullcopyGC, xx, yy, string, 6);
   XDrawImageString(display, window, fullcopyGC, xx, yy, string, 6);
   XSync(display, False);
   do_sleep(2);
   XCopyArea(display, save, map, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);
   XCopyPlane(display, lpac[0], map, orGC, 0, 0, GHOST_SIZE,
      GHOST_SIZE, pac_x, pac_y, 1);
   for (i = 0; i < num_ghosts; i++)
      XCopyPlane(display, bghost[0], map, orGC, 0, 0, GHOST_SIZE,
         GHOST_SIZE, ghost_x[i], ghost_y[i], 1);
   XCopyArea(display, map, window, fullcopyGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, 0);
   XSync(display, False);
   do_sleep(2);
}


/* the game-over sequence */
void game_over()
{
   int          xx, yy;
   int          direction, ascent, descent;
   XCharStruct  chars;
   char         *string = "GAME OVER";

   XQueryTextExtents(display, font, string, 9, &direction, &ascent,
      &descent, &chars);

   xx = (WIN_WIDTH - chars.width) / 2;
   yy = pac_sy + 1 + ascent;

   XDrawImageString(display, map, fullcopyGC, xx, yy, string, 9);
   XDrawImageString(display, window, fullcopyGC, xx, yy, string, 9);
   XSync(display, False);
   if (score > high_score)
      high_score = score;
   do_sleep(3);
}


/* the end-of-level sequence -- the screen flashes a few times */
void finish()
{
   int i;

   /* erase the fruit */
   XFillRectangle(display, save, clearGC, fruit_x - 2, fruit_y,
      FRUIT_WIDTH, FRUIT_HEIGHT);

   XCopyArea(display, save, map, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);
   XCopyPlane(display, lpac[0], map, orGC, 0, 0,
      GHOST_SIZE, GHOST_SIZE, pac_x, pac_y, 1);
   XCopyArea(display, map, window, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);
   XSync(display, False);
   print_score(100L * (level + 1));
   do_sleep(2);

   XCopyArea(display, save, map, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);
   XCopyArea(display, map, window, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT, 0, 0);
   for (i = 0; i < 7; i++) {
      XFillRectangle(display, map, invertGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
      XFillRectangle(display, window, invertGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
      XSync(display, False);
      do_usleep(350000);
   }
   XFillRectangle(display, map, clearGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
   XFillRectangle(display, window, clearGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
   XSync(display, False);
   do_sleep(2);
}


/* the paused-game sequence */
Bool pause_seq()
{
   XEvent         event;
   char           c_buf;
   XComposeStatus status;

   XCopyPlane(display, pause, window, fullcopyGC, 0, 0, TITLE_WIDTH,
      TITLE_HEIGHT, 12 * GHOST_SIZE, WIN_HEIGHT + 1, 1);
   XSync(display, False);

   while (True) {
      XNextEvent(display, &event);
      if (event.xany.window != window) continue;
      switch (event.type) {
         case KeyPress:
            XLookupString((XKeyEvent *) &event, &c_buf, 1, &last_key, &status);
            if ((last_key == XK_q) || (last_key == XK_Q))
               do_exit();
            if ((last_key == XK_r) || (last_key == XK_R))
               return False;
            display_title();
            XSync(display, False);
            return True;
         case Expose:
            XCopyArea(display, map, window, fullcopyGC, 0, 0,
               WIN_WIDTH, WIN_HEIGHT, 0, 0);
            restore_status();
               XCopyPlane(display, pause, window, fullcopyGC, 0, 0, TITLE_WIDTH,
               TITLE_HEIGHT, 12 * GHOST_SIZE, WIN_HEIGHT + 1, 1);
            break;
         case ClientMessage:
            if (event.xclient.data.l[0] == wm_delete_window_atom)
              do_exit();
            break;
         default: break;
      }
   }
}


void do_sleep(int secs)
{
   int i;

   for (i = 0; i < (secs * 40); i++) {
      check_normal_events();
      usleep(20000);
   }
}


void do_usleep(int usecs)
{
   int i, d, r;

   d = usecs / 25000;
   r = usecs % 25000;

   for (i = 0; i < d; i++) {
      check_normal_events();
      usleep(20000);
   }

   check_normal_events();
   usleep(r);
}


void check_normal_events()
{
   char            c_buf;
   XEvent          event;
   XComposeStatus  status;

   while (QLength(display) > 0) {
      XNextEvent(display, &event);
      if (event.xany.window != window) continue;
      switch (event.type) {
         case KeyPress:
            XLookupString((XKeyEvent *) &event, &c_buf, 1, &last_key, &status);
            if (last_key == XK_space)
               if (!pause_seq())
                  longjmp(jb_start, 1);
            break;
         case UnmapNotify:
            while (True) {
               XNextEvent(display, &event);
               if (event.xany.window != window) continue;
               if (event.type == MapNotify) break;
            }
            if (!pause_seq())
               longjmp(jb_start, 1);
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
               longjmp(jb_start, 1);
            break;
         case Expose:
            XCopyArea(display, map, window, fullcopyGC, 0, 0,
               WIN_WIDTH, WIN_HEIGHT, 0, 0);
            restore_status();
            break;
         case ClientMessage:
            if (event.xclient.data.l[0] == wm_delete_window_atom)
              do_exit();
            break;
         default: break;
      }
   }
   XSync(display, False);
}
