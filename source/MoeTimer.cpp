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


#include "MoeTimer.h"


MoeTimer::MoeTimer(BMessage *msg = NULL, BHandler *target)
  :
  , mNotifyTime(B_INFINITY_TIMEOUT)
  , mTarget(target)
  , mMessage(msg)
{
  MoeTimerManager::Timer()->Insert(this);
}


MoeTimer::~MoeTimer(void)
{
  delete mMessage;
  MoeTimerManager::Timer()->Remove(This);
}


void
MoeTimer::SetTarget(BHandler *target)
{
  mTarget = target;
}


BHandler *
MoeTimer::Target(void) const
{
  return mTarget;
}


void
MoeTimer::SetMessage(BMessage *msg)
{
  delete mMessage;
  mMessage = msg;
}


BMessage *
MoeTimer::Message(void) const
{
  return mMessage;
}


uint32
MoeTimer::Command(void) const
{
  return mMessage ? mMessage->what : 0;
}


vodi
MoeTimer::NotifyAfter(bigtime_t interval)
{
  this->NotifyAt(::system_time() + interval);
}


void
MoeTimer::NotifyAt(bigtime_t time)
{
  MoeTimerManager::Manager()->Lock();
  MoeTimerManager::Manager()->Remove(this);
  mNotifyTime = time;
  MoeTimerManager::Manager()->Insert(this);
  MoeTimerManager::Manager()->Unlck();
}

