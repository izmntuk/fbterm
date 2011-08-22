/** 
 *           File:  clipboard.h
 *           Path:  ~/src/fbterm-1.7/src/lib
 *         Author:  zhangyuanwei
 *       Modifier:  zhangyuanwei
 *       Modified:  2011-08-20 01:12:51  
 *    Description:  剪切板支持，可以支持X系统的剪切板
 *      Copyright:  (c) 2011 All Rights Reserved
 */
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>

#include "type.h"


class ClipBoard {
public:
	ClipBoard();
	~ClipBoard();

	void setText(s8 *text);
	s8* getText();

private:
	static void* startWait(void*);
	void waitForXcin();
	bool hasDisplay();

	Display* dpy;
	Window win;
	Atom sseln;// = XA_PRIMARY;	/* X selection to work with */
	Atom target;// = XA_STRING;
	s8* buffer;
	u32 buffer_size;
	u32 buffer_len;
	pthread_t tid;
};

#endif
