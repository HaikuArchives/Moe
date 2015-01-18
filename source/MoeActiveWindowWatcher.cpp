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


#define private public

#include <cstring>
#include <algorithm>
#include <Roster.h>
#include <Window.h>
#include <List.h>
#include <Locker.h>
#include <Autolock.h>
#include <Looper.h>
#include "MoeDefs.h"
#include "MoeProperty.h"
#include "MoeActiveWindowWatcher.h"


#define TIMEOUT 10000


static BLocker sLocker;
static MoeActiveWindowWatcher *sWatcher;


MoeActiveWindowWatcher *
MoeActiveWindowWatcher::Watcher(void)
{
  BAutolock lock(sLocker);

  if (! sWatcher)
    sWatcher = new MoeActiveWindowWatcher();

  return sWatcher;
}


MoeActiveWindowWatcher::MoeActiveWindowWatcher(void)
  : mCurrentFrame(-10000, -10000, -10000, -10000)
  , mIsActive(false)
{
}


MoeActiveWindowWatcher::~MoeActiveWindowWatcher(void)
{
}


void
MoeActiveWindowWatcher::Lock(void)
{
  sLocker.Lock();
}


void
MoeActiveWindowWatcher::Unlock(void)
{
  sLocker.Unlock();
}


void
MoeActiveWindowWatcher::AddObserver(BHandler *handler)
{
  BAutolock lock(sLocker);

  mObservers.AddItem(handler);

  BMessage msg(MOE_WINDOW_MODIFIED);
  msg.AddRect("frame", mCurrentFrame);
  handler->Looper()->PostMessage(&msg, handler);
}


void
MoeActiveWindowWatcher::RemoveObserver(BHandler *handler)
{
  BAutolock lock(sLocker);

  mObservers.RemoveItem(handler);
}


void
MoeActiveWindowWatcher::NotifyWatcher(void)
{
  BAutolock lock(sLocker);
  BMessage msg(MOE_WINDOW_MODIFIED);
  int32 i;

  for (i =  0; i < mObservers.CountItems(); i++)
    {
      BHandler *observer = reinterpret_cast<BHandler*>(mObservers.ItemAt(i));
      observer->Looper()->PostMessage(&msg, observer);
    }
}


bool
MoeActiveWindowWatcher::IsActive(void) const
{
  return mIsActive;
}


BRect
MoeActiveWindowWatcher::CurrentFrame(void) const
{
  return mCurrentFrame;
}


const BString &
MoeActiveWindowWatcher::CurrentApp(void) const
{
  return mCurrentApp;
}


const BString &
MoeActiveWindowWatcher::CurrentAppName(void) const
{
  return mCurrentAppName;
}


const BString &
MoeActiveWindowWatcher::CurrentWin(void) const
{
  return mCurrentWin;
}


void
MoeActiveWindowWatcher::Run(void)
{
  mThread = ::spawn_thread(MoeActiveWindowWatcher::_TaskLoop, 
			   "watcher",
			   B_LOW_PRIORITY,
			   this);
  ::resume_thread(mThread);
}


void
MoeActiveWindowWatcher::Stop(void)
{
  this->Lock();
  ::kill_thread(mThread);
  this->Unlock();
}


int32
MoeActiveWindowWatcher::_TaskLoop(void *data)
{
  static_cast<MoeActiveWindowWatcher*>(data)->TaskLoop();

  return B_OK;
}


// from interface_defs.h
struct window_info {
  team_id  team;
  int32    id;	  		  /* window's token */
  
  int32	   thread;
  int32	   client_token;
  int32	   client_port;
  uint32   workspaces;
  
  int32    layer;
  uint32   w_type;    	  /* B_TITLED_WINDOW, etc. */
  uint32   flags;	  	  /* B_WILL_FLOAT, etc. */
  int32    window_left;
  int32    window_top;
  int32    window_right;
  int32    window_bottom;
  int32    show_hide_level;
  bool     is_mini;
  char     name[1];
};

#ifndef _IMPEXP_BE
#define _IMPEXP_BE
#endif

_IMPEXP_BE window_info  *get_window_info(int32 a_token);
_IMPEXP_BE int32        *get_token_list(team_id app, int32 *count);


