#include "yGOD.h"

/*===[[ XLIB HEADERS ]]=======================================================*/
#include <X11/X.h>                   /* main header                           */
#include <X11/Xlib.h>                /* c-api (xlib) header                   */
#include <X11/Xutil.h>
#include <X11/Xatom.h>               /* for setting up new protocol messages  */
#include <X11/keysym.h>              /* for resolving keycodes/keysyms        */


/*===[[ GLX HEADERS ]]========================================================*/
#include <GL/gl.h>                   /* main header for opengl                */
#include <GL/glx.h>                  /* for xlib/opengl integration           */

/*===[[ CLIB HEADERS ]]=======================================================*/
#include <stdio.h>

#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>


#include <yFONT.h>


const float  DEG2RAD = 3.1415927 / 180.0;
const float  RAD2DEG = 180.0 / 3.1415927;


float     my_x;
float     my_y;
float     my_z;
float     my_vx;
float     my_vy;
float     my_vz;
float     my_roll;
float     my_yaw;
float     my_pitch;
float     my_vroll;
float     my_vyaw;
float     my_vpitch;

GLuint    dl_godball;
GLuint    dl_godcross;
GLuint    dl_godgrid;
/*> GLuint    dl_hammer;                                                              <*/

GLuint    dl_target;
GLuint    dl_cube;
GLuint    dl_pyramid;
GLuint    dl_sphere;

GLuint    dl_marker;


static    GLuint    yGOD__godball      (float a_side, char a_ticks);
static    GLuint    yGOD__godcross     (float a_side, char a_ticks);
static    GLuint    yGOD__godgrid      (void);
static    GLuint    yGOD__hammer       (float a_side);

static    GLuint    yGOD__cube         (float a_side, char a_solid);
static    GLuint    yGOD__sbox         (float a_side);
static    GLuint    yGOD__wbox         (float a_side);
static    GLuint    yGOD__pyramid      (float a_side, char a_solid);
static    GLuint    yGOD__sphere       (float a_side, char a_solid);

static    char      font_load          (void);
static    char      font_delete        (void);

char   face_sm [30]  = "courier";
int    txf_sm;



enum {
   GOD_VANTAGE , GOD_SPIN    , GOD_ORBIT   ,
   GOD_CRAB    , GOD_BOOM    , GOD_DOLLY   ,
   GOD_LEFT    , GOD_ELEVATE , GOD_APPROACH,
   GOD_PITCH   , GOD_ROLL    , GOD_YAW     ,
};

struct tAXIS {
   /*---(user set)-----------------------*/
   char        txt;               /* label for axis                           */
   double      min;               /* minimum value                            */
   double      max;               /* maximum value                            */
   double      start;             /* start/default value                      */
   double      curr;              /* current setting                          */
   double      step;              /* smallist increment                       */
   double      minor;             /* middle step multiplier                   */
   double      major;             /* major step multiplier                    */
   /*---(user set)-----------------------*/
   double      range;             /* full range (max - min)                   */
   double      half;              /* halfway point in range (center)          */
   double      scale;             /* unit for 100 point scale                 */
   char        wrap;              /* label for axis                           */
   /*---(done)---------------------------*/
} axis[15] = {
   /* txt, ---min, ---max, -start, --curr, --step, -range, --half, -scale, wr */
   /* world rotations                                                         */
   {  'v',      0,    360,      0,      0,      5,      5,     45,    360,    180,    3.6, 'y'},
   {  's',      0,    360,      0,      0,      5,      5,     45,    360,    180,    3.6, 'y'},
   {  'o',      0,    360,      0,      0,      5,      5,     45,    360,    180,    3.6, 'y'},
   /* world translations                                                      */
   {  'c',   -800,    800,      0,      0,     20,      5,     45,   1600,    800,     16, 'n'},
   {  'b',   -800,    800,      0,      0,     20,      5,     45,   1600,    800,     16, 'n'},
   {  'd',    100,   1700,    800,    800,     20,      5,     45,   1600,    800,     16, 'n'},
   /* end-of-table                                                            */
   {  '-',      0,      0,      0,      0,      0,      0,      0,      0, 'n'},
};

static char      yGOD_ver [200] = "";

char*      /* ---- : return library versioning information -------------------*/
yGOD_version       (void)
{
   char    t [20] = "";
#if    __TINYC__ > 0
   strncpy (t, "[tcc built]", 15);
#elif  __GNUC__  > 0
   strncpy (t, "[gnu gcc  ]", 15);
#else
   strncpy (t, "[unknown  ]", 15);
#endif
   snprintf (yGOD_ver, 100, "%s   %s : %s", t, YGOD_VER_NUM, YGOD_VER_TXT);
   return yGOD_ver;
}

char
yGOD_start (void)
{
   dl_godball   = yGOD__godball  ( 10.0, '-');
   dl_godcross  = yGOD__godcross ( 10.0, 'y');
   dl_godgrid   = yGOD__godgrid ();
   /*> dl_hammer    = yGOD__hammer  ( 6.0);                                           <*/

   /*> dl_marker    = yGOD__wbox    (1.65);                                           <*/
   dl_marker    = yGOD__wbox    ( 50.0);
   dl_cube      = yGOD__cube    (1.0 , 'n');
   dl_pyramid   = yGOD__pyramid (1.00, 'n');
   dl_sphere    = yGOD__sphere  (2.25, 'n');
   font_load    ();
   return 0;
}


char
yGOD_end   (void)
{
   font_delete ();
   return 0;
}

char
yGOD_axis (char a_txt, double a_min, double a_max, double a_start, double a_step, double a_minor, double a_major)
{
   int  i;
   /*> printf ("\n");                                                                 <* 
    *> printf ("%c, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf\n",                <* 
    *>          a_txt, a_min, a_max, a_start, a_step, a_minor, a_major);              <*/
   for (i = 0; i < 6; ++i) {
      if (axis[i].txt != a_txt)   continue;
      axis[i].min   =  a_min;
      axis[i].max   =  a_max;
      axis[i].start =  a_start;
      axis[i].step  =  a_step;
      axis[i].minor =  a_minor;
      axis[i].major =  a_major;
      axis[i].range =  a_max - a_min;
      axis[i].half  =  axis[i].range / 2;
      axis[i].scale =  (float) (axis[i].range) / 100.0;
      axis[i].curr  =  a_start;
      /*> printf ("\na, ----min-, ----max-, --start-, ---step-, --minor-, --major-, --range-, ---half-, --scale-, w, ---curr-\n");   <* 
       *> printf ("%c, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %c, %8.2lf\n",                        <* 
       *>       a_txt, axis[i].min, axis[i].max, axis[i].start, axis[i].step, axis[i].minor, axis[i].major,                          <* 
       *>       axis[i].range, axis[i].half, axis[i].scale, axis[i].wrap, axis[i].curr);                                             <*/
      break;
   }
   return 0;
}

static char      /* [------] adjust the six degrees of freedom ---------------*/
yGOD__move         (int a_index, char a_dir)
{
   /*---(locals)-----------+-----------+-*/
   int         truncer     = 0;
   double      step        = 0.0;
   /*---(before)-------------------------*/
   /*> printf ("\naction %1d %c\n", a_index, a_dir);                                                                                       <* 
    *> printf ("   ------  a, ----min-, ----max-, --start-, ---step-, --minor-, --major-, --range-, ---half-, --scale-, w, ---curr-\n");   <* 
    *> printf ("   before  %c, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %c, %8.2lf\n",                      <* 
    *>       axis[a_index].txt  , axis[a_index].min  , axis[a_index].max  , axis[a_index].start,                                           <* 
    *>       axis[a_index].step , axis[a_index].minor, axis[a_index].major,                                                                <* 
    *>       axis[a_index].range, axis[a_index].half , axis[a_index].scale,                                                                <* 
    *>       axis[a_index].wrap , axis[a_index].curr);                                                                                     <*/
   /*---(adjust to direction)------------*/
   switch (a_dir) {
   case '0'  :  /* back to start/default */
      axis[a_index].curr   = axis[a_index].start;
      break;
   case '+'  : /* single step positive   */
      axis[a_index].curr  += axis[a_index].step;
      break;
   case '-'  : /* single step negative   */
      axis[a_index].curr  -= axis[a_index].step;
      break;
   case '>'  : /* minor stop positive    */
      step                = (axis[a_index].step * axis[a_index].minor);
      axis[a_index].curr += step;
      truncer             = (int)    (axis[a_index].curr / step);
      axis[a_index].curr  = (double) truncer * step;
      break;
   case '<'  : /* minor stop negative    */
      step                = (axis[a_index].step * axis[a_index].minor);
      axis[a_index].curr -= step;
      if (axis[a_index].curr < axis[a_index].min && axis[a_index].wrap == 'y')
         axis[a_index].curr += axis[a_index].max;
      truncer             = (int)    (axis[a_index].curr / step);
      axis[a_index].curr  = (double) truncer * step;
      break;
   case 'f'  : /* major stop positive    */
      step                = (axis[a_index].step * axis[a_index].major);
      axis[a_index].curr += step;
      truncer             = (int)    (axis[a_index].curr / step);
      axis[a_index].curr  = (double) truncer * step;
      break;
   case 'r'  : /* major stop negative    */
      step                = (axis[a_index].step * axis[a_index].major);
      axis[a_index].curr -= step;
      if (axis[a_index].curr < axis[a_index].min && axis[a_index].wrap == 'y')
         axis[a_index].curr += axis[a_index].max;
      truncer             = (int)    (axis[a_index].curr / step);
      axis[a_index].curr  = (double) truncer * step;
      break;
   }
   /*---(contain in legal bounds)--------*/
   if (axis[a_index].wrap == 'n') {
      if (axis[a_index].curr < axis[a_index].min) axis[a_index].curr  = axis[a_index].min;
      if (axis[a_index].curr > axis[a_index].max) axis[a_index].curr  = axis[a_index].max;
   } else {
      if (axis[a_index].curr < axis[a_index].min) axis[a_index].curr += axis[a_index].max;
      if (axis[a_index].curr > axis[a_index].max) axis[a_index].curr -= axis[a_index].max;
   }
   /*---(output)-------------------------*/
   /*> printf ("   after   %c, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %8.2lf, %c, %8.2lf\n",   <* 
    *>       axis[a_index].txt  , axis[a_index].min  , axis[a_index].max  , axis[a_index].start,                        <* 
    *>       axis[a_index].step , axis[a_index].minor, axis[a_index].major,                                             <* 
    *>       axis[a_index].range, axis[a_index].half , axis[a_index].scale,                                             <* 
    *>       axis[a_index].wrap , axis[a_index].curr);                                                                  <*/
   /*---(complete)-----------------------*/
   return 0;
}

