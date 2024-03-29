#include <X11/XF86keysym.h>

/* See LICENSE file for copyright and license details. */

/* appearance */
static unsigned int borderpx  = 1;        /* border pixel of windows */
static unsigned int snap      = 32;       /* snap pixel */ 
static int showbar            = 1;        /* 0 means no bar */
static int topbar             = 1;        /* 0 means bottom bar */
static const int usealtbar          = 1;        /* 1 means use non-dwm status bar */
static const char *altbarclass      = "Polybar"; /* Alternate bar class name */
static const char *alttrayname      = "tray";    /* Polybar tray instance name */
static const unsigned int gappih    = 10;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 10;       /* vert inner gap between windows */
static const unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 10;       /* vert outer gap between windows and screen edge */
static const int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static const char *fonts[]          = { "Cozette:size=12:autohint=true:antialias=false", "Caskaydia Cove NF:size=12:autohint=true:antialias=true", "monospace:size=12" };
static char normbgcolor[]           = "#222222";
static char normbordercolor[]       = "#444444";
static char normfgcolor[]           = "#bbbbbb";
static char selfgcolor[]            = "#eeeeee";
static char selbordercolor[]        = "#005577";
static char selbgcolor[]            = "#005577";
static char *colors[][3] = {
       /*               fg           bg           border   */
       [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
       [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

/* tagging 
 * 
 * for reference:
 * 1) Misc
 * 2) Misc
 * 3) Misc
 * 4) Music 
 * 5) Movies
 * 6) Mail
 * 7) Chat
 * 8) Web
 * 9) Game
 */
static const char *tags[] = { "", "", "", "", "", "", "", "", "" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "qutebrowser",  "qutebrowser",       NULL,       1 << 7,       0,           -1 },
	{ "discord",     "discord",       NULL,       1 << 6,            0,           -1 },
	{ "messagesforweb",     "messagesforweb",       NULL,       1 << 6,            0,           -1 },
	{ "whatsapp",     "whatsapp",       NULL,       1 << 6,            0,           -1 },
	{ "Thunderbird",     "Mail",       NULL,       1 << 5,            0,           -1 },
	{ "Steam",     "Steam",       "Steam",       1 << 8,            0,           -1 },
};

/* layout(s) */
static float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "",      tile },    /* first entry is default */
	{ "",      NULL },    /* no layout function means floating behavior */
	{ "",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define SUPERKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ SUPERKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ SUPERKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ SUPERKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ SUPERKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} }, \
	{ SUPERKEY|MODKEY,                KEY,      focusnthmon,    {.i = TAG} }, \
	{ SUPERKEY|MODKEY|ShiftMask,      KEY,      tagnthmon,      {.i = TAG} }, \

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *volupcmd[] = { "pamixer",  "-i", "5"};
static const char *voldowncmd[] = { "pamixer",  "-d", "5"};
static const char *mutecmd[] = { "pamixer",  "-t"};
static const char *runcmd[] = { "/home/endoman123/bin/launch-app",  NULL };
static const char *powercmd[] = { "/home/endoman123/bin/powermenu", NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *sscmd[] = { "flameshot", "full", "-c", NULL };
static const char *gsscmd[] = { "flameshot", "gui", NULL };

/*
 * Xresources preferences to load at startup
 */
ResourcePref resources[] = {
		{ "normbgcolor",        STRING,  &normbgcolor },
		{ "normbordercolor",    STRING,  &normbordercolor },
		{ "normfgcolor",        STRING,  &normfgcolor },
		{ "selbgcolor",         STRING,  &selbgcolor },
		{ "selbordercolor",     STRING,  &selbordercolor },
		{ "selfgcolor",         STRING,  &selfgcolor },
		{ "borderpx",          	INTEGER, &borderpx },
		{ "snap",          	    INTEGER, &snap },
		{ "showbar",          	INTEGER, &showbar },
		{ "topbar",          	INTEGER, &topbar },
		{ "nmaster",          	INTEGER, &nmaster },
		{ "resizehints",       	INTEGER, &resizehints },
		{ "mfact",       	    FLOAT,   &mfact },
};

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ SUPERKEY,                         XF86XK_AudioRaiseVolume,      spawn,          {.v = volupcmd } },
	{ SUPERKEY,                         XF86XK_AudioLowerVolume,      spawn,          {.v = voldowncmd } },
	{ SUPERKEY,                         XF86XK_AudioMute,             spawn,          {.v = mutecmd } },
	{ SUPERKEY,                     XK_p,      spawn,          {.v = runcmd } },
	{ SUPERKEY|ShiftMask,           XK_Return, spawn,          {.v = termcmd } },
	{ SUPERKEY,                     XK_b,      togglebar,      {0} },
	{ SUPERKEY,                     XK_j,      focusstack,     {.i = +1 } },
	{ SUPERKEY,                     XK_k,      focusstack,     {.i = -1 } },
	{ SUPERKEY|MODKEY,              XK_i,      incnmaster,     {.i = +1 } },
	{ SUPERKEY|MODKEY,              XK_d,      incnmaster,     {.i = -1 } },
	{ SUPERKEY,                     XK_h,      setmfact,       {.f = -0.05} },
	{ SUPERKEY,                     XK_l,      setmfact,       {.f = +0.05} },
	{ SUPERKEY|MODKEY,              XK_h,      incrgaps,       {.i = +1 } },
	{ SUPERKEY|MODKEY,              XK_l,      incrgaps,       {.i = -1 } },
	{ SUPERKEY|MODKEY|ShiftMask,    XK_h,      incrogaps,      {.i = +1 } },
	{ SUPERKEY|MODKEY|ShiftMask,    XK_l,      incrogaps,      {.i = -1 } },
	{ SUPERKEY|MODKEY|ControlMask,  XK_h,      incrigaps,      {.i = +1 } },
	{ SUPERKEY|MODKEY|ControlMask,  XK_l,      incrigaps,      {.i = -1 } },
	{ SUPERKEY|MODKEY,              XK_0,      togglegaps,     {0} },
	{ SUPERKEY|MODKEY|ShiftMask,    XK_0,      defaultgaps,    {0} },
	{ SUPERKEY|MODKEY,              XK_y,      incrihgaps,     {.i = +1 } },
	{ SUPERKEY|MODKEY,              XK_o,      incrihgaps,     {.i = -1 } },
	{ SUPERKEY|MODKEY|ControlMask,  XK_y,      incrivgaps,     {.i = +1 } },
	{ SUPERKEY|MODKEY|ControlMask,  XK_o,      incrivgaps,     {.i = -1 } },
	{ SUPERKEY|MODKEY,              XK_y,      incrohgaps,     {.i = +1 } },
	{ SUPERKEY|MODKEY,              XK_o,      incrohgaps,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_y,      incrovgaps,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,      incrovgaps,     {.i = -1 } },
	{ SUPERKEY,                     XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ SUPERKEY|ShiftMask,           XK_c,      killclient,     {0} },
	{ SUPERKEY,                     XK_t,      setlayout,      {.v = &layouts[0]} },
	{ SUPERKEY,                     XK_f,      setlayout,      {.v = &layouts[1]} },
	{ SUPERKEY,                     XK_m,      setlayout,      {.v = &layouts[2]} },
	{ SUPERKEY,                     XK_space,  setlayout,      {0} },
	{ SUPERKEY|ShiftMask,           XK_space,  togglefloating, {0} },
	{ SUPERKEY|ShiftMask,           XK_f,      togglefullscr,  {0} },
	{ SUPERKEY,                     XK_0,      view,           {.ui = ~0 } },
	{ SUPERKEY|ShiftMask,           XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
 	{ SUPERKEY,                     XK_q,      quit,           {1} },
	{ SUPERKEY|ShiftMask,           XK_q,      spawn,          { .v = powercmd } },
	{ SUPERKEY,             	    XK_Print,  spawn,          { .v = sscmd } },
	{ SUPERKEY|ShiftMask,          	XK_Print,  spawn,          { .v = gsscmd } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         SUPERKEY,       Button1,        movemouse,      {0} },
	{ ClkClientWin,         SUPERKEY,       Button2,        togglefloating, {0} },
	{ ClkClientWin,         SUPERKEY,       Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            SUPERKEY,       Button1,        tag,            {0} },
	{ ClkTagBar,            SUPERKEY,       Button3,        toggletag,      {0} },
};

static const char *ipcsockpath = "/tmp/dwm.sock";
static IPCCommand ipccommands[] = {
  IPCCOMMAND(  view,                1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggleview,          1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tag,                 1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggletag,           1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tagmon,              1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  focusmon,            1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  focusstack,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  zoom,                1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  spawn,               1,      {ARG_TYPE_PTR}    ),
  IPCCOMMAND(  incnmaster,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  killclient,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  togglefloating,      1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  setmfact,            1,      {ARG_TYPE_FLOAT}  ),
  IPCCOMMAND(  setlayoutsafe,       1,      {ARG_TYPE_PTR}    ),
  IPCCOMMAND(  quit,                1,      {ARG_TYPE_NONE}   )
};
