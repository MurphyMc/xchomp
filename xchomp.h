
#ifndef EXTERN
#define EXTERN extern
#endif

#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifndef NULL
#define NULL  0L
#endif

#define sgn(x)    ((x) ? (((x) > 0) ? 1 : -1) : 0)
#define abs(x)    (((x) < 0) ? -(x) : (x))


/*-- MACHINE DEPENDENCIES ----------------------------------------------------*/

#ifdef VMS
EXTERN float      vms_delay;
#define random()  (rand() >> 16)
#define usleep(x) { vms_delay = (x) * 0.000001; lib$wait(&vms_delay); }
#endif

#ifdef ULTRIX
#include <sys/time.h>
EXTERN struct timeval  st_delay;
#define usleep(x)  { st_delay.tv_usec = (x); st_delay.tv_sec = 0; \
                     select(32, NULL, NULL, NULL, &st_delay); }
#endif

#ifdef stellar
#include <sys/time.h>
EXTERN struct timeval  st_delay;
#define usleep(x)  { st_delay.tv_usec = (x); st_delay.tv_sec = 0; \
                     select(32, NULL, NULL, NULL, &st_delay); }
#endif

/*----------------------------------------------------------------------------*/


#define GHOST_SIZE     16

#define BLOCK_WIDTH    21
#define BLOCK_HEIGHT   16

#define WIN_WIDTH      GHOST_SIZE * BLOCK_WIDTH
#define WIN_HEIGHT     GHOST_SIZE * BLOCK_HEIGHT

#define FRUIT_WIDTH    20
#define FRUIT_HEIGHT   16

#define ICON_WIDTH     32
#define ICON_HEIGHT    32

#define MAX_GHOSTS     4
#define MAX_POWER_DOTS 4

#define CREDIT_WIDTH   195
#define CREDIT_HEIGHT  22

#define DEMOBOX_WIDTH  81
#define DEMOBOX_HEIGHT 49

#define TITLE_WIDTH    80
#define TITLE_HEIGHT   16

typedef int     intm[8];
typedef char    charm[BLOCK_WIDTH];
typedef charm   mazedata[BLOCK_HEIGHT];
typedef void    (*funcptr)();

EXTERN Atom  DEC_icon_atom;
EXTERN Atom  wm_delete_window_atom;

/* Xlib parameters */
EXTERN Display * display;
EXTERN Window    root, window;
EXTERN int       screen, depth, black, white;
EXTERN Bool      normal;
EXTERN Font      font;
EXTERN int       ascent, descent;

/* graphics contexts */
EXTERN GC  copyGC, orGC, clearGC, invertGC;
EXTERN GC  powerGC, fullcopyGC, bitmapGC;

/* bitmaps */
EXTERN Pixmap    icon, map, save, powermap;
EXTERN Pixmap    demo_map[5], demo_mask[5], demo_back, demo_gray;
EXTERN Pixmap    credit, demobox, title, pause;
EXTERN Pixmap    bghost[16], eghost[16], fghost[16], gghost[16];
EXTERN Pixmap    lpac[16], rpac[16], upac[16], dpac[16];
EXTERN Pixmap    *pac, small_pac, maze[128];
EXTERN Pixmap    dead_prot[11], deadpac[11];
EXTERN Pixmap    eat_pix[4], fval_pix[14], fruit_pix[14];

/* regions */
EXTERN Region    fruit_region, power_region, full_region;
EXTERN Region    ghost_region[MAX_GHOSTS], pac_region, region[2];

/* position and direction variables */
EXTERN int    ghost_x[MAX_GHOSTS], ghost_y[MAX_GHOSTS];
EXTERN int    ghost_ix[MAX_GHOSTS], ghost_iy[MAX_GHOSTS];
EXTERN int    ghost_sx[MAX_GHOSTS], ghost_sy[MAX_GHOSTS];
EXTERN int    pac_x, pac_y, pac_ix, pac_iy, pac_sx, pac_sy;

/* additional ghost parameters */
EXTERN funcptr    drive[MAX_GHOSTS];
EXTERN funcptr    contact[MAX_GHOSTS];
EXTERN Pixmap    *ghost[MAX_GHOSTS];
EXTERN int        loops[MAX_GHOSTS];

/* fruit parameters */
EXTERN int    fruit_count, fruit_times, fruit_x, fruit_y;
EXTERN Bool   fruit_shown;

/* game parameters */
EXTERN int    level, plevel, lives;
EXTERN long   score, high_score;

/* miscellaneous variables */
EXTERN int      grey_tick, flash_tick, off_tick;
EXTERN int      count, count_sync, cr;
EXTERN Bool     eat_mode;
EXTERN int      door_x, door_y, eat_index;
EXTERN int      numdots, powerdots, num_ghosts;
EXTERN mazedata md, dd;
EXTERN KeySym   last_key;
EXTERN Bool     dead, completed;

EXTERN jmp_buf  jb_start;

EXTERN void     follow(int);
EXTERN void     run(int);
EXTERN void     go_home(int);
EXTERN void     hover(int);
EXTERN void     hover2(int);
EXTERN void     die(int);
EXTERN void     eat(int);
EXTERN void     noop(int);
EXTERN Bool     pause_seq();

void do_exit();
void play_game(int);
void destroy_regions();
void create_ghost();
void create_pac();
void create_fruit();
void create_maze_symbols();
void create_demo_images();
void create_GCs();
void create_window(int, char **);
void create_maps();
void create_regions();
void demo_seq();
void display_title();
void print_score(long);
void display_level(Bool);
void clear_maps();
void read_maze(int);
void position_players();
int set_lives(int);
void get_ready();
void restore_status();
void control_pac();
void usleep(int);
void do_sleep(int);
void do_usleep(int);
void game_over();
void finish();
void control_pac();
void check_dots();
void check_normal_events();