char
yGOD_key (char a_key)
{
   /*> printf("handling a <<%c>>\n", a_key);                                          <*/
   /*---(x-axis linear)------------------*/
   switch (a_key) {
   case  12  :  yGOD__move (GOD_CRAB     , 'f');   break;
   case  'L' :  yGOD__move (GOD_CRAB     , '>');   break;
   case  'l' :  yGOD__move (GOD_CRAB     , '+');   break;
   case  'h' :  yGOD__move (GOD_CRAB     , '-');   break;
   case  'H' :  yGOD__move (GOD_CRAB     , '<');   break;
   case   8  :  yGOD__move (GOD_CRAB     , 'r');   break;
   }
   /*---(y-axis linear)------------------*/
   switch (a_key) {
   case  11  :  yGOD__move (GOD_BOOM     , 'f');   break;
   case  'K' :  yGOD__move (GOD_BOOM     , '>');   break;
   case  'k' :  yGOD__move (GOD_BOOM     , '+');   break;
   case  'j' :  yGOD__move (GOD_BOOM     , '-');   break;
   case  'J' :  yGOD__move (GOD_BOOM     , '<');   break;
   case  10  :  yGOD__move (GOD_BOOM     , 'r');   break;
   }
   /*---(z-axis linear)------------------*/
   switch (a_key) {
   case  15  :  yGOD__move (GOD_DOLLY    , 'f');   break;   /* <Ctrl-o>     */
   case  'O' :  yGOD__move (GOD_DOLLY    , '>');   break;
   case  'o' :  yGOD__move (GOD_DOLLY    , '+');   break;
   case  'i' :  yGOD__move (GOD_DOLLY    , '-');   break;
   case  'I' :  yGOD__move (GOD_DOLLY    , '<');   break;
   case   9  :  yGOD__move (GOD_DOLLY    , 'r');   break;   /* <Ctrl-i>     */
   }
   /*---(x-axis pitch)-------------------*/
   switch (a_key) {
   case   1  :  yGOD__move (GOD_VANTAGE  , 'f');   break;   /* <Ctrl-a>     */
   case  'V' :  yGOD__move (GOD_VANTAGE  , '>');   break;
   case  'v' :  yGOD__move (GOD_VANTAGE  , '+');   break;
   case  'p' :  yGOD__move (GOD_VANTAGE  , '-');   break;
   case  'P' :  yGOD__move (GOD_VANTAGE  , '<');   break;
   case  16  :  yGOD__move (GOD_VANTAGE  , 'r');   break;   /* <Ctrl-p>     */
   }
   /*---(y-axis yaw)---------------------*/
   switch (a_key) {
   case  25  :  yGOD__move (GOD_ORBIT    , 'r');   break;   /* <Ctrl-y>     */
   case  'Y' :  yGOD__move (GOD_ORBIT    , '<');   break;
   case  'y' :  yGOD__move (GOD_ORBIT    , '-');   break;
   case  't' :  yGOD__move (GOD_ORBIT    , '+');   break;
   case  'T' :  yGOD__move (GOD_ORBIT    , '>');   break;
   case  20  :  yGOD__move (GOD_ORBIT    , 'f');   break;   /* <Ctrl-t>     */
   }
   /*---(z-axis roll)--------------------*/
   switch (a_key) {
   case  18  :  yGOD__move (GOD_SPIN     , 'r');   break;   /* <Ctrl-r>     */
   case  'R' :  yGOD__move (GOD_SPIN     , '<');   break;
   case  'r' :  yGOD__move (GOD_SPIN     , '-');   break;
   case  'w' :  yGOD__move (GOD_SPIN     , '+');   break;
   case  'W' :  yGOD__move (GOD_SPIN     , '>');   break;
   case  23  :  yGOD__move (GOD_SPIN     , 'f');   break;   /* <Ctrl-w>     */

   }
   /*---(x-axis yaw)---------------------*/
   switch (a_key) {
   case  '0' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                yGOD__move (GOD_VANTAGE  , '0');
                yGOD__move (GOD_SPIN     , '0');
                yGOD__move (GOD_ORBIT    , '0');
                break;

   case  '1' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                axis[GOD_VANTAGE].curr =   0; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =   0; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '2' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                axis[GOD_VANTAGE].curr =   0; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =  90; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '3' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                axis[GOD_VANTAGE].curr =   0; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr = 180; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '4' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                axis[GOD_VANTAGE].curr =   0; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr = 270; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '5' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                axis[GOD_VANTAGE].curr =  90; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =   0; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '6' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '0');
                axis[GOD_VANTAGE].curr = 270; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =   0; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '7' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '5');
                axis[GOD_VANTAGE].curr =   0; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =  20; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '8' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                yGOD__move (GOD_DOLLY    , '5');
                axis[GOD_VANTAGE].curr =  50; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =  20; yGOD__move (GOD_ORBIT    , ' ');
                break;

   case  '9' :  yGOD__move (GOD_CRAB     , '0');
                yGOD__move (GOD_BOOM     , '0');
                axis[GOD_DOLLY  ].curr = 300; yGOD__move (GOD_DOLLY    , ' ');
                axis[GOD_VANTAGE].curr =  90; yGOD__move (GOD_VANTAGE  , ' ');
                axis[GOD_SPIN   ].curr =   0; yGOD__move (GOD_SPIN     , ' ');
                axis[GOD_ORBIT  ].curr =  00; yGOD__move (GOD_ORBIT    , ' ');
                break;
   }
   return 0;
}

char
yGOD_predraw (void)
{
   return 0;
}

char
yGOD_postdraw (void)
{
   return 0;
}


char
draw_godview (void)
{
   /*> printf("yGOD_view -- called depricated draw_godview\n");                      <*/
   yGOD_view();
   return 0;
}

char
yGOD_view (void)
{
   glTranslatef((float) axis[GOD_CRAB ].curr * -10, (float) axis[GOD_BOOM ].curr * -10, (float) axis[GOD_DOLLY].curr * -10);
   /*> glTranslatef((float) axis[GOD_CRAB ].curr, (float) axis[GOD_BOOM ].curr, (float) axis[GOD_DOLLY].curr);   <*/
   glRotatef   ((float) axis[GOD_SPIN    ].curr  , 0.0f, 0.0f, 1.0f);
   glRotatef   ((float) axis[GOD_VANTAGE ].curr  , 1.0f, 0.0f, 0.0f);
   glRotatef   ((float) axis[GOD_ORBIT   ].curr  , 0.0f, 1.0f, 0.0f);
   return 0;
}

