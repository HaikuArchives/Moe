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


#include <Bitmap.h>
#include <Message.h>
#include <MessageQueue.h>
#include <Window.h>
#include "MoeWindow.h"
#include "MoeWindowSlice.h"


#define MOE_INVALIDATE 'Mivd'


MoeWindowSlice::MoeWindowSlice(MoeWindow *owner,
			       const char *name)
  : BView(BRect(0, 0, 0, 0), name, B_FOLLOW_NONE, B_WILL_DRAW)
  , mOwner(owner)
{
  this->SetViewColor(B_TRANSPARENT_32_BIT);

  BWindow *aWindow = 
    new BWindow(BRect(-10000, -10000, -10000, -10000),
		name,
		B_NO_BORDER_WINDOW_LOOK,
		B_FLOATING_ALL_WINDOW_FEEL,
		B_AVOID_FOCUS | B_WILL_ACCEPT_FIRST_CLICK);
  aWindow->SetSizeLimits(0, 10000, 0, 10000);
  aWindow->AddChild(this);
  aWindow->Run();
}


MoeWindowSlice::~MoeWindowSlice(void)
{
}
    

void
MoeWindowSlice::SetFrame(BRect srcRect)
{
  mSrcRect = srcRect;
  if (this->LockLooper())
    {
      this->Window()->ResizeTo(srcRect.Width(), srcRect.Height());
      this->ResizeTo(srcRect.Width(), srcRect.Height());
      this->UnlockLooper();
    }
}


void
MoeWindowSlice::MoveTo(BPoint point)
{
  BMessage msg(B_WINDOW_MOVE_TO);
  msg.AddPoint("data", point + mSrcRect.LeftTop());
  this->Window()->PostMessage(&msg);
}


void
MoeWindowSlice::MoveTo(float x, float y)
{
  this->MoveTo(BPoint(x, y));
}


void
MoeWindowSlice::Draw(BRect updateRect)
{
  mOwner->LockBitmap();
  this->DrawBitmap(mOwner->Bitmap(), mSrcRect, this->Bounds());
  mOwner->UnlockBitmap();

  if (! MoeWindow::IsDebugFrameVisible())
    return;

  this->SetHighColor(255, 0, 0);
  this->StrokeRect(this->Bounds());
}


void
MoeWindowSlice::MouseDown(BPoint point)
{
  mOwner->MouseDown(this, point);
}


void
MoeWindowSlice::MouseMoved(BPoint point, uint32 transit, const BMessage *msg)
{
  mOwner->MouseMoved(this, point, transit, msg);
}


void
MoeWindowSlice::MouseUp(BPoint point)
{
  mOwner->MouseUp(this, point);
}


void
MoeWindowSlice::MessageReceived(BMessage *msg)
{
  switch (msg->what)
    {
    default:
      BView::MessageReceived(msg);
      break;

    case MOE_INVALIDATE:
      {
	BRect frame;
	msg->FindRect("frame", &frame);
	this->Invalidate
	  (frame.OffsetBySelf(BPoint(0, 0) - mSrcRect.LeftTop()));
	break;
      }
    }	 
}
