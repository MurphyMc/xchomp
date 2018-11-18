
#include "xchomp.h"

/*
 * This file contains player-ghost contact handling functions.
 * When a collision is detected, execution is vectored to one
 * of these functions, depending on the state of the ghost
 * with which the player collided.
 */


/*
 * The following function is called when the player collides with
 * a solid ghost; the player dies.  This is the death sequence.
 * The parameter to this function, as well as all of the collision
 * handling functions, is the number (array index) of the ghost
 * with which the player collided.  In this case, it doesn't matter.
 */
/*ARGSUSED*/
void die(dummy)
int dummy;
{
   register int xx = pac_x, yy = pac_y, i, dx, dy;
   XImage       *old, *new;

   /*
    * Since the player may be facing one of four directions at
    * the time of death, there would be way too many images for
    * me to draw in order to handle all possible cases.  Therefore, I
    * only drew the images for the case in which the player is facing
    * left -- they are stored in dead_prot[].  The images for the
    * other cases are generated here, dynamically, by using the
    * Xlib image facilities.  Since we can easily read and place
    * single pixels using images, we can rotate all the death frames
    * to the desired direction very easily.  The final series of
    * bitmaps is stored in deadpac[].
    */
   new = XGetImage(display, dead_prot[1], 0, 0, GHOST_SIZE,
      GHOST_SIZE, 1, XYPixmap);
   for (i = 0; i < 11; i++) {
      old = XGetImage(display, dead_prot[i], 0, 0, GHOST_SIZE,
         GHOST_SIZE, 1, XYPixmap);
      for (dy = 0; dy < GHOST_SIZE; dy++)
         for (dx = 0; dx < GHOST_SIZE; dx++)
            if (pac == lpac)
                XPutPixel(new, dx, dy, XGetPixel(old, dx, dy));
            else if (pac == rpac)
                XPutPixel(new, dx, dy, XGetPixel(old, GHOST_SIZE - dx - 1, dy));
            else if (pac == dpac)
                XPutPixel(new, dx, dy, XGetPixel(old, GHOST_SIZE - dy - 1, dx));
            else
                XPutPixel(new, dx, dy, XGetPixel(old, dy, dx));
      XPutImage(display, deadpac[i], bitmapGC, new, 0, 0, 0, 0,
         GHOST_SIZE, GHOST_SIZE);
      XDestroyImage(old);
   }
   XDestroyImage(new);
   do_sleep(1);

   /* deactivate the fruit (if displayed) */
   if (fruit_shown) {
      XFillRectangle(display, save, clearGC, fruit_x - 2, fruit_y,
         FRUIT_WIDTH, FRUIT_HEIGHT);
      dd[fruit_y >> 4][fruit_x >> 4] = '\0';
      ++fruit_times;
      fruit_shown = False;
   }

   /*
    * Now we'll get rid of all of the ghosts on the screen,
    * and display the first frame of the death animation at the
    * position of the player.
    */
   XCopyArea(display, save, map, fullcopyGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, 0);
   XCopyPlane(display, deadpac[0], map, orGC, 0, 0, GHOST_SIZE,
      GHOST_SIZE, xx, yy, 1);
   XCopyArea(display, map, window, fullcopyGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, 0);
   XSync(display, False);
   do_sleep(1);

   /*
    * Now we'll cycle through the death animation frames using
    * our normal animation mechanism.
    */
   for (i = 0; i < 11; i++) {
      XCopyArea(display, save, map, fullcopyGC, xx, yy, GHOST_SIZE,
         GHOST_SIZE, xx, yy);
      XCopyPlane(display, deadpac[i], map, orGC, 0, 0, GHOST_SIZE,
         GHOST_SIZE, xx, yy, 1);
      XCopyArea(display, map, window, fullcopyGC, xx, yy, GHOST_SIZE,
         GHOST_SIZE, xx, yy);
      XSync(display, False);
      do_usleep(125000);
   }
   do_usleep(150000);

   /*
    * Now we'll remove everything except the maze from the
    * screen, and return.
    */
   XCopyArea(display, save, map, fullcopyGC, xx, yy, GHOST_SIZE,
      GHOST_SIZE, xx, yy);
   XCopyArea(display, map, window, fullcopyGC, xx, yy, GHOST_SIZE,
      GHOST_SIZE, xx, yy);
   XSync(display, False);
   dead = True;
}


/*
 * The following function is executed when the player collides
 * with a transparent or flashing ghost; the player eats the
 * ghost.  The game pauses for a moment, displaying the value
 * of the eaten ghost, and then continues.  The parameter is
 * the array index of the eaten ghost.
 */
void eat(i)
int i;
{
   register int  xx = pac_x, yy = pac_y, j;
   static long   val[] = { 200, 400, 800, 1600 };

   /*
    * The image we want to create here is the following: all of
    * the ghosts except the one being eaten are drawn, as well as
    * the score value of the eaten ghost in the position of the
    * player.  We'll use the normal method: first create the image
    * on the map, and then copy the selected areas of the map onto
    * the window.  The "copyGC" graphics context, at this point,
    * should contain the necessary clipping information.
    */
   for (j = 0; j < num_ghosts; j++) {
      XSetForeground(display, orGC, colors[j].pixel);
      if (j != i)
         XCopyPlane(display, ghost[j][count], map, orGC, 0, 0,
            GHOST_SIZE, GHOST_SIZE, ghost_x[j], ghost_y[j], 1);
   }
   XSetForeground(display, orGC, black);
   XCopyPlane(display, eat_pix[eat_index], map, orGC, 0, 0,
      GHOST_SIZE, GHOST_SIZE, xx, yy, 1);
   XCopyArea(display, map, window, copyGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, 0);

   /* adjust the score and wait a second */
   print_score(val[eat_index]);
   eat_index = (eat_index + 1) & 0x03;
   XSync(display, False);
   do_sleep(1);

   /* remove all moving figures from the map */
   XCopyArea(display, save, map, copyGC, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, 0);

   /*
    * Now we'll change the ghost state, so that it appears
    * as a pair of harmless eyes seeking return to the ghost
    * box.
    */
   ghost[i] = eghost;
   drive[i] = go_home;
   contact[i] = noop;
}


/*
 * The following is the collision handler for ghost-eyes.
 * The eyes are harmless; this is a no-op.
 */
/*ARGSUSED*/
void noop(dummy)
int dummy;
{
}

