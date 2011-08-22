/** 
 *           File:  clipboard.cpp
 *           Path:  ~/src/fbterm-1.7/src/lib
 *         Author:  zhangyuanwei
 *       Modifier:  zhangyuanwei
 *       Modified:  2011-08-20 01:19:57  
 *    Description:  剪切板支持，可以支持X系统的剪切板
 *      Copyright:  (c) 2011 All Rights Reserved
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "clipboard.h"
#include "xclib.h"

ClipBoard::ClipBoard()
{
	buffer_size = 1;
	buffer_len = 0;
	buffer = (s8*) malloc(buffer_size);
	buffer[buffer_size-1] = '\0';

	tid = 0;

	dpy = NULL;
	target = NULL;
	sseln = NULL;
	win = NULL;
}

ClipBoard::~ClipBoard()
{
	if(dpy){
		XCloseDisplay(dpy);
	}
	free(buffer);
}


bool ClipBoard::hasDisplay(){
	if (!dpy && (dpy = XOpenDisplay(NULL))) {
		target = XA_UTF8_STRING(dpy);
		sseln = XA_PRIMARY;
		win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0, 0, 0);
	}
	return dpy && win;
}

void* ClipBoard::startWait(void* pClip){
	ClipBoard* clipboard = (ClipBoard *) pClip;
	clipboard->waitForXcin();
}

void ClipBoard::waitForXcin()
{
	Window cwin;
	XEvent evt;
	Atom pty;
	//static unsigned int clear = 0;
	unsigned int context = XCLIB_XCIN_NONE;
	unsigned long sel_pos = 0;
	int finished;
   
	XSetSelectionOwner(dpy, sseln, win, CurrentTime);
	while(1){
		XNextEvent(dpy, &evt);
		xcin(dpy, &cwin, evt, &pty, target, (unsigned char*)buffer, buffer_len, &sel_pos, &context);
		if ((context == XCLIB_XCIN_NONE) && evt.type == SelectionClear )
			break;
	}
	tid = 0;
}

void ClipBoard::setText(s8 *text)
{
	u32 txt_len;
	txt_len = strlen(text);
	buffer_len = txt_len;
	if(buffer_size < txt_len+1){
		buffer_size = txt_len + 1;
		buffer = (s8*) realloc(buffer, buffer_size);
	}
	memcpy(buffer, text, txt_len);
	buffer[txt_len] = '\0';


	if(!tid && hasDisplay()){
		pthread_create( &tid, NULL, startWait, this);
	}
}

s8* ClipBoard::getText()
{
	Atom sel_type = None;
	unsigned char *sel_buf;
	unsigned long sel_len = 0;
	XEvent evt;

	unsigned int context = XCLIB_XCOUT_NONE;
	if(!tid && hasDisplay()){
		while (1) {
			if (context != XCLIB_XCOUT_NONE)
				XNextEvent(dpy, &evt);
			xcout(dpy, win, evt, sseln, target, &sel_type, &sel_buf, &sel_len, &context);
			if (context == XCLIB_XCOUT_BAD_TARGET) {
				if (target == XA_UTF8_STRING(dpy)) {
					context = XCLIB_XCOUT_NONE;
					target = XA_STRING;
					continue;
				}else {
					free(sel_buf);
					return "";
				}
			}
			if (context == XCLIB_XCOUT_NONE)
				break;
		}

		if (sel_len) {
			buffer_len = sel_len;
			if(buffer_size < sel_len+1){
				buffer_size = sel_len + 1;
				buffer = (s8*) realloc(buffer, buffer_size);
			}
			memcpy(buffer, sel_buf, sel_len);
			buffer[sel_len] = '\0';
			free(sel_buf);
		}
	}
	return buffer;
}
