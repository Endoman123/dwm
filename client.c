/*
 * (C)opyright MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "dwm.h"

static Rule rule[] = {
	/* class			instance	tags						floating */
	{ "Firefox-bin",	"Gecko",	{ [Twww] = "www" },			False },
};

Client *
next(Client *c)
{
	for(; c && !c->tags[tsel]; c = c->next);
	return c;
}

void
ban_client(Client *c)
{
	XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	XMoveWindow(dpy, c->title, c->tx + 2 * sw, c->ty);
}

static void
resize_title(Client *c)
{
	int i;

	c->tw = 0;
	for(i = 0; i < TLast; i++)
		if(c->tags[i])
			c->tw += textw(c->tags[i]);
	c->tw += textw(c->name);
	if(c->tw > c->w)
		c->tw = c->w + 2;
	c->tx = c->x + c->w - c->tw + 2;
	c->ty = c->y;
	XMoveResizeWindow(dpy, c->title, c->tx, c->ty, c->tw, c->th);
}

void
update_name(Client *c)
{
	XTextProperty name;
	int n;
	char **list = NULL;

	name.nitems = 0;
	c->name[0] = 0;
	XGetTextProperty(dpy, c->win, &name, net_atom[NetWMName]);
	if(!name.nitems)
		XGetWMName(dpy, c->win, &name);
	if(!name.nitems)
		return;
	if(name.encoding == XA_STRING)
		strncpy(c->name, (char *)name.value, sizeof(c->name));
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
				&& n > 0 && *list)
		{
			strncpy(c->name, *list, sizeof(c->name));
			XFreeStringList(list);
		}
	}
	XFree(name.value);
	resize_title(c);
}

