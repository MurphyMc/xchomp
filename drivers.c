#include "xchomp.h"

/*
 * This file contains functions which control the motion of the player
 * and the ghosts.
 */

void get_dirs (char * pc, int x, int y, int * up, int * down, int * left, int * right)
{
  *up = *down = *left = *right = 1;

  if (x <= 0) {
    *left = *right = 0;
  }
  else if (x >= (BLOCK_WIDTH-1)) {
    *left = *right = 0;
  }
  else {
    *left = pc[-1];
    *right = pc[1];
    *up = pc[-BLOCK_WIDTH];
    *down = pc[BLOCK_WIDTH];
  }
}

/*
 * The following function is called explicitly during each animation
 * cycle, to control the motion of the player.  It updates the position
 * variables (pac_x, pac_y), the direction variables (pac_ix, pac_iy),
 * and the clipping region (pac_region).
 */
void control_pac()
{
   register int         xx = pac_x, yy = pac_y, i, dx, dy;
   register char        *pc = md[yy >> 4] + (xx >> 4);
   int                  up, down, left, right;
   get_dirs(pc, xx>>4, yy>>4, &up, &down, &left, &right);


   /* check for a collision */
   for (i = 0; i < num_ghosts; i++) {
      dx = ghost_x[i] - xx;
      dy = ghost_y[i] - yy;
      if ((abs(dx) < 6) && (abs(dy) < 6))
         (*contact[i])(i);
      if (dead) return;
   }

   /*
    * The rest of this function determines the direction of the
    * player according to the surroundings and the last key pressed
    * by the user.
    */

   if (!(xx & 0x0f) && !(yy & 0x0f)) {
      if (pac_ix > 0) {
         if (xx == WIN_WIDTH) {
            pac_x = -GHOST_SIZE;
            XOffsetRegion(pac_region, -WIN_WIDTH-GHOST_SIZE, 0);
         }
         else if (xx >= WIN_WIDTH-GHOST_SIZE) { }
         else if (right) pac_ix = 0;
      }
      else if (pac_ix < 0) {
         if (xx == -GHOST_SIZE) {
            pac_x = WIN_WIDTH;
            XOffsetRegion(pac_region, WIN_WIDTH+GHOST_SIZE, 0);
         }
         else if (xx <= 0) { }
         else if (left) pac_ix = 0;
      }
      else if (pac_iy < 0) {
         if (up) pac_iy = 0;
      }
      else if (down) pac_iy = 0;
      switch (last_key) {
         case XK_Up:
            if (!up) pac_iy = (-2), pac_ix = 0, pac = upac;
            break;
         case XK_Down:
            if (!down) pac_iy = 2, pac_ix = 0, pac = dpac;
            break;
         case XK_Left:
            if (!left) pac_ix = (-2), pac_iy = 0, pac = lpac;
            break;
         case XK_Right:
            if (!right) pac_ix = 2, pac_iy = 0, pac = rpac;
            break;
         default: break;
      }
      check_dots();
      pac_x += pac_ix; pac_y += pac_iy;
      XOffsetRegion(pac_region, pac_ix, pac_iy);
      return;
   }

   if (pac_ix > 0) {
      if (last_key == XK_Left)
         pac_ix = (-2), pac = lpac;
   }
   else if (pac_ix < 0) {
      if (last_key == XK_Right)
         pac_ix = 2, pac = rpac;
   }
   else if (pac_iy > 0) {
      if (last_key == XK_Up)
         pac_iy = (-2), pac = upac;
   }
   else if (last_key == XK_Down)
      pac_iy = 2, pac = dpac;

   pac_x += pac_ix;  pac_y += pac_iy;
   XOffsetRegion(pac_region, pac_ix, pac_iy);
}


/*
 * The following function checks to see whether the player has
 * eaten something which is not a ghost -- a dot, a power-dot,
 * or the fruit.  If so, the appropriate action is taken.
 */
