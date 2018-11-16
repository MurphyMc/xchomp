
#include "xchomp.h"

/*
 * This file contains the code which implements the title screen
 * for the game.
 */

void demo_seq()
{
   int             i, xx, yy, direction, ascent, descent, len;
   XCharStruct     chars;
   char            string[50], c_buf;
   XEvent          event;
   Bool            done = False;
   long            sc;
   XComposeStatus  status;

   /* clear the entire window and the map */
   XFillRectangle(display, window, clearGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT + GHOST_SIZE + 2);
   XFillRectangle(display, map, clearGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT + GHOST_SIZE + 2);

   /* draw the marble background */
   XSetFillStyle(display, fullcopyGC, FillStippled);
   XFillRectangle(display, map, fullcopyGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT + GHOST_SIZE + 2);
   XCopyArea(display, map, window, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT + GHOST_SIZE + 2, 0, 0);
   XSetFillStyle(display, fullcopyGC, FillSolid);

   /* draw the big title (on the map) */
   XSetFillStyle(display, orGC, FillStippled);
   xx = (WIN_WIDTH - (48 * 5 - 10)) / 2;
   yy = 25;
   for (i = 0; i < 5; i++) {
      XSetClipMask(display, orGC, demo_mask[i]);
      XSetClipMask(display, fullcopyGC, demo_mask[i]);
      XSetClipOrigin(display, orGC, xx + 8, yy + 8);
      XSetClipOrigin(display, fullcopyGC, xx, yy);
      XFillRectangle(display, map, orGC, xx + 8, yy + 8, 48, 48);
      XCopyPlane(display, demo_map[i], map, fullcopyGC, 0, 0,
         48, 48, xx, yy, 1);
      xx += (i ? 48 : 42);      /* compensate for the 'c' cut-off */
   }
   XSetFillStyle(display, orGC, FillSolid);
   XSetClipMask(display, orGC, None);
   XSetClipMask(display, fullcopyGC, None);
   XSetClipOrigin(display, orGC, 0, 0);
   XSetClipOrigin(display, fullcopyGC, 0, 0);

   /* programmer credits */
   xx = (WIN_WIDTH - CREDIT_WIDTH) / 2;
   yy = 89;
   XSetFillStyle(display, orGC, FillStippled);
   XFillRectangle(display, map, orGC, xx + 8, yy + 8,
      CREDIT_WIDTH, CREDIT_HEIGHT);
   XCopyPlane(display, credit, map, fullcopyGC, 0, 0,
      CREDIT_WIDTH, CREDIT_HEIGHT, xx, yy, 1);
   XSetFillStyle(display, orGC, FillSolid);

   /* draw the first demo panel */
   xx = (WIN_WIDTH - DEMOBOX_WIDTH) / 2;
   yy = 126;
   XSetFillStyle(display, orGC, FillStippled);
   XFillRectangle(display, map, orGC, xx + 8, yy + 8,
      DEMOBOX_WIDTH, DEMOBOX_HEIGHT);
   XCopyPlane(display, demobox, map, fullcopyGC, 0, 0,
      DEMOBOX_WIDTH, DEMOBOX_HEIGHT, xx, yy, 1);
   XSetFillStyle(display, orGC, FillSolid);

   /* draw the second demo panel */
   XQueryTextExtents(display, font, " ", 1, &direction, &ascent,
      &descent, &chars);
   xx = WIN_WIDTH / 4;
   yy = WIN_HEIGHT - 2 * GHOST_SIZE - descent - ascent - 20;
   XSetFillStyle(display, orGC, FillStippled);
   XFillRectangle(display, map, orGC, xx + 8, yy + 8,
      WIN_WIDTH / 2, 4 * GHOST_SIZE);
   XFillRectangle(display, map, clearGC, xx, yy,
      WIN_WIDTH / 2, 4 * GHOST_SIZE);
   XDrawRectangle(display, map, fullcopyGC, xx, yy,
      WIN_WIDTH / 2, 4 * GHOST_SIZE);
   XSetFillStyle(display, orGC, FillSolid);

   /* draw the high score */
   strcpy(string, "High Score: 000000");
   sc = high_score;
   for (i = 5; i >= 0; i--) {
      string[12 + i] = '0' + (sc % 10);
      sc /= 10;
   }
   len = strlen(string);
   XQueryTextExtents(display, font, string, len, &direction, &ascent,
      &descent, &chars);
   xx = (WIN_WIDTH - chars.width) / 2;
   yy = WIN_HEIGHT -  2 * GHOST_SIZE - descent - 10;
   XDrawImageString(display, map, fullcopyGC, xx, yy, string, len);

   /* draw some text */
   strcpy(string, "Press \'Q\' To Quit");
   len = strlen(string);
   XQueryTextExtents(display, font, string, len, &direction, &ascent,
      &descent, &chars);
   xx = (WIN_WIDTH - chars.width) / 2;
   yy = WIN_HEIGHT - GHOST_SIZE - descent - 10;
   XDrawImageString(display, map, fullcopyGC, xx, yy, string, len);

   /* draw some more text */
   strcpy(string, "Any Other Key To Begin");
   len = strlen(string);
   XQueryTextExtents(display, font, string, len, &direction, &ascent,
      &descent, &chars);
   xx = (WIN_WIDTH - chars.width) / 2;
   yy = WIN_HEIGHT - descent - 10;
   XDrawImageString(display, map, fullcopyGC, xx, yy, string, len);

   /* now copy the whole thing to the screen */
   XCopyArea(display, map, window, fullcopyGC, 0, 0,
      WIN_WIDTH, WIN_HEIGHT + GHOST_SIZE + 2, 0, 0);
   XSync(display, True);

   /* wait until the user hits a key */
   while (!done) {
      XNextEvent(display, &event);
      if (event.xany.window != window) continue;
      switch (event.type) {
         case KeyPress:
            XLookupString((XKeyEvent *) &event, &c_buf, 1, &last_key, &status);
            if ((last_key == XK_q) || (last_key == XK_Q))
               do_exit();
            XFillRectangle(display, window, clearGC, 0, 0, WIN_WIDTH,
               WIN_HEIGHT + GHOST_SIZE + 2);
            XSync(display, True);
            done = True;
            break;
         case Expose:
            XCopyArea(display, map, window, fullcopyGC, 0, 0,
               WIN_WIDTH, WIN_HEIGHT + GHOST_SIZE + 2, 0, 0);
            break;
         default: break;
      }
   }
}

