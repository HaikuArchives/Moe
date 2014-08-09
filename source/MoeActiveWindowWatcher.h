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


#ifndef MOE_ACTIVE_WINDOW_WATCHER_H
#define MOE_ACTIVE_WINDOW_WATCHER_H


#include <String.h>
#include <Messenger.h>
#include <OS.h>


class BHandler;
class BList;


class MoeActiveWindowWatcher
{
public:
  static MoeActiveWindowWatcher *Watcher(void);

  void Run(void);
  void Stop(void);

  void AddObserver(BHandler *handler);
  void RemoveObserver(BHandler *handler);

  bool IsActive(void) const;
  BRect CurrentFrame(void) const;
  const BString &CurrentApp(void) const;
  const BString &CurrentAppName(void) const;
  const BString &CurrentWin(void) const;

  void Lock(void);
  void Unlock(void);

private:
  MoeActiveWindowWatcher(void);
  virtual ~MoeActiveWindowWatcher(void);

  void NotifyWatcher(void);

  static int32 _TaskLoop(void *data);
  void TaskLoop(void);

  thread_id mThread;
  BList mObservers;
  BRect mCurrentFrame;
  BString mCurrentApp, mCurrentAppName, mCurrentWin;
  bool mIsActive;
};


#endif