void check_dots()
{
   register char        *pi;
   register int         i;
   register funcptr     driver;
   static long          fval[] = { 100, 200, 300, 300, 500, 700, 700,
                                   1000, 1000, 2000, 2000, 3000, 3000,
                                   5000 };

   /*
    * The following line produces a pointer to the character in the
    * dot information array (dd[]) which corresponds to the player's
    * position on the screen.
    */
   if (pac_x < 0) return;
   if (pac_y < 0) return;
   if (pac_x >= WIN_WIDTH) return;
   if (pac_y >= WIN_HEIGHT) return;
   pi = dd[pac_y >> 4] + (pac_x >> 4);

   /* check for a regular dot */
   if (*pi == '.') {
      *pi = '\0';

      /* erase the dot from the background image */
      XFillRectangle(display, save, clearGC, pac_x + 6,
         pac_y + 6, 4, 4);
      print_score(10L);
      if (--numdots == 0) {
         completed = True;
         return;
      }
   }

   /* check for a power-dot */
   else if (*pi == 'O') {
      *pi = '\0';

      /*
       * Here we'll erase the power-dot from both the power-dot
       * map and the background map, so that it no longer flashes.
       */
      XFillRectangle(display, powermap, clearGC, pac_x, pac_y,
         GHOST_SIZE, GHOST_SIZE);
      XCopyArea(display, powermap, save, fullcopyGC, pac_x, pac_y,
         GHOST_SIZE, GHOST_SIZE, pac_x, pac_y);
      print_score(50L);
      if (--numdots == 0) {
         completed = True;
         return;
      }

      /* set up ghost-eating mode */
      eat_index = 0;
      eat_mode = True;
      grey_tick = 0;
      count_sync = count;

      /*
       * Change the state of each solid ghost to that of a white
       * ghost running away from the player at half speed.
       */
      for (i = 0; i < num_ghosts; i++) {
         if ((driver = drive[i]) == follow) {
            drive[i] = run;
            contact[i] = eat;
            ghost[i] = gghost;
            ghost_ix[i] = -(ghost_ix[i]+sgn(ghost_ix[i])) / 2;
            ghost_iy[i] = -(ghost_iy[i]+sgn(ghost_iy[i])) / 2;
         }
         else if (driver == hover) {
            drive[i] = hover2;
            contact[i] = eat;
            ghost[i] = gghost;
            ghost_ix[i] = (ghost_ix[i]+sgn(ghost_ix[i])) / 2;
            ghost_iy[i] = (ghost_iy[i]+sgn(ghost_iy[i])) / 2;
         }
         else if ((driver == hover2) || (driver == run))
            ghost[i] = gghost;
      }
   }

   /* check for the fruit */
   else if (*pi == 'F') {
      *pi = '\0';
      print_score(fval[plevel]);

      /*
       * We have to do some fancy stuff here.  We want to instantly
       * change the fruit on the screen to the image of a score value,
       * without stopping the game (as when a ghost is eaten).  The problem
       * is that this subroutine is called AFTER the background image has
       * been restored onto the map in the game loop, and therefore, AFTER
       * the clipping information has been set for the pending screen
       * update.  Therefore, we have to copy this image onto BOTH off-
       * screen maps, and we have to reset the clipping information here,
       * so that the image is displayed on the screen immediately.  This
       * would not be a problem if the score value images were the same
       * size as the player, as in the case of dots and power-dots.
       */
      XCopyPlane(display, fval_pix[plevel], save, fullcopyGC,
         0, 0, FRUIT_WIDTH, FRUIT_HEIGHT, fruit_x - 2, fruit_y, 1);
      XCopyArea(display, save, map, fullcopyGC, fruit_x - 2,
         fruit_y, FRUIT_WIDTH, FRUIT_HEIGHT, fruit_x - 2, fruit_y);
      XUnionRegion(region[cr], fruit_region, region[cr ^ 1]);
      XSetRegion(display, copyGC, region[cr ^= 1]);

      /*
       * Now we'll set the fruit frame counter to 43.  The main loop will
       * clear the fruit area when it is at 50, so the fruit score value
       * will disappear nicely in a few seconds.
       */
      fruit_count = 43;
   }
}


/*-- GHOST DRIVERS -----------------------------------------------------*/


/*
 * The rest of this file contains ghost drivers.  These routines are invoked
 * through pointers, to control the motion of the ghosts.  There are several
 * of these routines, corresponding to the several different ghost states.
 * Each one modifies the direction/velocity arrays (ghost_ix[], ghost_iy[])
 * for the ghost whose array index is passed.
 */