void
update_size(Client *c)
{
	XSizeHints size;
	long msize;
	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
		size.flags = PSize;
	c->flags = size.flags;
	if(c->flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else
		c->basew = c->baseh = 0;
	if(c->flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(c->flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(c->flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else
		c->minw = c->minh = 0;
	if(c->flags & PWinGravity)
		c->grav = size.win_gravity;
	else
		c->grav = NorthWestGravity;
}

void
craise(Client *c)
{
	XRaiseWindow(dpy, c->win);
	XRaiseWindow(dpy, c->title);
}

void
lower(Client *c)
{
	XLowerWindow(dpy, c->title);
	XLowerWindow(dpy, c->win);
}

void
focus(Client *c)
{
	Client *old = sel;
	XEvent ev;

	XFlush(dpy);
	sel = c;
	if(old && old != c)
		draw_client(old);
	draw_client(c);
	XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	XFlush(dpy);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

static void
init_tags(Client *c)
{
	XClassHint ch;
	static unsigned int len = rule ? sizeof(rule) / sizeof(rule[0]) : 0;
	unsigned int i, j;
	Bool matched = False;

	if(!len) {
		c->tags[tsel] = tags[tsel];
		return;
	}

	if(XGetClassHint(dpy, c->win, &ch)) {
		if(ch.res_class && ch.res_name) {
			for(i = 0; i < len; i++)
				if(!strncmp(rule[i].class, ch.res_class, sizeof(rule[i].class))
					&& !strncmp(rule[i].instance, ch.res_name, sizeof(rule[i].instance)))
				{
					for(j = 0; j < TLast; j++)
						c->tags[j] = rule[i].tags[j];
					c->floating = rule[i].floating;
					matched = True;
					break;
				}
		}
		if(ch.res_class)
			XFree(ch.res_class);
		if(ch.res_name)
			XFree(ch.res_name);
	}

	if(!matched)
		c->tags[tsel] = tags[tsel];
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c, **l;
	XSetWindowAttributes twa;
	Window trans;

	c = emallocz(sizeof(Client));
	c->win = w;
	c->tx = c->x = wa->x;
	c->ty = c->y = wa->y;
	if(c->y < bh)
		c->ty = c->y += bh;
	c->tw = c->w = wa->width;
	c->h = wa->height;
	c->th = bh;
	c->border = 1;
	c->proto = win_proto(c->win);
	update_size(c);
	XSelectInput(dpy, c->win,
			StructureNotifyMask | PropertyChangeMask | EnterWindowMask);
	XGetTransientForHint(dpy, c->win, &trans);
	twa.override_redirect = 1;
	twa.background_pixmap = ParentRelative;
	twa.event_mask = ExposureMask;

	c->title = XCreateWindow(dpy, root, c->tx, c->ty, c->tw, c->th,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen),
			CWOverrideRedirect | CWBackPixmap | CWEventMask, &twa);

	update_name(c);
	init_tags(c);

	for(l = &clients; *l; l = &(*l)->next);
	c->next = *l; /* *l == nil */
	*l = c;

	XGrabButton(dpy, Button1, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button2, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);
	XGrabButton(dpy, Button3, Mod1Mask, c->win, False, ButtonPressMask,
			GrabModeAsync, GrabModeSync, None, None);

	if(!c->floating)
		c->floating = trans
			|| ((c->maxw == c->minw) && (c->maxh == c->minh));

	arrange(NULL);
	/* mapping the window now prevents flicker */
	if(c->tags[tsel]) {
		XMapRaised(dpy, c->win);
		XMapRaised(dpy, c->title);
		focus(c);
	}
	else {
		ban_client(c);
		XMapRaised(dpy, c->win);
		XMapRaised(dpy, c->title);
	}
}

void
gravitate(Client *c, Bool invert)
{
	int dx = 0, dy = 0;

	switch(c->grav) {
	case StaticGravity:
	case NorthWestGravity:
	case NorthGravity:
	case NorthEastGravity:
		dy = c->border;
		break;
	case EastGravity:
	case CenterGravity:
	case WestGravity:
		dy = -(c->h / 2) + c->border;
		break;
	case SouthEastGravity:
	case SouthGravity:
	case SouthWestGravity:
		dy = -c->h;
		break;
	default:
		break;
	}

	switch (c->grav) {
	case StaticGravity:
	case NorthWestGravity:
	case WestGravity:
	case SouthWestGravity:
		dx = c->border;
		break;
	case NorthGravity:
	case CenterGravity:
	case SouthGravity:
		dx = -(c->w / 2) + c->border;
		break;
	case NorthEastGravity:
	case EastGravity:
	case SouthEastGravity:
		dx = -(c->w + c->border);
		break;
	default:
		break;
	}

	if(invert) {
		dx = -dx;
		dy = -dy;
	}
	c->x += dx;
	c->y += dy;
}


void
resize(Client *c, Bool inc)
{
	XConfigureEvent e;

	if(inc) {
		if(c->incw)
			c->w -= (c->w - c->basew) % c->incw;
		if(c->inch)
			c->h -= (c->h - c->baseh) % c->inch;
	}
	if(c->x > sw) /* might happen on restart */
		c->x = sw - c->w;
	if(c->y > sh)
		c->ty = c->y = sh - c->h;
	if(c->minw && c->w < c->minw)
		c->w = c->minw;
	if(c->minh && c->h < c->minh)
		c->h = c->minh;
	if(c->maxw && c->w > c->maxw)
		c->w = c->maxw;
	if(c->maxh && c->h > c->maxh)
		c->h = c->maxh;
	resize_title(c);
	XSetWindowBorderWidth(dpy, c->win, 1);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	e.type = ConfigureNotify;
	e.event = c->win;
	e.window = c->win;
	e.x = c->x;
	e.y = c->y;
	e.width = c->w;
	e.height = c->h;
	e.border_width = c->border;
	e.above = None;
	e.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&e);
	XFlush(dpy);
}

static int
dummy_error_handler(Display *dsply, XErrorEvent *err)
{
	return 0;
}

void
unmanage(Client *c)
{
	Client **l;

	XGrabServer(dpy);
	XSetErrorHandler(dummy_error_handler);

	XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
	XDestroyWindow(dpy, c->title);

	for(l = &clients; *l && *l != c; l = &(*l)->next);
	*l = c->next;
	for(l = &clients; *l; l = &(*l)->next)
		if((*l)->revert == c)
			(*l)->revert = NULL;
	if(sel == c)
		sel = sel->revert ? sel->revert : clients;

	free(c);

	XFlush(dpy);
	XSetErrorHandler(error_handler);
	XUngrabServer(dpy);
	arrange(NULL);
	if(sel)
		focus(sel);
}

Client *
gettitle(Window w)
{
	Client *c;
	for(c = clients; c; c = c->next)
		if(c->title == w)
			return c;
	return NULL;
}

Client *
getclient(Window w)
{
	Client *c;
	for(c = clients; c; c = c->next)
		if(c->win == w)
			return c;
	return NULL;
}