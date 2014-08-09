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


#include <Looper.h>
#include <Message.h>
#include "MoeForwardFilter.h"


MoeForwardFilter::MoeForwardFilter(uint32 command, BHandler *target)
  : BMessageFilter(command)
  , mCommand(command)
  , mTarget(target)
{
}


MoeForwardFilter::MoeForwardFilter(uint32 command, 
				 uint32 commandModifier,
				 BHandler *target)
  : BMessageFilter(command)
  , mCommand(commandModifier)
  , mTarget(target)
{
}


MoeForwardFilter::~MoeForwardFilter(void)
{
}


filter_result
MoeForwardFilter::Filter(BMessage *msg, BHandler **target)
{
  BMessage *forward = new BMessage(*msg);

  mTarget->Looper()->PostMessage(forward, mTarget);

  return B_DISPATCH_MESSAGE;
}

