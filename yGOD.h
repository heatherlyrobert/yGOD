/*============================----beg-of-source---============================*/
/*===[[ HEADER ]]=============================================================*

 *   focus         : visualization
 *   niche         : opengl
 *   application   : yGOD
 *   purpose       : consistent, complete god-view services to opengl programs
 *
 *   base_system   : gnu/linux   (powerful, ubiquitous, technical, and hackable)
 *   lang_name     : ansi-c      (righteous, limitless, universal, and forever)
 *   dependencies  : x11,glx
 *   size goal     : small (less than 2,000 slocL)
 *
 *   created       : 2010-03        (after two specific implementations)
 *   author        : the_heatherlys
 *   priorities    : direct, simple, brief, vigorous, and lucid (h.w. fowler)
 *   end goal      : loosely coupled, strict interface, maintainable, portable
 *
 */
/*---[[ SUMMARY ]]------------------------------------------------------------*

 *   yGOD is a standard set of functions to implement consistent, powerful,
 *   reliable, testable, and maintainable god-view services to opengl
 *   applications.
 *
 */

/*---[[ PURPOSE ]]------------------------------------------------------------*

 *   >> problem (pr)
 *
 *      visualization environments are quite difficult to control and
 *      navigate as the interactions are complex.  adding to that challenge
 *      is that each programmer has a "clever" way of communicating so the
 *      various interfaces make learning diffucult and make it clumsy to switch
 *      applications.
 *
 *   >> objectives (o)
 *      0. linux and c api
 *      1. single, consistent interface for 3d interactions and feedback
 *      2. control all six degrees of freedom seamlessly (xyz, ypr)
 *      3. keyboard/command driven (no freaking mouse or tablet)
 *      4. feedback system is visual and text is optional
 *      5. does not require special opengl programming or new routines
 *      6. smaller plug in library format that does not disrupt programs
 *      7. free of commercial and support entanglements
 *      8. allows us to hack and learn
 *      9. maintainable
 *
 *   >> alternatives (a)
 *      a) professional library
 *      b) game harness (ogre or others)
 *      c) heatherly shared library
 *      d) custom for each program
 *
 *   >> consequences (c)
 *
 *      a) professional library
 *         0. not likely given linux
 *         1. almost certainly
 *         2. almost certainly
 *         3. not likely (most will be mousy)
 *         4. almost certainly
 *         5. NO
 *         6. NO WAY (big will be the rule)
 *         7. NO
 *         8. NO
 *         9. NO
 *         *. DOA
 *
 *      b) game harness (ogre or others)
 *         0. several
 *         1. almost certainly
 *         2. almost certainly
 *         3. not likely (most will be mousy)
 *         4. almost certainly
 *         5. not likely
 *         6. NO
 *         7. NO
 *         8. NO
 *         9. NO
 *         *. DOA
 *
 *      c) heatherly shared library
 *         0. yes (as its custom)
 *         1. yes
 *         2. yes
 *         3. yes
 *         4. yes
 *         5. yes
 *         6. yes
 *         7. yes
 *         8. yes
 *         9. mostly ;)
 *         *. this is our sweet spot
 *
 *      d) custom for each program
 *         0. yes (as its custom)
 *         1. yes
 *         2. yes
 *         3. yes
 *         4. yes
 *         5. yes
 *         6. yes
 *         7. yes
 *         8. yes
 *         9. no
 *         *. tried it, lots of one-off debugging
 *
 *
 *
 *   yGOD is a set of functions that provide consistent, powerful, reliable,
 *   testable, and maintainable god-view services to opengl applications.  this
 *   capability is difficlt to get right given all the transformation and
 *   feedback required to get it correct, and then to maintain and use multiple
 *   versions is even worse.  this library is meant to take a necessary,
 *   a painful, tedious, haphazard, and error prone process and turn it a easy,
 *   flexible, predictable, rapid, and automated set of functions.  better yet
 *   it will provide a single, consistent user interface to increase its
 *   efficiency and effectiveness.
 *
 *
 *   there are many existing functions, libraries and utilities have been built
 *   by better programmers and are likely superior in speed, size, capability,
 *   and reliability; BUT, i would not have learned nearly as much using them,
 *   so i follow the adage..
 *
 *   TO TRULY LEARN> do not seek to follow in the footsteps of the men of old;
 *   seek what they sought ~ Matsuo Basho
 *
 */
/*---[[ END HDOC ]]-----------------------------------------------------------*/


/*===[[ HEADER GUARD ]]=======================================================*/
#ifndef  YGOD
#define  YGOD loaded

#define  YGOD_VER_NUM       "3.2l"
#define  YGOD_VER_TXT       "darken axis colors to make markers standout more"



/*---(depricated)------------------------------------------*/
char      draw_godview            (void);



/*---(current)---------------------------------------------*/
char     *yGOD_version           (void);

char      yGOD_view              (void);
char      yGOD_orient            (void);
char      yGOD_orient_xzy        (float, float, float);
char      yGOD_locate            (void);
char      yGOD_locate_xzy        (float, float, float);
char      yGOD_grid              (void);

char      yGOD_start             (void);
char      yGOD_end               (void);
char      yGOD_axis              (char a_txt, double a_min, double a_max, double a_start, double a_step, double a_minor, double a_major);
char      yGOD_key               (char a_key);





#endif