/*
 * The function below causes ghosts to follow the player around, with a bit
 * of randomness thrown in as well.
 */
void follow(i)
register int i;
{
   int            xx = ghost_x[i], yy = ghost_y[i];
   register char  *pc = md[yy >> 4] + (xx >> 4);
   register int   dir = 0x0f, sense;
   register int   *px = ghost_ix + i, *py = ghost_iy + i;
   static intm    find[3] = { { 0, 1, 2 }, { 3, 3, 4 }, { 5, 6, 7 } };
   int            up, down, left, right;
   int            tx, ty;

   static intm  fxvec[16] = {
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* no way to go */
                { 2, 2, 2, 2, 2, 2, 2, 2 },             /* right only */
                { -2, -2, -2, -2, -2, -2, -2, -2 },     /* left only */
                { -2, 2, 2, -2, 2, -2, -2, 2 },         /* left or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* down only */
                { 2, 2, 2, 0, 2, 0, 0, 2 },             /* down or right */
                { -2, -2, -2, -2, 0, 0, 0, 0 },         /* down or left */
                { -2, -2, 2, -2, 2, -2, 0, 0 },         /* down, left, or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* up only */
                { 0, 0, 0, 0, 2, 2, 2, 2 },             /* up or right */
                { -2, 0, 0, -2, 0, -2, -2, -2 },        /* up or left */
                { 0, 0, 2, -2, 2, -2, -2, 2 },          /* up, left, or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* up or down */
                { 0, 0, 2, 0, 2, 0, 0, 2 },             /* up, down, or right */
                { -2, 0, 0, -2, 0, 0, 0, 0 },           /* up, down, or left */
                { -2, 0, 0, -2, 2, 0, 0, 2 } };         /* any which way */

   static intm  fyvec[16] = {
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* no way to go */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* right only */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* left only */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* left or right */
                { 2, 2, 2, 2, 2, 2, 2, 2 },             /* down only */
                { 0, 0, 0, 2, 0, 2, 2, 0 },             /* down or right */
                { 0, 0, 0, 0, 2, 2, 2, 2 },             /* down or left */
                { 0, 0, 0, 0, 0, 0, 2, 2 },             /* down, left, or right */
                { -2, -2, -2, -2, -2, -2, -2, -2 },     /* up only */
                { -2, -2, -2, -2, 0, 0, 0, 0 },         /* up or right */
                { 0, -2, -2, 0, -2, 0, 0, 0 },          /* up or left */
                { -2, -2, 0, 0, 0, 0, 0, 0 },           /* up, left, or right */
                { -2, -2, -2, -2, 2, 2, 2, 2 },         /* up or down */
                { -2, -2, 0, 2, 0, 2, 2, 0 },           /* up, down, or right */
                { 0, -2, -2, 0, -2, 2, 2, 2 },          /* up, down, or left */
                { 0, -2, -2, 0, 0, 2, 2, 0 } };         /* any which way */

   /* first, find the directions in which this ghost can go */
   get_dirs(pc, xx>>4, yy>>4, &up, &down, &left, &right);
   if (right || (*px < 0)) dir &= ~0x01;
   if (left || (*px > 0)) dir &= ~0x02;
   if (down || (*py < 0)) dir &= ~0x04;
   if (up || (*py > 0)) dir &= ~0x08;

   /* now choose the new direction for the ghost */
   if ((dir != 0x01) && (dir != 0x02) && (dir != 0x04) && (dir != 0x08)) {
      if ((random() & 0x0f) >= (i == 3 ? 4 : 2)) {
         if (i == 0) { // Red
            tx = pac_x; ty = pac_y;
         }
         else if (i == 1) { // Pink
            tx = pac_x; ty = pac_y;
            if (pac_ix > 0) tx += 2*GHOST_SIZE;
            else if (pac_ix < 0) tx -= 2*GHOST_SIZE;
            if (pac_iy > 0) ty += 2*GHOST_SIZE;
            else if (pac_iy < 0) ty -= 2*GHOST_SIZE;
         }
         else if (i == 2) { // Blue
            tx = pac_x; ty = pac_y;
            if (pac_ix > 0) tx += 2*GHOST_SIZE;
            else if (pac_ix < 0) tx -= 2*GHOST_SIZE;
            if (pac_iy > 0) ty += 2*GHOST_SIZE;
            else if (pac_iy < 0) ty -= 2*GHOST_SIZE;
            tx += 2 * (tx - ghost_x[0]);
            ty += 2 * (ty - ghost_y[0]);
         }
         else // Orange
         {
            // Less accurate of a translation of traditional behavior, but
            // we don't currently have scatter mode, so...
            tx = pac_x; ty = pac_y;
            if ((abs(tx-ghost_x[3])+abs(ty-ghost_y[3])) < (6*GHOST_SIZE)) {
              tx = WIN_WIDTH * (ghost_x[3] - pac_x);
              ty = WIN_HEIGHT * (ghost_y[3] - pac_y);
            }
         }

         sense = find[sgn(ty - yy) + 1][sgn(tx - xx) + 1];
         if ((dir&0x02) && ((xx>>4) <= 1) && ((pac_x>>4) > BLOCK_WIDTH - BLOCK_WIDTH/3))
         {
           *px = -1;
           *py = 0;
           return;
         }
         else if ((dir&0x01) && ((xx>>4) >= (BLOCK_WIDTH-2)) && ((pac_x>>4) < BLOCK_WIDTH/3))
         {
           *px = 1;
           *py = 0;
           return;
         }
      }
      else sense = random() & 0x07;
      *px = fxvec[dir][sense];
      *py = fyvec[dir][sense];
   }
   else {
      *px = *fxvec[dir];
      *py = *fyvec[dir];
   }
}


