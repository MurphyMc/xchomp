
#include "xchomp.h"
#include "bitmaps.h"


/*
 * This file contains all of the functions which build the image and
 * window/pixmap resources for the game, as well as the graphics
 * contexts.
 */

void create_window(argc, argv)
int argc;
char *argv[];
{
   XSizeHints   hints;
   XWMHints     xwmh;
   char         *name = "xchomp";

   window = XCreateSimpleWindow(display, root, 0, 0, WIN_WIDTH,
      WIN_HEIGHT + GHOST_SIZE + 2, 1, black, white);

   icon = XCreatePixmapFromBitmapData(display, root, icon_bits,
      ICON_WIDTH, ICON_HEIGHT, 1, 0, 1);

   hints.flags = PSize | PMinSize | PMaxSize;
   hints.width = hints.min_width = hints.max_width = WIN_WIDTH;
   hints.height = hints.min_height =
      hints.max_height = WIN_HEIGHT + GHOST_SIZE + 2;

   XSetStandardProperties(display, window, name, name, icon,
      argv, argc, &hints);

   xwmh.input = True;
   xwmh.initial_state = NormalState;
   xwmh.icon_pixmap = icon;
   xwmh.flags = InputHint | StateHint | IconPixmapHint;
   XSetWMHints(display, window, &xwmh);
}