char
yGOD_orient_xzy    (float a_x, float a_z, float a_y)
{
   /*---(locals)---------------------------*/
   /*> float    a = a_side / 2.0;                                                     <*/
   float    a = 25;
   float    b = a / 2.0;
   float    i, j, d;
   float    r, x, y, z;
   char      msg [200];
   /*---(begin)----------------------------*/
   glPushMatrix(); { 
      /*---(begin)----------------------------*/
      glLineWidth (2.0f);
      /*---(xz plane)-----------------------*/
      glColor4f   (0.0, 0.0, 1.0, 1.0);
      for (i = 0; i <=  4; ++i) {
         glBegin(GL_LINE_STRIP); {
            for (j = 15; j <= 75; j += 5) {
               d = (i * 90) + j;
               r = d * DEG2RAD;
               x = a * cos (r);
               z = a * sin (r);
               glVertex3f ( x,   0.00f, z);
            }
         } glEnd();
      }
      glPointSize (12.0f);
      glLineWidth (4.0f);
      glBegin     (GL_LINES); {
         glVertex3f(  0.0f,   0.0f,  0.0f);
         r = (-axis[GOD_ORBIT].curr + 90) * DEG2RAD;
         x = (a * 1.5) * cos(r);
         z = (a * 1.5) * sin(r);
         glVertex3f(  x,   0.0f,  z);
      } glEnd();
      glLineWidth (2.0f);
      glColor4f   (0.0, 0.0, 0.0, 1.0);
      /*> glPushMatrix(); {                                                           <* 
       *>    glTranslatef( -4.0f,  -7.0,   0.0);                                      <* 
       *>    snprintf (msg, 50, "%03d", axis[GOD_ORBIT].curr);                        <* 
       *>    yFONT_print (txf_sm,   2, YF_MIDRIG, msg);                               <* 
       *> } glPopMatrix();                                                            <*/
      glPointSize (6.0f);
      glBegin     (GL_POINTS); {
         for (d = 0; d <= 360; d +=  90) {
            r = d * DEG2RAD;
            x = a * cos(r);
            z = a * sin(r);
            glVertex3f( x,   0.00f, z);
         }
      } glEnd();
      /*---(xy plane)-----------------------*/
      glColor4f(1.0, 0.0, 0.0, 1.0);
      for (i = 0; i <=  4; ++i) {
         glBegin(GL_LINE_STRIP); {
            for (j = 15; j <= 75; j += 5) {
               d = (i * 90) + j;
               r = d * DEG2RAD;
               x = a * cos (r);
               y = a * sin (r);
               glVertex3f( x,  y,  0.00f);
            }
         } glEnd();
      }
      glPointSize (12.0f);
      glLineWidth (4.0f);
      glBegin     (GL_LINES); {
         glVertex3f(  0.0f,   0.0f,  0.0f);
         r = axis[GOD_SPIN ].curr * DEG2RAD;
         x = (a * 1.5) * cos(r);
         y = (a * 1.5) * sin(r);
         glVertex3f(     x,      y,  0.0f);
      } glEnd();
      glLineWidth (2.0f);
      glColor4f   (0.0, 0.0, 0.0, 1.0);
      glPointSize (5.0f);
      glBegin     (GL_POINTS); {
         for (d = 0; d <= 360; d +=  90) {
            r = d * DEG2RAD;
            x = a * cos(r);
            y = a * sin(r);
            glVertex3f( x,  y,  0.00f);
         }
      } glEnd();
      /*---(yz plane)-----------------------*/
      glColor4f(1.0, 1.0, 0.0, 1.0);
      for (i = 0; i <=  4; ++i) {
         glBegin(GL_LINE_STRIP); {
            for (j = 15; j <= 75; j += 5) {
               d = (i * 90) + j;
               r = d * DEG2RAD;
               y = a * cos (r);
               z = a * sin (r);
               glVertex3f( 0.00f,  y,  z);
            }
         } glEnd();
      }
      glPointSize (12.0f);
      glLineWidth (4.0f);
      glBegin     (GL_LINES); {
         glVertex3f (  0.0f,   0.0f,  0.0f);
         r = axis[GOD_VANTAGE ].curr * DEG2RAD;
         y = (a * 1.3) * cos(r);
         z = (a * 1.3) * sin(r);
         glVertex3f (  0.0f,     y,     z);
         glVertex3f (  0.0f, -1.0f,  0.0f);
         glVertex3f (  0.0f,  1.0f,  0.0f);
      } glEnd();
      glLineWidth (2.0f);
      glColor4f   (0.0, 0.0, 0.0, 1.0);
      glPointSize (5.0f);
      glBegin(GL_POINTS); {
         for (d = 0; d <= 360; d +=  90) {
            r = d * DEG2RAD;
            y = a * cos(r);
            z = a * sin(r);
            glVertex3f( 0.00f,  y,  z);
         }
      } glEnd();
      /*---(finish up)------------------------*/
   } glPopMatrix();
   /*---(complete)-------------------------*/
   return 0;
}

char
yGOD_orient_NEW    (void)
{
   /*---(locals)--------------------------------*/
   float       x_pos       = 0;
   char        x_msg       [200];
   glPushMatrix(); {
      /*---(get to the right place)----------------*/
      glBindTexture   (GL_TEXTURE_2D, 0);
      glPolygonMode   (GL_FRONT_AND_BACK, GL_FILL);
      /*---(draw god's balls)----------------------*/
      glColor3f   (0.0f, 0.5f, 0.0f);
      glLineWidth (0.8f);
      glCallList  (dl_godball);
      /*===[yaw]==================================*/
      x_pos = (axis[GOD_ORBIT ].curr - axis[GOD_ORBIT ].min);
      glColor3f    (   0.0f,   1.0f,   0.0f);
      /*---(marker)--------------*/
      glPushMatrix(); {
         glRotatef    (-90 + x_pos  , 0.0f, 1.0f, 0.0f);
         glTranslatef (   5.0f,   1.5f,   0.0f);
         glRotatef    (-180.0f,   0.0f,   0.0f,   1.0f);
         glCallList   (dl_pyramid);
      } glPopMatrix();
      /*---(label)---------------*/
      glPushMatrix(); {
         /*---(sign)-------------*/
         if (axis[GOD_ORBIT  ].curr >= 0) strcpy (x_msg, "+");
         else                             strcpy (x_msg, "-");
         glTranslatef ( -7.5f,   4.5f,   0.0f);
         yFONT_print  (txf_sm,   2, YF_MIDLEF, x_msg);
         /*---(value)------------*/
         snprintf     (x_msg, 50, "%03d", abs ((int) axis[GOD_ORBIT].curr));
         glTranslatef (  0.0f,   2.5f,   0.0f);
         yFONT_print  (txf_sm,   2, YF_MIDLEF, x_msg);
         /*---(done)-------------*/
      } glPopMatrix();
      /*===[pitch]================================*/
      x_pos = (axis[GOD_VANTAGE ].curr - axis[GOD_VANTAGE ].min);
      glColor3f    (   1.0f,   0.0f,   0.0f);
      /*---(marker)--------------*/
      glPushMatrix(); {
         glRotatef    (90 - x_pos, 1.0f, 0.0f, 0.0f);
         glTranslatef (   1.0f,   6.0f,   0.0f);
         glRotatef    (  90.0f,   0.0f,   0.0f,   1.0f);
         glCallList   (dl_pyramid);
      } glPopMatrix();
      /*---(label)---------------*/
      glPushMatrix(); {
         /*---(sign)-------------*/
         if (axis[GOD_VANTAGE].curr >= 0) strcpy (x_msg, "+");
         else                             strcpy (x_msg, "-");
         glTranslatef (  7.5f,  -4.5,   0.0);
         yFONT_print  (txf_sm,   2, YF_MIDRIG, x_msg);
         /*---(value)------------*/
         glTranslatef (  0.0f,  -2.5,   0.0);
         snprintf     (x_msg, 50, "%03d", abs((int) axis[GOD_VANTAGE].curr));
         yFONT_print  (txf_sm,   2, YF_MIDRIG, x_msg);
         /*---(done)-------------*/
      } glPopMatrix();
      /*===[roll]=================================*/
      x_pos = (axis[GOD_SPIN    ].curr - axis[GOD_SPIN    ].min);
      glColor3f    (   1.0f,   1.0f,   0.0f);
      /*---(marker)--------------*/
      glPushMatrix(); {
         glRotatef    (x_pos - 90.0, 0.0f, 0.0f, 1.0f);
         glTranslatef (   7.0f,   0.0f,   0.0f);
         glRotatef    (  90.0f,   0.0f,   0.0f,   1.0f);
         glCallList   (dl_pyramid);
      } glPopMatrix();
      /*---(label)---------------*/
      glPushMatrix(); {
         /*---(sign)-------------*/
         if (axis[GOD_SPIN   ].curr >= 0) strcpy (x_msg, "+");
         else                             strcpy (x_msg, "-");
         glTranslatef ( -7.5f,  -4.5,   0.0);
         yFONT_print  (txf_sm,   2, YF_MIDLEF, x_msg);
         /*---(value)------------*/
         glTranslatef (  0.0f,  -2.5,   0.0);
         snprintf     (x_msg, 50, "%03d", abs((int) axis[GOD_SPIN   ].curr));
         yFONT_print  (txf_sm,   2, YF_MIDLEF, x_msg);
         /*---(done)-------------*/
      } glPopMatrix();
   } glPopMatrix();
   return 0;
}