/*
 * The function below causes ghosts to run away from the player
 * at half speed.  It is set up as the driver function during
 * the ghost-eating periods of the game.
 */
void run(i)
register int i;
{
   int            xx = ghost_x[i], yy = ghost_y[i];
   register char  *pc = md[yy >> 4] + (xx >> 4);
   register int   dir = 0x0f, sense;
   register int   *px = ghost_ix + i, *py = ghost_iy + i;
   static intm    find[3] = { { 0, 1, 2 }, { 3, 3, 4 }, { 5, 6, 7 } };
   int            up, down, left, right;
   get_dirs(pc, xx>>4, yy>>4, &up, &down, &left, &right);

   static intm  rxvec[16] = {
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* no way to go */
                { 1, 1, 1, 1, 1, 1, 1, 1 },             /* right only */
                { -1, -1, -1, -1, -1, -1, -1, -1 },     /* left only */
                { 1, -1, -1, 1, -1, 1, 1, -1 },         /* left or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* down only */
                { 0, 1, 0, 0, 0, 1, 1, 0 },             /* down or right */
                { 0, -1, -1, 0, 0, 0, -1, -1 },         /* down or left */
                { 1, 1, -1, 0, 0, 1, 1, -1 },           /* down, left, or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* up only */
                { 1, 1, 0, 0, 0, 1, 1, 0 },             /* up or right */
                { 0, -1, -1, 0, 0, 0, -1, -1 },         /* up or left */
                { 1, -1, -1, 0, 0, 0, -1, -1 },         /* up, left, or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* up or down */
                { 0, 1, 0, 0, 0, 1, 1, 0 },             /* up, down, or right */
                { 0, -1, -1, 0, 0, 0, -1, -1 },         /* up, down, or left */
                { 1, -1, 0, 0, 0, 0, 1, -1 } };         /* any which way */

   static intm  ryvec[16] = {
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* no way to go */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* right only */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* left only */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* left or right */
                { 1, 1, 1, 1, 1, 1, 1, 1 },             /* down only */
                { 1, 0, 1, 1, 1, 0, 0, 1 },             /* down or right */
                { 1, 0, 0, 1, 1, 1, 0, 0 },             /* down or left */
                { 0, 0, 0, 1, 1, 0, 0, 0 },             /* down, left, or right */
                { -1, -1, -1, -1, -1, -1, -1, -1 },     /* up only */
                { 0, 0, -1, -1, -1, 0, 0, -1 },         /* up or right */
                { -1, 0, 0, -1, -1, -1, 0, 0 },         /* up or left */
                { 0, 0, 0, -1, -1, -1, 0, 0 },          /* up, left, or right */
                { 1, 1, 1, 1, -1, -1, -1, -1 },         /* up or down */
                { 1, 0, 1, 1, -1, 0, 0, -1 },           /* up, down, or right */
                { 1, 0, 0, -1, 1, -1, 0, 0 },           /* up, down, or left */
                { 0, 0, 1, 1, -1, -1, 0, 0 } };         /* any which way */

   /* first, find the directions in which this ghost can go */
   if (right || (*px < 0)) dir &= ~0x01;
   if (left || (*px > 0)) dir &= ~0x02;
   if (down || (*py < 0)) dir &= ~0x04;
   if (up || (*py > 0))  dir &= ~0x08;

   /* now choose the new direction for the ghost */
   if ((dir != 0x01) && (dir != 0x02) && (dir != 0x04) && (dir != 0x08)) {
      sense = find[sgn(pac_y - yy) + 1][sgn(pac_x - xx) + 1];
      *px = rxvec[dir][sense];
      *py = ryvec[dir][sense];
   }
   else {
      *px = *rxvec[dir];
      *py = *ryvec[dir];
   }
}