static app_info sAppInfo;

static window_info *
get_active_window(void)
{
  int32 *tokens, count, i;
  window_info *winInfo;

  be_roster->GetActiveAppInfo(&sAppInfo);

  if (MoeProperty::Property()->IsIgnoreApp(sAppInfo.signature))
    return NULL;

  winInfo = NULL;
  tokens = ::get_token_list(sAppInfo.team, &count);

  if (tokens)
    {
      for (i = 0; i < count; i++)
	{
	  winInfo = ::get_window_info(tokens[i]);

	  if (! winInfo)
	    continue;

	  if (winInfo->layer != 3 ||     // avoid unusual windows(e.g. Menu).
	      ((winInfo->w_type >> 8) & 0xff) > 3) // avoid floating windows.
	    {
	      free(winInfo);
	      winInfo = NULL;
	      continue;
	    }

	  break;
	}
      free(tokens);
    }

  return winInfo;
}


static status_t
get_window_state(window_info *winInfo, bool *active, BRect *frame)
{
#ifndef __HAIKU__
  BMessenger win(winInfo->team, winInfo->client_port, winInfo->client_token, false);
#else
  BMessenger win;
  win._SetTo(winInfo->team, winInfo->client_port, winInfo->client_token);
#endif
  BMessage msg(B_GET_PROPERTY), reply;

  msg.AddSpecifier("Active");
  win.SendMessage(&msg, &reply, TIMEOUT, TIMEOUT);

  if (reply.what != B_REPLY ||
      reply.FindBool("result", active) != B_OK)
    return B_ERROR;

  msg.MakeEmpty();
  reply.MakeEmpty();

  msg.AddSpecifier("Frame");
  win.SendMessage(&msg, &reply, TIMEOUT, TIMEOUT);

  if (reply.what != B_REPLY ||
      reply.FindRect("result", frame) != B_OK)
    return B_ERROR;

  return B_OK;
}
   

static inline BRect
window_info_to_rect(window_info *info)
{
  return BRect(float(info->window_left),
	       float(info->window_top), 
	       float(info->window_right), 
	       float(info->window_bottom));
}


static bool
is_beos_decorated(int32 diff)
{
  switch (diff)
    {
    default:
      return true;

    case 14:  // AmigaOS Decoration
    case 22:  // MacOS 8 Decoration
    case 23:  // Windows 95/98 Decorationn
      return false;
    }
}


void
MoeActiveWindowWatcher::TaskLoop(void)
{
  int32 diff;

  while (true)
    {
      window_info *win, *active_win;
      
      while (! (active_win = ::get_active_window()))
	::snooze(MoeProperty::Property()->GetPollingInterval());

      bool active;
      BRect prevFrame;

      if (::get_window_state(active_win, &active, &prevFrame)
	  != B_OK ||
	  ! active)
	{
	  free(active_win);
	  ::snooze(MoeProperty::Property()->GetPollingInterval());
	  continue;
	}

      diff = int(prevFrame.top) - active_win->window_top;
      if (! ::is_beos_decorated(diff))
	diff = 0;
      prevFrame = window_info_to_rect(active_win);
      prevFrame.top += diff;

      sLocker.Lock();
      mIsActive = true;
      mCurrentFrame = prevFrame;
      mCurrentApp = sAppInfo.signature;
      mCurrentAppName = sAppInfo.ref.name;
      mCurrentWin = active_win->name;
      this->NotifyWatcher();
      sLocker.Unlock();

      while (true)
	{
	  win = ::get_active_window();

	  if (! win)
	    break;

	  if (win->id != active_win->id)
	    {
	      free (win);
	      break;
	    }

	  BRect frame = window_info_to_rect(win);
	  frame.top += diff;

	  if (frame != prevFrame)
	    {
	      prevFrame = frame;
	      sLocker.Lock();
	      mCurrentFrame = frame;
	      this->NotifyWatcher();
	      sLocker.Unlock();
	    }

	  free (win);

	  ::snooze(MoeProperty::Property()->GetPollingInterval());
	}
      
      free(active_win);

      sLocker.Lock();
      mIsActive = false;
      this->NotifyWatcher();
      sLocker.Unlock();
    }
}