char
yGOD_orient_xzy_REAL (float a_x, float a_z, float a_y)
{
   /*---(locals)--------------------------------*/
   float    x_pos    = 0;
   char      msg [200];
   glPushMatrix(); {
      /*---(get to the right place)----------------*/
      /*> glTranslatef( 42.0f, -15.0f,  -100.0f);                                     <*/
      glTranslatef( a_x, a_y,  a_z);
      glBindTexture   (GL_TEXTURE_2D, 0);
      glPolygonMode   (GL_FRONT_AND_BACK, GL_FILL);
      /*---(draw background)-----------------------*/
      /*> glColor4f (0.00f, 0.05f, 0.00f, 1.00f);                                     <*/
      /*> glColor4f (1.00f, 0.05f, 0.00f, 1.00f);                                     <* 
       *> glBegin(GL_POLYGON); {                                                      <* 
       *>    glVertex3f(  15.00,   6.00, -10.00);                                     <* 
       *>    glVertex3f(  15.00, -14.00, -10.00);                                     <* 
       *>    glVertex3f(  -5.00, -14.00, -10.00);                                     <* 
       *>    glVertex3f(  -5.00,   6.00, -10.00);                                     <* 
       *> } glEnd();                                                                  <*/
      glLineWidth(2.0f);
      glColor4f  (0.00f, 0.25f, 0.00f, 0.50f);
      glBegin(GL_LINE_STRIP); {
         glVertex3f(  15.00,   6.00, -10.00);
         glVertex3f(  15.00, -14.00, -10.00);
         glVertex3f(  -5.00, -14.00, -10.00);
         glVertex3f(  -5.00,   6.00, -10.00);
         glVertex3f(  15.00,   6.00, -10.00);
      } glEnd();
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      /*---(draw god's balls)----------------------*/
      glColor3f(0.0f, 0.5f, 0.0f);
      glCallList (dl_marker);
      glLineWidth(0.8f);
      glCallList (dl_godball);
      /*> glCallList (dl_godball);                                                    <*/
      /*---(axis)-----------------------------------*/
      glLineWidth(0.8f);
      /*> glPointSize(5.0f);                                                          <*/
      /*---(orbit)---------------------------------*/
      x_pos = (axis[GOD_ORBIT ].curr - axis[GOD_ORBIT ].min);
      glColor3f(0.0f, 0.3f, 1.0f);
      glPushMatrix(); {
         glTranslatef( -4.0f,  -7.0,   0.0);
         snprintf (msg, 50, "%03d", axis[GOD_ORBIT].curr);
         yFONT_print (txf_sm,   2, YF_MIDRIG, msg);
      } glPopMatrix();
      glPushMatrix(); {
         glRotatef(-90.0, 0.0f, 1.0f, 0.0f);
         glRotatef(x_pos  , 0.0f, 1.0f, 0.0f);
         glTranslatef(  6.0,   0.0,    0.0);
         glCallList(dl_pyramid);
      } glPopMatrix();
      /*---(vantage)------------------------------*/
      x_pos = (axis[GOD_VANTAGE ].curr - axis[GOD_VANTAGE ].min);
      glColor3f(1.0f, 0.0f, 0.0f);
      glPushMatrix(); {
         glTranslatef(  3.0f,   7.0,   0.0);
         snprintf (msg, 50, "%03d", axis[GOD_VANTAGE].curr);
         yFONT_print (txf_sm,   2, YF_MIDLEF, msg);
      } glPopMatrix();
      glColor3f(0.7f, 0.0f, 0.0f);
      glPushMatrix(); {
         glRotatef( 90.0, 0.0f, 0.0f, 1.0f);
         glRotatef(-x_pos, 0.0f, 1.0f, 0.0f);
         glTranslatef(  6.0,   0.0,    0.0);
         glCallList(dl_marker);
      } glPopMatrix();
      /*---(spin)---------------------------------*/
      x_pos = (axis[GOD_SPIN    ].curr - axis[GOD_SPIN    ].min);
      glColor3f    ( 0.5f, 0.5f, 0.0f);
      glPushMatrix(); {
         glTranslatef(  3.0f,  -7.0,   0.0);
         snprintf (msg, 50, "%03d", axis[GOD_SPIN   ].curr);
         yFONT_print (txf_sm,   2, YF_MIDLEF, msg);
      } glPopMatrix();
      glPushMatrix(); {
         glRotatef    ( 90.0, 1.0f, 0.0f, 0.0f);
         glRotatef    (x_pos - 90.0, 0.0f, 1.0f, 0.0f);
         glTranslatef (  6.0,  0.0,  0.0);
         glCallList   (dl_sphere);
      } glPopMatrix();
   } glPopMatrix();
   return 0;
}

char
yGOD_orient(void)
{
   yGOD_orient_NEW  ();
   return 0;
}

char
yGOD_locate_xzy    (float a_x, float a_z, float a_y)
{
   /*---(locals)-------------------------*/
   float       a = 30;
   float       b = a / 2.0;
   float       c = b * 1.5;
   float       d = b * 0.7;
   float       x_pos    = 0;
   float    x_factor = 40.0 / 100.0;
   /*---(begin)--------------------------*/
   glLineWidth ( 3.0f);
   glPointSize ( 6.0f);
   /*---(x-axis)-------------------------*/
   glColor4f(0.0, 0.0, 1.0, 1.0);
   glBegin(GL_LINES); {
      glVertex3f (    -a,   0.0f,   0.0f);
      glVertex3f (     a,   0.0f,   0.0f);
   } glEnd();
   x_pos = -((axis[GOD_CRAB ].half - (axis[GOD_CRAB ].curr - axis[GOD_CRAB ].min)) / axis[GOD_BOOM ].scale * x_factor);
   glBegin(GL_LINES); {
      glVertex3f ( x_pos,     -d,   0.0f);
      glVertex3f ( x_pos,      d,   0.0f);
   } glEnd();
   glBegin(GL_POINTS); {
      glVertex3f (    -a,  0.0f,  0.0f);
      glVertex3f (    -b,  0.0f,  0.0f);
      glVertex3f (  0.0f,  0.0f,  0.0f);
      glVertex3f (     b,  0.0f,  0.0f);
      glVertex3f (     a,  0.0f,  0.0f);
   } glEnd();
   /*---(y-axis)-------------------------*/
   glColor4f(1.0, 0.0, 0.0, 1.0);
   glBegin(GL_LINES); {
      glVertex3f (  0.0f,    -a,  0.0f);
      glVertex3f (  0.0f,     a,  0.0f);
   } glEnd();
   x_pos = -((axis[GOD_BOOM ].half - (axis[GOD_BOOM ].curr - axis[GOD_BOOM ].min)) / axis[GOD_BOOM ].scale * x_factor);
   glBegin(GL_LINES); {
      glVertex3f (    -d,  x_pos,   0.0f);
      glVertex3f (     d,  x_pos,   0.0f);
   } glEnd();
   glBegin(GL_POINTS); {
      glVertex3f (  0.0f,    -a,  0.0f);
      glVertex3f (  0.0f,    -b,  0.0f);
      glVertex3f (  0.0f,  0.0f,  0.0f);
      glVertex3f (  0.0f,     b,  0.0f);
      glVertex3f (  0.0f,     a,  0.0f);
   } glEnd();
   /*---(z-axis)-------------------------*/
   glColor4f(1.0, 1.0, 0.0, 1.0);
   glBegin(GL_LINES); {
      glVertex3f (  0.0f,  0.0f,    -a);
      glVertex3f (  0.0f,  0.0f,     a);
   } glEnd();
   x_pos = -((axis[GOD_DOLLY].half - (axis[GOD_DOLLY].curr - axis[GOD_DOLLY].min)) / axis[GOD_DOLLY].scale * x_factor);
   glBegin(GL_LINES); {
      glVertex3f (  0.0f,     -d,  x_pos);
      glVertex3f (  0.0f,      d,  x_pos);
   } glEnd();
   glBegin(GL_POINTS); {
      glVertex3f (  0.0f,  0.0f,    -a);
      glVertex3f (  0.0f,  0.0f,    -b);
      glVertex3f (  0.0f,  0.0f,  0.0f);
      glVertex3f (  0.0f,  0.0f,     b);
      glVertex3f (  0.0f,  0.0f,     a);
   } glEnd();
   /*---(complete)-----------------------*/
   return 0;
}