/*
 * The function below causes ghosts to return to the ghost box at
 * high speed.  It is set up as the driver for ghosts which have
 * been eaten.
 */
void go_home(i)
register int i;
{
   int            xx = ghost_x[i], yy = ghost_y[i];
   int            pmx = door_x << 4, pmy = (door_y - 1) << 4;
   register char  *pc = md[yy >> 4] + (xx >> 4);
   register int   dir = 0x0f, sense;
   register int   *px = ghost_ix + i, *py = ghost_iy + i;
   static intm    find[3] = { { 0, 1, 2 }, { 3, 3, 4 }, { 5, 6, 7 } };
   int            up, down, left, right;
   get_dirs(pc, xx>>4, yy>>4, &up, &down, &left, &right);

   static intm  pxvec[16] = {
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* no way to go */
                { 4, 4, 4, 4, 4, 4, 4, 4 },             /* right only */
                { -4, -4, -4, -4, -4, -4, -4, -4 },     /* left only */
                { -4, 4, 4, -4, 4, -4, -4, 4 },         /* left or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* down only */
                { 4, 4, 4, 0, 4, 0, 0, 4 },             /* down or right */
                { -4, -4, -4, -4, 0, 0, 0, 0 },         /* down or left */
                { -4, -4, 4, -4, 4, -4, 0, 0 },         /* down, left, or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* up only */
                { 0, 0, 0, 0, 4, 4, 4, 4 },             /* up or right */
                { -4, 0, 0, -4, 0, -4, -4, -4 },        /* up or left */
                { 0, 0, 4, -4, 4, -4, -4, 4 },          /* up, left, or right */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* up or down */
                { 0, 0, 4, 0, 4, 0, 0, 4 },             /* up, down, or right */
                { -4, 0, 0, -4, 0, 0, 0, 0 },           /* up, down, or left */
                { -4, 0, 0, -4, 4, 0, 0, 4 } };         /* any which way */

   static intm  pyvec[16] = {
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* no way to go */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* right only */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* left only */
                { 0, 0, 0, 0, 0, 0, 0, 0 },             /* left or right */
                { 4, 4, 4, 4, 4, 4, 4, 4 },             /* down only */
                { 0, 0, 0, 4, 0, 4, 4, 0 },             /* down or right */
                { 0, 0, 0, 0, 4, 4, 4, 4 },             /* down or left */
                { 0, 0, 0, 0, 0, 0, 4, 4 },             /* down, left, or right */
                { -4, -4, -4, -4, -4, -4, -4, -4 },     /* up only */
                { -4, -4, -4, -4, 0, 0, 0, 0 },         /* up or right */
                { 0, -4, -4, 0, -4, 0, 0, 0 },          /* up or left */
                { -4, -4, 0, 0, 0, 0, 0, 0 },           /* up, left, or right */
                { -4, -4, -4, -4, 4, 4, 4, 4 },         /* up or down */
                { -4, -4, 0, 4, 0, 4, 4, 0 },           /* up, down, or right */
                { 0, -4, -4, 0, -4, 4, 4, 4 },          /* up, down, or left */
                { 0, -4, -4, 0, 0, 4, 4, 0 } };         /* any which way */

   if (xx == pmx) {
      if ((yy >= pmy) && (yy <= (pmy + 48))) {
         if (yy == pmy) {

            /*
             * The ghost is right above the door to the ghost box.
             * We'll send it down into the box.  We're assuming
             * here that the ghost box is shaped a certain way.
             * If not, the results will be unpredictable.
             */
            *px = 0;
            *py = 4;
         }
         else if (yy == (pmy + 48)) {

            /*
             * The ghost is all the way inside the box.  Here it'll
             * be "reborn" -- its state will be changed to that of a
             * solid ghost hovering inside the ghost box.
             */
            drive[i] = hover;
            loops[i] = 0;
            ghost[i] = bghost;
            contact[i] = die;
            *px = 2;
            *py = 0;
         }
         return;
      }
   }

   /* otherwise, find the directions in which this ghost can go */
   if (right || (*px < 0)) dir &= ~0x01;
   if (left || (*px > 0)) dir &= ~0x02;
   if (down || (*py < 0)) dir &= ~0x04;
   if (up || (*py > 0))  dir &= ~0x08;

   /* now choose the new direction for the ghost */
   if ((dir != 0x01) && (dir != 0x02) && (dir != 0x04) && (dir != 0x08)) {
      sense = find[sgn(pmy - yy) + 1][sgn(pmx - xx) + 1];
      *px = pxvec[dir][sense];
      *py = pyvec[dir][sense];
   }
   else {
      *px = *pxvec[dir];
      *py = *pyvec[dir];
   }
}


