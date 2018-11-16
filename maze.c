
#include "xchomp.h"

/*
 * The following are the maze data arrays.  In order to avoid confusion,
 * and to ensure that nothing goes wrong, each maze should have the
 * following structure at the center.  This structure defines the
 * ghost box, the starting ghost positions, and the player/fruit
 * position:
 *
 *      q->D<-e
 *      |+++++|
 *      |GGGG+|
 *      z-----c
 *         P
 *
 *
 * Most of the characters in the maze data are used as indeces into
 * an array of pixmaps which define the images at the corresponding
 * location.
 */

static mazedata   mazes[] = {
    { "q---------w---------e",
      "|         |         |",
      "|O[-] [w] | [-] [-]O|",
      "|      |  |         |",
      "a-] [e v [x] [] ^ ^ |",
      "|    |          | | |",
      "| tu | q->D<-e [c v |",
      "| gj v |+++++|      |",
      "| gj   |GGGG+| tyyu |",
      "| bm ^ z-----c bnnm |",
      "|    |    P         |",
      "a-] [c [--w--] ^ o [d",
      "|         |    |    |",
      "|O[-----] v [--x--]O|",
      "|                   |",
      "z-------------------c" },

    { "q---------w---------e",
      "|         |         |",
      "|O[] q--] v [--e []O|",
      "|    |         |    |",
      "| tu v [-----] v tu |",
      "| gj             gj |",
      "| gj ^ q->D<-e ^ gj |",
      "| bm | |+++++| | bm |",
      "|    | |GGGG+| |    |",
      "| [] v z-----c v tu |",
      "|         P      gj |",
      "a--] ^ [-----] ^ bm |",
      "|    |         |    |",
      "|O[--x--] ^ [--x--]O|",
      "|         |         |",
      "z---------x---------c" },

    { "q-------------------e",
      "|                   |",
      "|O[--] ^ [-] ^ [--]O|",
      "|      |     |      |",
      "a--] [-x-] [-x-] [--d",
      "|                   |",
      "| tu ^ q->D<-e ^ tu |",
      "| gj | |+++++| | gj |",
      "| gj | |GGGG+| | gj |",
      "| bm v z-----c v bm |",
      "|         P         |",
      "| [-] q-] ^ [-e [-e |",
      "|     |   |   |   | |",
      "|O[-] | [-x-] | o vO|",
      "|     |       |     |",
      "z-----x-------x-----c" },

    { "q-------------------e",
      "|                   |",
      "|O[--] ^ [-] ^ [--]O|",
      "|      |     |      |",
      "a-] tu z-] [-c tu [-d",
      "|   gj         gj   |",
      "| ^ gj q->D<-e gj ^ |",
      "| | bm |+++++| bm | |",
      "| |    |GGGG+|    | |",
      "| v [e z-----c q] v |",
      "|    |    P    |    |",
      "a--] v [-----] v [--d",
      "|                   |",
      "|O[--] ^ [-] ^ [--]O|",
      "|      |     |      |",
      "z------x-----x------c" },

    { "q---------w---------e",
      "|         |         |",
      "|O^ [w] ^ v ^ [w] ^O|",
      "| |  |  |   |  |  | |",
      "| z] v [x] [x] v [c |",
      "|                   |",
      "| [e ^ q->D<-e ^ q] |",
      "|  v v |+++++| v v  |",
      "a]     |GGGG+|     [d",
      "|  ^ ^ z-----c ^ ^  |",
      "| [c |    P    | z] |",
      "|    z-] [w] [-c    |",
      "| tu      |      tu |",
      "|Obm [] ^ v ^ [] bmO|",
      "|       |   |       |",
      "z-------x---x-------c" },

    { "q---------w---------e",
      "|         |         |",
      "|O[-] [-] | [-] [-]O|",
      "|         |         |",
      "a-] [-] [-x-] [-] [-d",
      "|                   |",
      "| tyyu q->D<-e tyyu |",
      "| bnnm |+++++| bnnm |",
      "|      |GGGG+|      |",
      "| [-w] z-----c [w-] |",
      "|   |     P     |   |",
      "| ^ v q-] ^ [-e v ^ |",
      "| |   |   |   |   | |",
      "|Ov ^ v ^ v ^ v ^ vO|",
      "|   |   |   |   |   |",
      "z---x---x---x---x---c" } };