char
yGOD_locate_NEW    (void)
{
   /*---(locals)--------------------------------*/
   float       x_pos       = 0;
   char        x_msg       [200];
   char        msg         [200];
   float       x_factor    = 14.0 / 100.0;
   glPushMatrix(); {
      glBindTexture   (GL_TEXTURE_2D, 0);
      glPolygonMode   (GL_FRONT_AND_BACK, GL_FILL);
      /*---(cross)----------------------------------*/
      glColor3f    (0.0f, 0.5f, 0.0f);
      glLineWidth  (0.8f);
      glCallList   (dl_godcross);
      /*===[x-axis]===============================*/
      glColor3f    (1.0f, 0.0f, 0.0f);
      /*---(marker)--------------*/
      glPushMatrix(); {
         x_pos = (axis[GOD_CRAB ].half - (axis[GOD_CRAB ].curr - axis[GOD_CRAB ].min)) / axis[GOD_BOOM ].scale * x_factor;
         glTranslatef (-x_pos,  -1.5,   0.0);
         glCallList   (dl_pyramid);
      } glPopMatrix();
      /*---(label)---------------*/
      glPushMatrix(); {
         /*---(sign)-------------*/
         if (axis[GOD_CRAB   ].curr >= 0) strcpy (x_msg, "+");
         else                             strcpy (x_msg, "-");
         glTranslatef ( -7.0f,  -4.5,   0.0);
         yFONT_print  (txf_sm,   2, YF_MIDLEF, x_msg);
         /*---(value)------------*/
         snprintf     (x_msg, 50, "%03d", abs ((int) axis[GOD_CRAB ].curr));
         glTranslatef (  0.0f,  -2.5,   0.0);
         yFONT_print  (txf_sm,   2, YF_MIDLEF, x_msg);
         /*---(done)-------------*/
      } glPopMatrix();
      /*===[y-axis]===============================*/
      glColor3f    (0.0f, 1.0f, 0.0f);
      /*---(marker)--------------*/
      glPushMatrix(); {
         x_pos = (axis[GOD_BOOM ].half - (axis[GOD_BOOM ].curr - axis[GOD_BOOM ].min)) / axis[GOD_BOOM ].scale * x_factor;
         glTranslatef (  -1.5, -x_pos + 1,   0.0);
         glRotatef    (-90.0, 0.0f, 0.0f, 1.0f);
         glCallList   (dl_pyramid);
      } glPopMatrix();
      /*---(label)---------------*/
      glPushMatrix(); {
         /*---(sign)-------------*/
         if (axis[GOD_BOOM   ].curr >= 0) strcpy (x_msg, "+");
         else                             strcpy (x_msg, "-");
         glTranslatef (  7.0f,   4.5,   0.0);
         yFONT_print  (txf_sm,   2, YF_MIDRIG, x_msg);
         /*---(value)------------*/
         glTranslatef (  0.0f,   2.5,   0.0);
         snprintf     (msg, 50, "%03d", abs ((int) axis[GOD_BOOM   ].curr));
         yFONT_print  (txf_sm,   2, YF_MIDRIG, msg);
         /*---(done)-------------*/
      } glPopMatrix();
      /*===[z-axis]===============================*/
      glColor3f    (1.0f, 1.0f, 0.0f);
      glPushMatrix(); {
         x_pos = (axis[GOD_DOLLY].half - (axis[GOD_DOLLY].curr - axis[GOD_DOLLY].min)) / axis[GOD_DOLLY].scale * x_factor;
         glTranslatef (   0.0,   1.5, -x_pos * 5.0);
         glRotatef    (180.0, 0.0f, 0.0f, 1.0f);
         glCallList   (dl_pyramid);
      } glPopMatrix();
      /*---(label)---------------*/
      glPushMatrix(); {
         /*---(sign)-------------*/
         if (axis[GOD_DOLLY  ].curr >= 0) strcpy (x_msg, "+");
         else                             strcpy (x_msg, "-");
         glTranslatef (  7.0f,  -4.5,   0.0);
         yFONT_print  (txf_sm,   2, YF_MIDRIG, x_msg);
         /*---(value)------------*/
         glTranslatef (  0.0f,  -2.5,   0.0);
         snprintf     (msg, 50, "%03d", abs ((int) axis[GOD_DOLLY  ].curr));
         yFONT_print  (txf_sm,   2, YF_MIDRIG, msg);
         /*---(done)-------------*/
      } glPopMatrix();
   } glPopMatrix();
   return 0;
}

char
yGOD_locate_xzy_REAL (float a_x, float a_z, float a_y)
{
   /*> return 0;                                                                      <*/
   float    x_pos    = 0;
   float    x_factor = 14.0 / 100.0;
   char      msg [100];
   /*> glTranslatef(-42.0f, -15.0f,  -100.0f);                                        <*/
   glPushMatrix(); {
      glTranslatef(a_x, a_y, a_z);
      glBindTexture   (GL_TEXTURE_2D, 0);
      glPolygonMode   (GL_FRONT_AND_BACK, GL_FILL);
      /*---(draw background)-----------------------*/
      glColor4f (0.00f, 0.05f, 0.00f, 1.00f);
      glBegin   (GL_POLYGON); {
         glVertex3f ( -15.00,   6.00, -10.00);
         glVertex3f ( -15.00, -14.00, -10.00);
         glVertex3f (   5.00, -14.00, -10.00);
         glVertex3f (   5.00,   6.00, -10.00);
      } glEnd();
      glLineWidth(2.0f);
      glColor4f  (0.00f, 0.25f, 0.00f, 0.50f);
      glBegin    (GL_LINE_STRIP); {
         glVertex3f ( -15.00,   6.00, -10.00);
         glVertex3f ( -15.00, -14.00, -10.00);
         glVertex3f (   5.00, -14.00, -10.00);
         glVertex3f (   5.00,   6.00, -10.00);
         glVertex3f ( -15.00,   6.00, -10.00);
      } glEnd();
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
      glLineWidth(0.8f);
      /*---(write the label)-----------------------*/
      glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
      /*---(cross)----------------------------------*/
      glColor3f    (0.0f, 0.5f, 0.0f);
      glCallList   (dl_godcross);
      glLineWidth  (0.8f);
      /*---(x location)-----------------------------*/
      glColor3f(0.0f, 0.3f, 1.0f);
      glPushMatrix(); {
         glTranslatef(  -9.0f,  -9.0,   0.0);
         snprintf (msg, 50, "%+4d", axis[GOD_CRAB ].curr - axis[GOD_CRAB ].start);
         /*> yFONT_print (txf_sm,   2, YF_BOTLEF, msg);                               <*/
      } glPopMatrix();
      glPushMatrix(); {
         x_pos = (axis[GOD_CRAB ].half - (axis[GOD_CRAB ].curr - axis[GOD_CRAB ].min)) / axis[GOD_BOOM ].scale * x_factor;
         glTranslatef(-x_pos,  -6.0,   0.0);
         glCallList(dl_pyramid);
      } glPopMatrix();
      /*---(y location)-----------------------------*/
      glColor3f(1.0f, 0.0f, 0.0f);
      glPushMatrix(); {
         glTranslatef(  4.0f,   7.0,   0.0);
         snprintf (msg, 50, "%+4d", axis[GOD_BOOM ].curr - axis[GOD_BOOM ].start);
         /*> yFONT_print (txf_sm,   2, YF_TOPRIG, msg);                               <*/
      } glPopMatrix();
      glColor3f(0.7f, 0.0f, 0.0f);
      glPushMatrix(); {
         x_pos = (axis[GOD_BOOM ].half - (axis[GOD_BOOM ].curr - axis[GOD_BOOM ].min)) / axis[GOD_BOOM ].scale * x_factor;
         glTranslatef(   5.0,-x_pos,   0.0);
         glCallList(dl_marker);
      } glPopMatrix();
      /*---(z location)-----------------------------*/
      glColor3f(0.5f, 0.5f, 0.0f);
      glPushMatrix(); {
         glTranslatef( -3.0f,   1.5,   0.0);
         snprintf (msg, 50, "%+4d", axis[GOD_DOLLY].curr - axis[GOD_DOLLY].start);
         /*> yFONT_print (txf_sm,   2, YF_TOPRIG, msg);                               <*/
      } glPopMatrix();
      glPushMatrix(); {
         x_pos = (axis[GOD_DOLLY].half - (axis[GOD_DOLLY].curr - axis[GOD_DOLLY].min)) / axis[GOD_DOLLY].scale * x_factor;
         glTranslatef(   0.0,   0.0,-x_pos);
         glCallList(dl_sphere);
      } glPopMatrix();
   } glPopMatrix();
   return 0;
}

char
yGOD_locate(void)
{
   yGOD_locate_NEW ();
   return 0;
}