void create_pac()
{
   int i;

   lpac[3] = XCreatePixmapFromBitmapData(display, root, pacl1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   lpac[2] = XCreatePixmapFromBitmapData(display, root, pacl2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   lpac[1] = XCreatePixmapFromBitmapData(display, root, pacl3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   lpac[0] = XCreatePixmapFromBitmapData(display, root, pacl4_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   lpac[4] = lpac[3];
   lpac[5] = lpac[2];
   lpac[6] = lpac[1];
   lpac[7] = lpac[0];
   for (i = 8; i < 16; i++)
      lpac[i] = lpac[i - 8];

   rpac[3] = XCreatePixmapFromBitmapData(display, root, pacr1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   rpac[2] = XCreatePixmapFromBitmapData(display, root, pacr2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   rpac[1] = XCreatePixmapFromBitmapData(display, root, pacr3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   rpac[0] = lpac[0];
   rpac[4] = rpac[3];
   rpac[5] = rpac[2];
   rpac[6] = rpac[1];
   rpac[7] = rpac[0];
   for (i = 8; i < 16; i++)
      rpac[i] = rpac[i - 8];

   upac[3] = XCreatePixmapFromBitmapData(display, root, pacu1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   upac[2] = XCreatePixmapFromBitmapData(display, root, pacu2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   upac[1] = XCreatePixmapFromBitmapData(display, root, pacu3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   upac[0] = lpac[0];
   upac[4] = upac[3];
   upac[5] = upac[2];
   upac[6] = upac[1];
   upac[7] = upac[0];
   for (i = 8; i < 16; i++)
      upac[i] = upac[i - 8];

   dpac[3] = XCreatePixmapFromBitmapData(display, root, pacd1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dpac[2] = XCreatePixmapFromBitmapData(display, root, pacd2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dpac[1] = XCreatePixmapFromBitmapData(display, root, pacd3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dpac[0] = lpac[0];
   dpac[4] = dpac[3];
   dpac[5] = dpac[2];
   dpac[6] = dpac[1];
   dpac[7] = dpac[0];
   for (i = 8; i < 16; i++)
      dpac[i] = dpac[i - 8];

   dead_prot[0] = lpac[0];
   dead_prot[1] = lpac[1];
   dead_prot[2] = lpac[2];
   dead_prot[3] = lpac[3];
   dead_prot[4] = XCreatePixmapFromBitmapData(display, root, pdie4_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dead_prot[5] = XCreatePixmapFromBitmapData(display, root, pdie5_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dead_prot[6] = XCreatePixmapFromBitmapData(display, root, pdie6_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dead_prot[7] = XCreatePixmapFromBitmapData(display, root, pdie7_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dead_prot[8] = XCreatePixmapFromBitmapData(display, root, pdie8_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dead_prot[9] = XCreatePixmapFromBitmapData(display, root, pdie9_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   dead_prot[10] = XCreatePixmapFromBitmapData(display, root, pdie10_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);

   for (i = 0; i < 11; i++)
      deadpac[i] = XCreatePixmap(display, root, GHOST_SIZE, GHOST_SIZE, 1);

   small_pac = XCreatePixmapFromBitmapData(display, root, pacsmall_bits,
      12, GHOST_SIZE, 1, 0, 1);
}


void create_ghost()
{
   int i;

   bghost[0] = XCreatePixmapFromBitmapData(display, root, frame1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   bghost[2] = XCreatePixmapFromBitmapData(display, root, frame2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   bghost[6] = XCreatePixmapFromBitmapData(display, root, frame3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   bghost[1] = bghost[4] = bghost[5] = bghost[0];
   bghost[3] = bghost[2];
   bghost[7] = bghost[6];
   for (i = 8; i < 16; i++)
      bghost[i] = bghost[i - 8];

   gghost[0] = XCreatePixmapFromBitmapData(display, root, grey1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   gghost[2] = XCreatePixmapFromBitmapData(display, root, grey2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   gghost[6] = XCreatePixmapFromBitmapData(display, root, grey3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   gghost[1] = gghost[4] = gghost[5] = gghost[0];
   gghost[3] = gghost[2];
   gghost[7] = gghost[6];
   for (i = 8; i < 16; i++)
      gghost[i] = gghost[i - 8];

   for (i = 0; i <  8; i++) fghost[i] = gghost[i];
   for (i = 8; i < 16; i++) fghost[i] = bghost[i];

   eghost[0] = XCreatePixmapFromBitmapData(display, root, eye_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   for (i = 1; i < 16; i++) eghost[i] = eghost[0];
}


void create_maze_symbols()
{
   int          i;
   Pixmap       empty;
   static char  index[] = { 'a', 'b', 'c', 'd', 'v', 'e', 'g', '-',
                            'j', '[', 'm', 'n', 'o', 'q', ']', 's',
                            't', 'u', '^', '|', 'w', 'x', 'y', 'z',
                            '.', 'O', 'D', '<', '>' };
   static char  *data[] = { ma_bits, mb_bits,
                            mc_bits, md_bits, mdown_bits, me_bits, mg_bits,
                            mhorz_bits, mj_bits, mleft_bits, mm_bits, mn_bits,
                            mo_bits, mq_bits, mright_bits, ms_bits, mt_bits,
                            mu_bits, mup_bits, mvert_bits, mw_bits, mx_bits,
                            my_bits, mz_bits, mdot_bits, mpower_bits,
                            mdoor_bits, mless_bits, mgreat_bits };

   empty = XCreatePixmapFromBitmapData(display, root, mempty_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   for (i = 0; i < 128; maze[i++] = empty);

   for (i = 0; i < sizeof(index); i++)
      maze[index[i]] = XCreatePixmapFromBitmapData(display, root,
         data[i], GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
}


void create_maps()
{
   powermap = XCreatePixmap(display, root, WIN_WIDTH, WIN_HEIGHT, depth);
   save = XCreatePixmap(display, root, WIN_WIDTH, WIN_HEIGHT, depth);
   map  = XCreatePixmap(display, root, WIN_WIDTH,
      WIN_HEIGHT + GHOST_SIZE + 2, depth);
}


void create_fruit()
{
   fruit_pix[0] = XCreatePixmapFromBitmapData(display, root, fcherry_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[1] = XCreatePixmapFromBitmapData(display, root, fstraw_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[2] = XCreatePixmapFromBitmapData(display, root, fwater_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[3] = fruit_pix[2];
   fruit_pix[4] = XCreatePixmapFromBitmapData(display, root, fapple_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[5] = XCreatePixmapFromBitmapData(display, root, fgrape_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[6] = fruit_pix[5];
   fruit_pix[7] = XCreatePixmapFromBitmapData(display, root, fbell_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[8] = fruit_pix[7];
   fruit_pix[9] = XCreatePixmapFromBitmapData(display, root, fclock_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[10] = fruit_pix[9];
   fruit_pix[11] = XCreatePixmapFromBitmapData(display, root, fxlogo_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   fruit_pix[12] = fruit_pix[11];
   fruit_pix[13] = XCreatePixmapFromBitmapData(display, root, fkey_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);

   fval_pix[0] = XCreatePixmapFromBitmapData(display, root, ff1_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[1] = XCreatePixmapFromBitmapData(display, root, ff2_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[2] = XCreatePixmapFromBitmapData(display, root, ff3_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[3] = fval_pix[2];
   fval_pix[4] = XCreatePixmapFromBitmapData(display, root, ff4_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[5] = XCreatePixmapFromBitmapData(display, root, ff5_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[6] = fval_pix[5];
   fval_pix[7] = XCreatePixmapFromBitmapData(display, root, ff6_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[8] = fval_pix[7];
   fval_pix[9] = XCreatePixmapFromBitmapData(display, root, ff7_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[10] = fval_pix[9];
   fval_pix[11] = XCreatePixmapFromBitmapData(display, root, ff8_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);
   fval_pix[12] = fval_pix[11];
   fval_pix[13] = XCreatePixmapFromBitmapData(display, root, ff9_bits,
      FRUIT_WIDTH, FRUIT_HEIGHT, 1, 0, 1);

   eat_pix[0] = XCreatePixmapFromBitmapData(display, root, fg1_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   eat_pix[1] = XCreatePixmapFromBitmapData(display, root, fg2_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   eat_pix[2] = XCreatePixmapFromBitmapData(display, root, fg3_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
   eat_pix[3] = XCreatePixmapFromBitmapData(display, root, fg4_bits,
      GHOST_SIZE, GHOST_SIZE, 1, 0, 1);
}


void create_GCs()
{
   XGCValues            gcv;
   unsigned long        mask;

   mask = GCForeground | GCBackground | GCFunction |
          GCFont | GCGraphicsExposures;

   /* resolve colors */
   if ((black | white) == black)
      normal = True;
   else if ((black | white) == white)
      normal = False;
   else {
      black &= white;
      normal = False;
   }

   /* context for copying; used for animation */
   gcv.foreground         = black;
   gcv.background         = white;
   gcv.function           = GXcopy;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   copyGC                 = XCreateGC(display, root, mask, &gcv);

   /* context for copying; used for full size areas */
   gcv.foreground         = black;
   gcv.background         = white;
   gcv.function           = GXcopy;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   gcv.stipple            = demo_back;
   fullcopyGC             = XCreateGC(display, root, mask | GCStipple, &gcv);

   /* context for overlaying; used for animation */
   gcv.foreground         = black;
   gcv.background         = white;
   gcv.function           = normal ? GXor : GXand;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   gcv.stipple            = demo_gray;
   orGC                   = XCreateGC(display, root, mask | GCStipple, &gcv);

   /* context for clearing */
   gcv.foreground         = white;
   gcv.background         = black;
   gcv.function           = GXcopy;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   clearGC                = XCreateGC(display, root, mask, &gcv);

   /* context for inverting */
   gcv.foreground         = white;
   gcv.background         = black;
   gcv.function           = GXinvert;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   gcv.plane_mask         = black ^ white;
   invertGC               = XCreateGC(display, root, mask | GCPlaneMask, &gcv);

   /* context for flashing the power dots */
   gcv.foreground         = black;
   gcv.background         = white;
   gcv.function           = normal ? GXxor : GXequiv;
   gcv.plane_mask         = black ^ white;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   powerGC                = XCreateGC(display, root, mask | GCPlaneMask, &gcv);

   /* context of depth 1; used for transferring onto bitmaps */
   gcv.foreground         = 1;
   gcv.background         = 0;
   gcv.function           = GXcopy;
   gcv.font               = font;
   gcv.graphics_exposures = False;
   bitmapGC               = XCreateGC(display, bghost[0], mask, &gcv);
}


void create_demo_images()
{
   demo_mask[0] = XCreatePixmapFromBitmapData(display, root, bigc_bits,
      48, 48, 1, 0, 1);
   demo_mask[1] = XCreatePixmapFromBitmapData(display, root, bigh_bits,
      48, 48, 1, 0, 1);
   demo_mask[2] = XCreatePixmapFromBitmapData(display, root, bigo_bits,
      48, 48, 1, 0, 1);
   demo_mask[3] = XCreatePixmapFromBitmapData(display, root, bigm_bits,
      48, 48, 1, 0, 1);
   demo_mask[4] = XCreatePixmapFromBitmapData(display, root, bigp_bits,
      48, 48, 1, 0, 1);

   demo_map[0] = XCreatePixmapFromBitmapData(display, root, bigcl_bits,
      48, 48, 1, 0, 1);
   demo_map[1] = XCreatePixmapFromBitmapData(display, root, bighl_bits,
      48, 48, 1, 0, 1);
   demo_map[2] = XCreatePixmapFromBitmapData(display, root, bigol_bits,
      48, 48, 1, 0, 1);
   demo_map[3] = XCreatePixmapFromBitmapData(display, root, bigml_bits,
      48, 48, 1, 0, 1);
   demo_map[4] = XCreatePixmapFromBitmapData(display, root, bigpl_bits,
      48, 48, 1, 0, 1);

   demo_back = XCreatePixmapFromBitmapData(display, root, backpix_bits,
      32, 32, 1, 0, 1);
   demo_gray = XCreatePixmapFromBitmapData(display, root, graypix_bits,
      16, 16, 1, 0, 1);
   credit = XCreatePixmapFromBitmapData(display, root, credit_bits,
      CREDIT_WIDTH, CREDIT_HEIGHT, 1, 0, 1);
   demobox = XCreatePixmapFromBitmapData(display, root, demobox_bits,
      DEMOBOX_WIDTH, DEMOBOX_HEIGHT, 1, 0, 1);
   title = XCreatePixmapFromBitmapData(display, root, title_bits,
      TITLE_WIDTH, TITLE_HEIGHT, 1, 0, 1);
   pause = XCreatePixmapFromBitmapData(display, root, pause_bits,
      TITLE_WIDTH, TITLE_HEIGHT, 1, 0, 1);
}


void clear_maps()
{
   XFillRectangle(display, powermap, clearGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
   XFillRectangle(display, save, clearGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
   XFillRectangle(display, map, clearGC, 0, 0, WIN_WIDTH, WIN_HEIGHT);
}


void create_regions()
{
   XRectangle  full_rect;
   int         i;

   fruit_region = XCreateRegion();
   power_region = XCreateRegion();
   full_region  = XCreateRegion();
   pac_region   = XCreateRegion();
   for (i = 0; i < MAX_GHOSTS; i++)
      ghost_region[i] = XCreateRegion();

   region[0]    = XCreateRegion();
   region[1]    = XCreateRegion();

   full_rect.x = 0;
   full_rect.y = 0;
   full_rect.width = WIN_WIDTH;
   full_rect.height = WIN_HEIGHT;
   XUnionRectWithRegion(&full_rect, full_region, full_region);
}


void destroy_regions()
{
   int i;

   XDestroyRegion(fruit_region);
   XDestroyRegion(power_region);
   XDestroyRegion(full_region);
   XDestroyRegion(pac_region);
   for (i = 0; i < MAX_GHOSTS; i++)
      XDestroyRegion(ghost_region[i]);

   XDestroyRegion(region[0]);
   XDestroyRegion(region[1]);
}