/*
 * The function below drives the solid ghosts inside the ghost box.
 * They simply hover around in a circular pattern.  Randomness is
 * used to decide when the ghosts leave the box.
 */
void hover(i)
register int i;
{
   register int yy = ghost_y[i] >> 4, xx = ghost_x[i] >> 4;
   char         *pc = md[yy] + xx;
   register int *px = ghost_ix + i, *py = ghost_iy + i;
   int            up, down, left, right;
   get_dirs(pc, xx, yy, &up, &down, &left, &right);

   if (xx == door_x) {
      if (yy == (door_y - 1)) {

         /*
          * The ghost is now completely outside the box; we will
          * change its driver so that it follows the player around
          */
         drive[i] = follow;
         follow(i);
         return;
      }
      else if (yy == (door_y + 1))

         /*
          * The ghost is directly underneath the door to the
          * outside.  We'll use the number of loops it has made
          * inside the box, as well as a bit of randomness,
          * to determine whether or not to send it out.
          */
         if ((++loops[i]) > 1)
            if ((random() & 0x0f) > 7) {
               *px = 0, *py = (-2);
               return;
            }
   }

   /*
    * The rest of the function drives the ghost around the
    * box in a circular counterclockwise pattern.
    */
   if (*px > 0) {
      if (right) *px = 0, *py = (-2);
   }
   else if (*px < 0) {
      if (left) *px = 0, *py = 2;
   }
   else if (*py > 0) {
      if (down) *px = 2, *py = 0;
   }
   else if (up) *px = (-2), *py = 0;
}


/*
 * The function below is just like hover() above, except that
 * it handles the motion of ghosts inside the box during
 * the ghost-eating periods of the game -- they move at half
 * speed.
 */
void hover2(i)
register int i;
{
   register int yy = ghost_y[i] >> 4, xx = ghost_x[i] >> 4;
   char         *pc = md[yy] + xx;
   register int *px = ghost_ix + i, *py = ghost_iy + i;
   int            up, down, left, right;
   get_dirs(pc, xx, yy, &up, &down, &left, &right);

   if (xx == door_x) {
      if (yy == (door_y - 1)) {
         drive[i] = run;
         run(i);
         return;
      }
      else if (yy == (door_y + 1))
         if ((++loops[i]) > 1) {
            *px = 0, *py = (-1);
            return;
         }
   }

   if (*px > 0) {
      if (right) *px = 0, *py = (-1);
   }
   else if (*px < 0) {
      if (left) *px = 0, *py = 1;
   }
   else if (*py > 0) {
      if (down) *px = 1, *py = 0;
   }
   else if (up) *px = (-1), *py = 0;
}