static GLuint
yGOD__godball      (float a_side, char a_ticks)
{
   /*> printf ("yGOD_godball (a_side = %4.2f)\n", a_side);                            <*/
   /*---(locals)-----------+-----------+-*/
   float       a           = a_side / 2.0;
   float       b           = a      / 2.0;
   float       d;
   float       r, x, y, z;
   GLuint      displist;
   float       xz_min = -250, xz_max =   70;
   float       xy_min = -180, xy_max =  180;
   float       yz_min =  110, yz_max =  430;
   /*---(begin)--------------------------*/
   displist    = glGenLists(1);
   glNewList   (displist, GL_COMPILE); {
      /*---(general settings)---------------*/
      glLineWidth (2.0f);
      glPointSize (4.0f);
      /*---(xz plane)-----------------------*/
      glColor4f   (0.0, 0.7, 0.0, 1.0);
      glBegin     (GL_LINE_STRIP); {
         for (d = xz_min; d <= xz_max; d += 5) {
            r    = d * DEG2RAD;
            x   =  a * cos(r);
            z   =  a * sin(r);
            glVertex3f ( x,   0.00f, z);
         }
      } glEnd     ();
      if (a_ticks == 'y') {
         glBegin     (GL_POINTS); {
            for (d =    0; d <= 360; d +=  45) {
               r    = d * DEG2RAD;
               x   =  a * cos(r);
               z   =  a * sin(r);
               glVertex3f ( x,   0.00f, z);
            }
         } glEnd     ();
      }
      /*---(xy plane)-----------------------*/
      a += a_side / 15.0;
      glColor4f   (0.7, 0.7, 0.0, 1.0);
      glBegin     (GL_LINE_STRIP); {
         for (d = xy_min; d <= xy_max; d += 5) {
            r = d * DEG2RAD;
            x   =  a * cos(r);
            y   =  a * sin(r);
            glVertex3f ( x,  y,  0.00f);
         }
      } glEnd     ();
      if (a_ticks == 'y') {
         glBegin     (GL_POINTS); {
            for (d =   0; d <= 360; d += 45) {
               r = d * DEG2RAD;
               x   =  a * cos(r);
               y   =  a * sin(r);
               glVertex3f ( x,  y,  0.00f);
            }
         } glEnd     ();
      }
      /*---(yz plane)-----------------------*/
      a += a_side / 15.0;
      glColor4f   (0.7, 0.0, 0.0, 1.0);
      glBegin     (GL_LINE_STRIP); {
         for (d = yz_min; d <= yz_max; d += 5) {
            r = d * DEG2RAD;
            y   =  a * cos(r);
            z   =  a * sin(r);
            glVertex3f ( 0.00f,  y,  z);
         }
      } glEnd     ();
      if (a_ticks == 'y') {
         glBegin     (GL_POINTS); {
            for (d =   0; d <= 360; d += 45) {
               r = d * DEG2RAD;
               y   =  a * cos(r);
               z   =  a * sin(r);
               glVertex3f( 0.00f,  y,  z);
            }
         } glEnd     ();
      }
      /*---(end)-------------------------------*/
   } glEndList   ();
   /*---(complete)-------------------------*/
   return displist;
}



char
yGOD_grid (void)
{
   /*> glCallList(dl_godgrid);                                                        <*/
   /*---(locals)---------------------------*/
   int i;
   glDisable(GL_TEXTURE_2D);    /* NEW */
   glColor4f(1.0, 1.0, 0.0, 0.3);
   glBegin(GL_POLYGON); {
      glVertex3f(-1000.0, -1000.0, 0.0);
      glVertex3f(-1000.0,  1000.0, 0.0);
      glVertex3f( 1000.0,  1000.0, 0.0);
      glVertex3f( 1000.0, -1000.0, 0.0);
   } glEnd();
   glColor4f(0.0, 0.0, 1.0, 0.3);
   glBegin(GL_POLYGON); {
      glVertex3f(-1000.0, 0.0, -1000.0);
      glVertex3f(-1000.0, 0.0,  1000.0);
      glVertex3f( 1000.0, 0.0,  1000.0);
      glVertex3f( 1000.0, 0.0, -1000.0);
   } glEnd();
   glColor4f(1.0, 0.0, 0.0, 0.3);
   glBegin(GL_POLYGON); {
      glVertex3f( 0.0, -1000.0, -1000.0);
      glVertex3f( 0.0, -1000.0,  1000.0);
      glVertex3f( 0.0,  1000.0,  1000.0);
      glVertex3f( 0.0,  1000.0, -1000.0);
   } glEnd();
   /*---(settings)-------------------------*/
   /*---(xy)-------------------------------*/
   glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
   glLineWidth( 1.0f);
   glBegin(GL_LINES); {
      for (i = -1000; i <= 1000; i += 200) {
         glVertex3i( -1000, i, 0);
         glVertex3i(  1000, i, 0);
         glVertex3i( i, -1000, 0);
         glVertex3i( i,  1000, 0);
      }
   }; glEnd();
   /*---(xz)-------------------------------*/
   glColor4f(0.0f, 0.0f, 0.8f, 1.0f);
   glBegin(GL_LINES); {
      for (i = -1000; i <= 1000; i += 200) {
         glVertex3i( i,  0, -1000);
         glVertex3i( i,  0,  1000);
         glVertex3i( -1000, 0, i);
         glVertex3i(  1000, 0, i);
      }
   }; glEnd();
   /*---(yz)-------------------------------*/
   glColor4f(0.8f, 0.0f, 0.0f, 1.0f);
   glBegin(GL_LINES); {
      for (i = -1000; i <= 1000; i += 200) {
         glVertex3i( 0,  -1000, i);
         glVertex3i( 0,   1000, i);
         glVertex3i( 0, i, -1000);
         glVertex3i( 0, i,  1000);
      }
   }; glEnd();
   /*---(end)-------------------------------*/
   glPointSize(1.0f);
   glLineWidth(0.8f);
   return 0;
}



static GLuint
yGOD__godgrid ()
{
   /*---(locals)---------------------------*/
   int      i;
   /*---(begin)----------------------------*/
   GLuint displist = glGenLists(1);
   glNewList(displist, GL_COMPILE);
   /*---(settings)-------------------------*/
   glEnable(GL_LINE_STIPPLE);
   glLineStipple(1, 0xFFFF);
   /*---(xy)-------------------------------*/
   glColor4f(0.5f, 0.5f, 0.0f, 1.0f);
   glBegin(GL_LINES); {
      for (i = -1000; i <= 1000; i += 200) {
         glLineWidth( 1.0f);
         if (i == 0) glLineWidth(10.0f);
         glLineStipple(1, 0xF0F0);
         glVertex3i( -1000, i, 0);
         glVertex3i(     0, i, 0);
         glLineStipple(1, 0xFFFF);
         glVertex3i(     0, i, 0);
         glVertex3i(  1000, i, 0);
      }
      for (i = -1000; i <= 1000; i += 200) {
         if (i == 0) continue;
         if (i >  0) glLineStipple(1, 0xFFFF);
         else        glLineStipple(1, 0xF0F0);
         glVertex3i( i, -1000, 0);
         glVertex3i( i,  1000, 0);
      }
   }; glEnd();
   /*---(xz)-------------------------------*/
   glColor4f(0.0f, 0.0f, 0.8f, 1.0f);
   glBegin(GL_LINES); {
      for (i = -1000; i <= 1000; i += 200) {
         glLineWidth( 1.0f);
         if (i == 0) glLineWidth(10.0f);
         glLineStipple(1, 0xF0F0);
         glVertex3i( i,  0, -1000);
         glVertex3i( i,  0,     0);
         glLineStipple(1, 0xFFFF);
         glVertex3i( i,  0,     0);
         glVertex3i( i,  0,  1000);
      }
      for (i = -1000; i <= 1000; i += 200) {
         if (i == 0) continue;
         if (i >  0) glLineStipple(1, 0xFFFF);
         else        glLineStipple(1, 0xF0F0);
         glVertex3i( -1000, 0, i);
         glVertex3i(  1000, 0, i);
      }
   }; glEnd();
   /*---(yz)-------------------------------*/
   glColor4f(0.6f, 0.0f, 0.0f, 1.0f);
   glBegin(GL_LINES); {
      for (i = -1000; i <= 1000; i += 200) {
         glLineWidth( 1.0f);
         if (i == 0) glLineWidth(10.0f);
         glLineStipple(1, 0xF0F0);
         glVertex3i( 0,  -1000, i);
         glVertex3i( 0,      0, i);
         glLineStipple(1, 0xFFFF);
         glVertex3i( 0,      0, i);
         glVertex3i( 0,   1000, i);
      }
      for (i = -1000; i <= 1000; i += 200) {
         if (i == 0) continue;
         if (i >  0) glLineStipple(1, 0xFFFF);
         else        glLineStipple(1, 0xF0F0);
         glVertex3i( 0, i, -1000);
         glVertex3i( 0, i,  1000);
      }
   }; glEnd();
   /*> glBegin(GL_LINES); {                                                           <* 
    *>    /+---(xy)-----------------------------------------+/                        <* 
    *>    glColor4f(0.0f, 0.5f, 0.0f, 1.0f);                                          <* 
    *>    for (i = -1000; i <= 1000; i += 200) {                                      <* 
    *>       if (i == 0) continue;                                                    <* 
    *>       glVertex3i( -1000, i, 0);                                                <* 
    *>       glVertex3i(  1000, i, 0);                                                <* 
    *>    }                                                                           <* 
    *>    for (i = -1000; i <= 1000; i += 200) {                                      <* 
    *>       if (i == 0) continue;                                                    <* 
    *>       glVertex3i( i, -1000, 0);                                                <* 
    *>       glVertex3i( i,  1000, 0);                                                <* 
    *>    }                                                                           <* 
    *>    /+---(xz)-----------------------------------------+/                        <* 
    *>    glColor4f(0.0f, 0.0f, 0.8f, 1.0f);                                          <* 
    *>    for (i = -1000; i <= 1000; i += 200) {                                      <* 
    *>       if (i == 0) continue;                                                    <* 
    *>       glVertex3i( -1000, 0, i);                                                <* 
    *>       glVertex3i(  1000, 0, i);                                                <* 
    *>    }                                                                           <* 
    *>    for (i = -1000; i <= 1000; i += 200) {                                      <* 
    *>       if (i == 0) continue;                                                    <* 
    *>       glVertex3i( i, 0,  -1000);                                               <* 
    *>       glVertex3i( i, 0,   1000);                                               <* 
    *>    }                                                                           <* 
    *>    /+---(yz)-----------------------------------------+/                        <* 
    *>    glColor4f(0.4f, 0.0f, 0.0f, 1.0f);                                          <* 
    *>    for (i = -1000; i <= 1000; i += 200) {                                      <* 
    *>       if (i == 0) continue;                                                    <* 
    *>       glVertex3i( 0, -1000, i);                                                <* 
    *>       glVertex3i( 0,  1000, i);                                                <* 
    *>    }                                                                           <* 
    *>    for (i = -1000; i <= 1000; i += 200) {                                      <* 
    *>       if (i == 0) continue;                                                    <* 
    *>       glVertex3i( 0, i, -1000);                                                <* 
    *>       glVertex3i( 0, i,  1000);                                                <* 
    *>    }                                                                           <* 
    *> } glEnd();                                                                     <*/
   /*---(end)-------------------------------*/
   glPointSize(1.0f);
   glLineWidth(0.8f);
   glDisable(GL_LINE_STIPPLE);
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}