void read_maze(num)
int num;
{
   int          i, xx, yy;
   XRectangle  power_rect[MAX_POWER_DOTS], fruit_rect;

   for (i = 0; i < BLOCK_HEIGHT; i++)
      strncpy(md[i], mazes[num][i], BLOCK_WIDTH);

   /*
    * The rest of this function analyzes the maze data array,
    * and builds the maze image, as well as the dot information
    * array (dd[]).  The image is created on the background map (save),
    * and the power-dot images are created on the power-dot map (powermap).
    */

   numdots = 0;
   powerdots = 0;
   for (yy = 0; yy < BLOCK_HEIGHT; yy++)
      for (xx = 0; xx < BLOCK_WIDTH; xx++) {
         dd[yy][xx] = '\0';
         switch (md[yy][xx]) {

            case ' ':

                /* wherever there's a space, we'll put a dot */
                md[yy][xx] = '\0';
                dd[yy][xx] = '.';
                numdots++;
                break;

            case 'O':

                /* there is a power-dot here */
                md[yy][xx] = '\0';
                if (powerdots < MAX_POWER_DOTS) {
                   dd[yy][xx] = 'O';
                   power_rect[powerdots].x = xx * GHOST_SIZE;
                   power_rect[powerdots].y = yy * GHOST_SIZE;
                   power_rect[powerdots].width = GHOST_SIZE;
                   power_rect[powerdots].height = GHOST_SIZE;
                   powerdots++;
                   numdots++;
                   XCopyPlane(display, maze['O'], powermap, fullcopyGC, 0, 0,
                      GHOST_SIZE, GHOST_SIZE, xx * GHOST_SIZE,
                      yy * GHOST_SIZE, 1);
                }
                break;

            case 'P':

                /*
                 * This is the starting position of the player, as well
                 * as the location of the fruit when it appears.
                 */
                md[yy][xx] = '\0';
                pac_sx = fruit_x = xx * GHOST_SIZE;
                pac_sy = fruit_y = yy * GHOST_SIZE;
                fruit_rect.x = fruit_x - 2;
                fruit_rect.y = fruit_y;
                fruit_rect.width = FRUIT_WIDTH;
                fruit_rect.height = FRUIT_HEIGHT;
                break;

            case 'G':

                /*
                 * This is the starting position of a ghost.  It had
                 * better be somewhere at the bottom of the ghost box,
                 * and not in the rightmost position.  This is because
                 * initially, the ghosts will be sent to the right.
                 */
                md[yy][xx] = '\0';
                if (num_ghosts < MAX_GHOSTS) {
                   ghost_sx[num_ghosts] = xx * GHOST_SIZE;
                   ghost_sy[num_ghosts] = yy * GHOST_SIZE;
                   num_ghosts++;
                }
                break;

            case 'D':

                /*
                 * This is the position of the ghost box door.  It
                 * had better be placed correctly.
                 */
                door_x = xx;
                door_y = yy;
                break;

            case '+':

                /* this space should be left blank */
                md[yy][xx] = '\0';
                break;

            default: break;
         }
      }

   /*
    * The graphics context used to flash the power-dots will be loaded
    * with clipping information which defines only those areas of the
    * maze which contain the power-dots.
    */
   XDestroyRegion(power_region);
   power_region = XCreateRegion();
   for (i = 0; i < powerdots; i++)
      XUnionRectWithRegion(power_rect + i, power_region, power_region);
   XSetRegion(display, powerGC, power_region);

   /* create the fruit region */
   XDestroyRegion(fruit_region);
   fruit_region = XCreateRegion();
   XUnionRectWithRegion(&fruit_rect, fruit_region, fruit_region);

   /* build the maze image */
   for (yy = 0; yy < BLOCK_HEIGHT; yy++)
      for (xx = 0; xx < BLOCK_WIDTH; xx++) {
         if (dd[yy][xx])
            XCopyPlane(display, maze[dd[yy][xx]], save, fullcopyGC, 0, 0,
               GHOST_SIZE, GHOST_SIZE, xx * GHOST_SIZE,
               yy * GHOST_SIZE, 1);
         else
            XCopyPlane(display, maze[md[yy][xx]], save, fullcopyGC, 0, 0,
               GHOST_SIZE, GHOST_SIZE, xx * GHOST_SIZE,
               yy * GHOST_SIZE, 1);
      }
}


/*
 * The function which follows is used at the beginning of each level to
 * set up the initial parameters for all of the moving figures.
 */
void position_players()
{
   int         i;
   XRectangle  ghost_rect[MAX_GHOSTS], pac_rect;

   for (i = 0; i < num_ghosts; i++) {
      ghost_x[i] = ghost_sx[i];
      ghost_y[i] = ghost_sy[i];
      ghost_ix[i] = 2;
      ghost_iy[i] = 0;
      ghost[i] = bghost;
      drive[i] = hover;
      loops[i] = 0;
      contact[i] = die;
      ghost_rect[i].x = ghost_x[i] - 2;
      ghost_rect[i].y = ghost_y[i] - 2;
      ghost_rect[i].width = GHOST_SIZE + 4;
      ghost_rect[i].height = GHOST_SIZE + 4;
      XDestroyRegion(ghost_region[i]);
      ghost_region[i] = XCreateRegion();
      XUnionRectWithRegion(ghost_rect + i, ghost_region[i], ghost_region[i]);
   }

   pac_x = pac_sx;
   pac_y = pac_sy;
   pac_ix = (-2);
   pac_iy = 0;
   pac_rect.x = pac_x - 2;
   pac_rect.y = pac_y - 2;
   pac_rect.width = GHOST_SIZE + 4;
   pac_rect.height = GHOST_SIZE + 4;
   XDestroyRegion(pac_region);
   pac_region = XCreateRegion();
   XUnionRectWithRegion(&pac_rect, pac_region, pac_region);
   pac = lpac;
}

