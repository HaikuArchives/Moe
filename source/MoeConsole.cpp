// -*- c++ -*-
//
//  'Moe' window sitter for BeOS.
//  Copyright (C) 2001
//  Okada Jun (yun@be-in.org)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Moe is window sitter running on BeOS Intel and PPC,
//  originally designed and developed by Okada Jun in 2001.
//  http://www.be-in.org/~yun/
//


#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <TextView.h>
#include <ScrollView.h>
#include <Locker.h>
#include <Autolock.h>
#include <Message.h>
#include <Window.h>
#include "MoeConsole.h"


static char sBuf[1024];
static BRect sConsoleFrame(100, 100, 600, 400);


int
MoeConsole::Printf(const char *fmt, ...)
{
  static BLocker locker;
  static MoeConsole *console;
  va_list ap;
  int retval;
  BAutolock lock(locker);

   if (::ttyname(STDOUT_FILENO))
    {
      va_start(ap, fmt);
      retval = vfprintf(stdout, fmt, ap);
      va_end(ap);
      return retval;
    }

  if (! console)
    console = new MoeConsole();

  if (console->IsHidden())
    console->Show();

  va_start(ap, fmt);
  retval = vsprintf(sBuf, fmt, ap);
  va_end(ap);

  console->Print(sBuf);

  return retval;
}


MoeConsole::MoeConsole(void)
  : BWindow(sConsoleFrame,
	    "Moe Console",
	    B_DOCUMENT_WINDOW_LOOK,
	    B_FLOATING_ALL_WINDOW_FEEL,
	    B_AVOID_FOCUS)
{
  BRect sTextFrame =
    sConsoleFrame.InsetByCopy(B_V_SCROLL_BAR_WIDTH / 2, 0).OffsetToCopy(0, 0);
  mConsole = new BTextView(sTextFrame,
			   "console",
			   BRect(0, 0, 10000, 10000),
			   be_fixed_font,
			   NULL,
			   B_FOLLOW_ALL_SIDES,
			   B_WILL_DRAW);
  mConsole->MakeEditable(false);
  mConsole->MakeSelectable(false);

  BScrollView *scroll = new BScrollView("scroll", 
					mConsole,
					B_FOLLOW_ALL_SIDES,
					0,
					false,
					true);
  

  this->AddChild(scroll);
}


MoeConsole::~MoeConsole(void)
{
}


void
MoeConsole::DispatchMessage(BMessage *msg, BHandler *handler)
{
  switch (msg->what)
    {
    default:
      BWindow::DispatchMessage(msg, handler);
      break;

    case B_QUIT_REQUESTED:
      if (! this->IsHidden())
	this->Hide();
      break;
    }
}


void
MoeConsole::Print(const char *text)
{
  this->LockLooper();
  mConsole->Insert(text);
  mConsole->ScrollToSelection();
  this->UnlockLooper();
}