static GLuint
yGOD__godcross_OLD (float a_side)
{
   /*---(locals)---------------------------*/
   float    a = a_side;
   float    b = a / 2.0;
   float    c = b * 1.5;
   float    d = b * 0.7;
   /*---(begin)----------------------------*/
   GLuint displist = glGenLists(1);
   glNewList(displist, GL_COMPILE);
   glLineWidth ( 2.0f);
   glPointSize ( 6.0f);
   /*---(axis)-----------------------------*/
   glColor4f(0.0, 0.5, 1.0, 0.4);
   glBegin(GL_LINES); {
      glVertex3f(-a   , -c    ,   0.0f);
      glVertex3f( a   , -c    ,   0.0f);
      glVertex3f( 0.0f, -c + d,   0.0f);
      glVertex3f( 0.0f, -c - d,   0.0f);
      glVertex3f( 0.0f, -c    ,   d   );
      glVertex3f( 0.0f, -c    ,  -d   );
   } glEnd();
   glColor4f(1.0, 0.0, 0.0, 0.4);
   glBegin(GL_LINES); {
      glVertex3f( c    ,-a   ,   0.0f);
      glVertex3f( c    , a   ,   0.0f);
      glVertex3f( c + d, 0.0f,     0.0f);
      glVertex3f( c - d, 0.0f,     0.0f);
      glVertex3f( c   ,  0.0f ,   d   );
      glVertex3f( c   ,  0.0f ,  -d   );
   } glEnd();
   glColor4f(1.0, 1.0, 0.0, 0.4);
   glBegin(GL_LINES); {
      glVertex3f(  0.0f,   0.0f,  -a);
      glVertex3f(  0.0f,   0.0f,   a);
      glVertex3f(  d   ,   0.0f, 0.0f);
      glVertex3f( -d   ,   0.0f, 0.0f);
      glVertex3f(  0.0f,   -d  , 0.0f);
      glVertex3f(  0.0f,    d  , 0.0f);
   } glEnd();
   /*---(tickmarks)------------------------*/
   /*> glPointSize ( 8.0f);                                                           <*/
   glColor4f(0.0, 0.5, 1.0, 0.3);
   glBegin(GL_POINTS); {
      glVertex3f( -a,  -c   ,   0.0f);
      glVertex3f( -b,  -c   ,   0.0f);
      glVertex3f(0.0,  -c   ,   0.0f);
      glVertex3f(  b,  -c   ,   0.0f);
      glVertex3f(  a,  -c   ,   0.0f);
   } glEnd();
   glColor4f(1.0, 0.0, 0.0, 0.3);
   glBegin(GL_POINTS); {
      glVertex3f(  c   ,  -a   ,  0.0f);
      glVertex3f(  c   ,  -b   ,  0.0f);
      glVertex3f(  c   ,   0.0f,  0.0f);
      glVertex3f(  c   ,   b   ,  0.0f);
      glVertex3f(  c   ,   a   ,  0.0f);
   } glEnd();
   glColor4f(1.0, 1.0, 0.0, 0.3);
   glBegin(GL_POINTS); {
      glVertex3f(  0.0f,   0.0f,  -a);
      glVertex3f(  0.0f,   0.0f,  -b);
      glVertex3f(  0.0f,   0.0f, 0.0f);
      glVertex3f(  0.0f,   0.0f,   b);
      glVertex3f(  0.0f,   0.0f,   a);
   } glEnd();
   /*---(end)-------------------------------*/
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}

static GLuint
yGOD__godcross     (float a_side, char a_ticks)
{
   /*---(locals)---------------------------*/
   float    a = a_side;
   float    b = a / 1.7;
   float    c = b * 1.5;
   float    d = b * 0.7;
   float    e = a / 8.0;
   /*---(begin)----------------------------*/
   GLuint displist = glGenLists(1);
   glNewList(displist, GL_COMPILE); {
      glLineWidth ( 2.0f);
      glPointSize ( 6.0f);
      /*---(z-axis)-------------------------*/
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glColor4f(0.7, 0.7, 0.0, 1.0);
      /*---(baseline)----------*/
      glBegin(GL_LINES); {
         glVertex3f( 0.0f,   0.0f,    b * 1.5);
         glVertex3f( 0.0f,   0.0f,   -b * 1.5);
      } glEnd();
      /*---(end caps)----------*/
      if (a_ticks == 'y') {
         glBegin     (GL_POINTS); {
            glVertex3f( 0.0f,   0.0f,    b * 1.5);
            glVertex3f( 0.0f,   0.0f,   -b * 1.5);
         } glEnd     ();
      }
      /*---(xy axis)------------------------*/
      glColor4f(0.7, 0.0, 0.0, 1.0);
      /*---(baseline)----------*/
      glBegin(GL_LINES); {
         glVertex3f(    b,    0.0,  0.0);
         glVertex3f(   -b,    0.0,  0.0);
      } glEnd();
      /*---(end caps)----------*/
      if (a_ticks == 'y') {
         glBegin     (GL_POINTS); {
            glVertex3f(    b,    0.0,  0.0);
            glVertex3f(   -b,    0.0,  0.0);
         } glEnd     ();
      }
      /*---(yz axis)------------------------*/
      glColor4f(0.0, 0.7, 0.0, 1.0);
      /*---(baseline)----------*/
      glBegin(GL_LINES); {
         glVertex3f(  0.0,      b,  0.0);
         glVertex3f(  0.0,     -b,  0.0);
      } glEnd();
      /*---(end caps)----------*/
      if (a_ticks == 'y') {
         glBegin     (GL_POINTS); {
            glVertex3f(  0.0,      b,  0.0);
            glVertex3f(  0.0,     -b,  0.0);
         } glEnd     ();
      }
      /*---(end)-------------------------------*/
   } glEndList();
   /*---(complete)-------------------------*/
   return displist;
}

/*> GLuint                                                                             <* 
 *> yGOD__hammer (float a_side)                                                        <* 
 *> {                                                                                  <* 
 *>    /+---(locals)---------------------------+/                                      <* 
 *>    float    i = 0.00f;                                                             <* 
 *>    /+---(begin)----------------------------+/                                      <* 
 *>    GLuint  displist = glGenLists(1);                                               <* 
 *>    glNewList(displist, GL_COMPILE);                                                <* 
 *>    /+---(draw)-----------------------------+/                                      <* 
 *>    glBegin(GL_POINTS); {                                                           <* 
 *>       for (i = 0; i <= a_side + 0.5; i += 0.5)  glVertex3f(    i, 0.00f, 0.00f);   <* 
 *>    } glEnd();                                                                      <* 
 *>    /+---(end)-------------------------------+/                                     <* 
 *>    glEndList();                                                                    <* 
 *>    /+---(complete)-------------------------+/                                      <* 
 *>    return displist;                                                                <* 
 *> }                                                                                  <*/

static GLuint
yGOD__sbox (float a_side)
{
   /*---(locals)---------------------------*/
   float    a = a_side / 2;
   /*---(begin)----------------------------*/
   GLuint  displist = glGenLists(1);
   glNewList(displist, GL_COMPILE);
   /*---(state)----------------------------*/
   glDisable(GL_CULL_FACE);
   /*> glPolygonMode(GL_FRONT, GL_FILL);                                              <* 
    *> glPolygonMode(GL_BACK , GL_FILL);                                              <*/
   /*---(draw)-----------------------------*/
   glBegin(GL_POLYGON); {   /* top         */
      glVertex3f(  a,  a,  a);
      glVertex3f( -a,  a,  a);
      glVertex3f( -a,  a, -a);
      glVertex3f(  a,  a, -a);
   } glEnd();
   glBegin(GL_POLYGON); {   /* bottom      */
      glVertex3f(  a, -a,  a);
      glVertex3f( -a, -a,  a);
      glVertex3f( -a, -a, -a);
      glVertex3f(  a, -a, -a);
   } glEnd();
   glBegin(GL_POLYGON); {   /* left        */
      glVertex3f( -a,  a,  a);
      glVertex3f( -a,  a, -a);
      glVertex3f( -a, -a, -a);
      glVertex3f( -a, -a,  a);
   } glEnd();
   glBegin(GL_POLYGON); {   /* right       */
      glVertex3f(  a,  a,  a);
      glVertex3f(  a, -a,  a);
      glVertex3f(  a, -a, -a);
      glVertex3f(  a,  a, -a);
   } glEnd();
   glBegin(GL_POLYGON); {   /* front       */
      glVertex3f(  a,  a,  a);
      glVertex3f(  a, -a,  a);
      glVertex3f( -a, -a,  a);
      glVertex3f( -a,  a,  a);
   } glEnd();
   glBegin(GL_POLYGON); {   /* back        */
      glVertex3f(  a,  a, -a);
      glVertex3f( -a,  a, -a);
      glVertex3f( -a, -a, -a);
      glVertex3f(  a, -a, -a);
   } glEnd();
   /*---(end)------------------------------*/
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}

static GLuint
yGOD__wbox (float a_side)
{
   /*---(locals)---------------------------*/
   float    a = a_side / 2;
   /*---(begin)----------------------------*/
   GLuint  displist = glGenLists(1);
   glNewList(displist, GL_COMPILE);
   /*---(draw)-----------------------------*/
   glBegin(GL_LINE_LOOP); {
      glVertex3f(  a,  a,  a);
      glVertex3f(  a,  a, -a);
      glVertex3f( -a,  a, -a);
      glVertex3f( -a,  a,  a);
   } glEnd();
   glBegin(GL_LINE_LOOP); {
      glVertex3f(  a, -a,  a);
      glVertex3f(  a, -a, -a);
      glVertex3f( -a, -a, -a);
      glVertex3f( -a, -a,  a);
   } glEnd();
   glBegin(GL_LINE_LOOP); {
      glVertex3f( -a,  a,  a);
      glVertex3f( -a,  a, -a);
      glVertex3f( -a, -a, -a);
      glVertex3f( -a, -a,  a);
   } glEnd();
   glBegin(GL_LINE_LOOP); {
      glVertex3f(  a,  a,  a);
      glVertex3f(  a,  a, -a);
      glVertex3f(  a, -a, -a);
      glVertex3f(  a, -a,  a);
   } glEnd();
   /*---(end)------------------------------*/
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}

static GLuint
yGOD__cube(float a_side, char a_solid)
{
   /*---(locals)---------------------------*/
   float    a = a_side / 2;
   /*---(begin)----------------------------*/
   GLuint  displist = glGenLists(1);
   glNewList(displist, GL_COMPILE);
   /*---(draw)-----------------------------*/
   glEnable(GL_LINE_STIPPLE);
   glLineStipple(1, 0xAAAA);
   glBegin(GL_LINE_LOOP); {
      glVertex3f(  a,  a,  a);
      glVertex3f(  a,  a, -a);
      glVertex3f( -a,  a, -a);
      glVertex3f( -a,  a,  a);
   } glEnd();
   glBegin(GL_LINE_LOOP); {
      glVertex3f(  a, -a,  a);
      glVertex3f(  a, -a, -a);
      glVertex3f( -a, -a, -a);
      glVertex3f( -a, -a,  a);
   } glEnd();
   glBegin(GL_LINE_LOOP); {
      glVertex3f( -a,  a,  a);
      glVertex3f( -a,  a, -a);
      glVertex3f( -a, -a, -a);
      glVertex3f( -a, -a,  a);
   } glEnd();
   glBegin(GL_LINE_LOOP); {
      glVertex3f(  a,  a,  a);
      glVertex3f(  a,  a, -a);
      glVertex3f(  a, -a, -a);
      glVertex3f(  a, -a,  a);
   } glEnd();
   glDisable(GL_LINE_STIPPLE);
   /*---(end)------------------------------*/
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}

static GLuint
yGOD__pyramid(float a_side, char a_solid)
{
   /*---(locals)---------------------------*/
   float    a = a_side / 2;
   float    b = a_side * 1.3;
   /*---(begin)----------------------------*/
   GLuint   displist = glGenLists(1);
   glNewList(displist, GL_COMPILE);
   /*---(draw)--------------------------------------------*/
   glBegin(GL_QUADS);
   glVertex3f(   -a,   0.0,   -a);
   glVertex3f(   -a,   0.0,    a);
   glVertex3f(    a,   0.0,    a);
   glVertex3f(    a,   0.0,   -a);
   glEnd();
   glBegin(GL_LINES);
   glVertex3f(  0.0,     b,  0.0);
   glVertex3f(   -a,   0.0,   -a);
   glVertex3f(  0.0,     b,  0.0);
   glVertex3f(   -a,   0.0,    a);
   glVertex3f(  0.0,     b,  0.0);
   glVertex3f(    a,   0.0,    a);
   glVertex3f(  0.0,     b,  0.0);
   glVertex3f(    a,   0.0,   -a);
   glEnd();
   /*---(end)-------------------------------*/
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}

static GLuint
yGOD__sphere(float a_side, char a_solid)
{
   /*---(locals)---------------------------*/
   float    a = a_side / 2;
   int      d;
   float    r, x, y, z;
   /*---(begin)----------------------------*/
   GLuint displist = glGenLists(1);
   glNewList (displist, GL_COMPILE);
   /*---(draw)--------------------------------------------*/
   /*---(margins)---------------------------*/
   /*> glEnable(GL_LINE_STIPPLE);                                                     <* 
    *> glLineStipple(1, 0xAAAA);                                                      <*/
   glBegin(GL_LINE_STRIP);
   for (d = 0; d <= 360; d += 20) {
      r = d * DEG2RAD;
      x   =  a * cos(r);
      z   =  a * sin(r);
      glVertex3f( x,   0.00f, z);
   }
   glEnd();
   glBegin(GL_LINE_STRIP);
   for (d = 0; d <= 360; d += 20) {
      r = d * DEG2RAD;
      x   =  a * cos(r);
      y   =  a * sin(r);
      glVertex3f( x,  y,  0.00f);
   }
   glEnd();
   glBegin(GL_LINE_STRIP);
   for (d = 0; d <= 360; d += 20) {
      r = d * DEG2RAD;
      y   =  a * cos(r);
      z   =  a * sin(r);
      glVertex3f( 0.00f,  y,  z);
   }
   glEnd();
   /*> glDisable(GL_LINE_STIPPLE);                                                    <*/
   /*---(end)-------------------------------*/
   glEndList();
   /*---(complete)-------------------------*/
   return displist;
}




/*====================------------------------------------====================*/
/*===----                             fonts                            ----===*/
/*====================------------------------------------====================*/
static void      o___FONTS___________________o (void) {;}

static char
font_load          (void)
{
   txf_sm  = yFONT_load (face_sm);
   if (txf_sm <  0) {
      fprintf(stderr, "Problem loading %s\n", face_sm);
      exit(1);
   }
   return 0;
}

static char
font_delete        (void)
{
   yFONT_free (txf_sm);
   return 0;
}
