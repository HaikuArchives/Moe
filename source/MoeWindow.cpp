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


#include <cstring>
#include <algorithm>
#include <String.h>
#include <Message.h>
#include <Bitmap.h>
#include <Screen.h>
#include <Window.h>
#include <OS.h>
#include "MoeDefs.h"
#include "MoeSurface.h"
#include "MoeAnime.h"
#include "MoeWindowSlice.h"
#include "MoeWindow.h"


#define MOE_INVALIDATE 'Mivd'


bool sIsDebugFrameVisible = false;


void
MoeWindow::SetDebugFrameVisible(bool visible)
{
  if (visible == sIsDebugFrameVisible)
    return;

  sIsDebugFrameVisible = visible;

  thread_info info;
  int32 cookie = 0;

  while (::get_next_thread_info(0, &cookie, &info) == B_OK)
    {
      MoeWindow *win =
	dynamic_cast<MoeWindow*>(BLooper::LooperForThread(info.thread));
      if (win)
	win->PostMessage(MOE_INVALIDATE);
    }
}


bool
MoeWindow::IsDebugFrameVisible(void)
{
  return sIsDebugFrameVisible;
}


MoeWindow::MoeWindow(const char *name)
  : BLooper(name)
  , mSurface(NULL)
  , mBackground(NULL)
  , mBitmap(NULL)
  , mPoint(-10000, -10000)
  , mShowCount(-1)
{
}


MoeWindow::~MoeWindow(void)
{
  delete mBackground;
  delete mBitmap;
  delete mSurface;
}


void
MoeWindow::Quit(void)
{
  int32 i;

  for (i = 0; i < this->CountSlices(); i++)
    {
      this->SliceAt(i)->Looper()->Lock();
      this->SliceAt(i)->Looper()->Quit();
    }

  BLooper::Quit();
}


void
MoeWindow::AllocSlices(void)
{
  int32 i, objSize, curSize;

  objSize = mSurface->CountRects();
  curSize = mSlices.CountItems();

  for (i = curSize; i < objSize; i++)
    {
      BString buf;
      buf << this->Name() << " [" << i << "]";
      MoeWindowSlice *slice = new MoeWindowSlice(this, buf.String());
      mSlices.AddItem(slice);
    }
    
  for (i = objSize; i < curSize; i++)
    {
      this->SliceAt(i)->Looper()->Lock();
      this->SliceAt(i)->Looper()->Quit();
    }

  if (objSize < curSize)
    mSlices.RemoveItems(objSize, curSize - objSize);

  for (i = 0; i < objSize; i++)
    this->SliceAt(i)->SetFrame(mSurface->RectAt(i));
}


void
MoeWindow::SetCellSize(int32 cellSize)
{
  mSurface->SetCellSize(cellSize);

  this->AllocSlices();
}


void
MoeWindow::AttachSurface(MoeSurface *surface)
{
  delete mBitmap;
  delete mSurface;

  mSurface = surface;

  mBitmap = new BBitmap(mSurface->Bitmap()->Bounds(), B_RGB32, true);
  mOffscreenView = new BView(mSurface->Bounds(), "", 0, B_WILL_DRAW);
  mBitmap->Lock();
  mBitmap->AddChild(mOffscreenView);
  mBitmap->Unlock();

  this->AllocSlices();
}


int32
MoeWindow::CountSlices(void) const
{
  return mSlices.CountItems();
}


MoeWindowSlice *
MoeWindow::SliceAt(int32 index)
{
  return reinterpret_cast<MoeWindowSlice*>(mSlices.ItemAt(index));
}


MoeSurface *
MoeWindow::Surface(void)
{
  return mSurface;
}


BRect
MoeWindow::Frame(void) const
{
  return mBitmap->Bounds().OffsetToCopy(mPoint);
}


const BBitmap *
MoeWindow::Bitmap(void) const
{
  return mBitmap;
}


status_t
MoeWindow::LockBitmap(void)
{
  return mBitmap->Lock();
}


void
MoeWindow::UnlockBitmap(void)
{
  mBitmap->Unlock();
}


void
MoeWindow::Sync(void)
{
  int32 i;

  for (i = 0; i < this->CountSlices(); i++)
    this->SliceAt(i)->Window()->Sync();
}


void
MoeWindow::ComposeBitmap(bool captureScreen)
{
  BRect bounds;
  int32 i;

  if (captureScreen)
    {
      delete mBackground;
      mBackground = NULL;

      bounds = mSurface->Bounds().OffsetToSelf(mPoint);
      mClipping = bounds & BScreen().Frame();
      
      if (! mClipping.IsValid())
	return;

      BScreen().GetBitmap(&mBackground, false, &mClipping);
    }

  if (! mClipping.IsValid())
    return;

  mBitmap->Lock();
  mOffscreenView->SetDrawingMode(B_OP_COPY);
  mOffscreenView->DrawBitmap(mSurface->Bitmap());

  for (i = 1; i < this->CountHandlers(); i++)
    {
      MoeAnime *anime = dynamic_cast<MoeAnime*>(this->HandlerAt(i));
      if (! anime)
	continue;
      anime->Draw(mOffscreenView);
    }

  BBitmap buf(mBitmap);

  mOffscreenView->DrawBitmap(mBackground,
			     mClipping.LeftTop() - mPoint);
  mOffscreenView->SetDrawingMode(B_OP_ALPHA);
  mOffscreenView->DrawBitmap(&buf);
  mBitmap->Unlock();
}


void
MoeWindow::Invalidate(BRect updateRect)
{
  int32 i;

  BMessage msg(MOE_INVALIDATE);
  msg.AddRect("frame", updateRect);

  for (i = 0; i < this->CountSlices(); i++)
    {
      MoeWindowSlice *slice = this->SliceAt(i);
      if (slice->mSrcRect.Intersects(updateRect))
	slice->Looper()->PostMessage(&msg, slice);
    }
}


void
MoeWindow::MoveTo(BPoint point)
{
  mPoint = point;

  for (int32 i = 0; i < this->CountSlices(); i++)
    this->SliceAt(i)->MoveTo(mPoint);
}


void
MoeWindow::MoveTo(float x, float y)
{
  this->MoveTo(BPoint(x, y));
}


bool
MoeWindow::IsHidden(void) const
{
  return (mShowCount < 0);
}


void
MoeWindow::Show(void)
{
  if (++mShowCount != 0)
    return;

  this->ComposeBitmap();

  for (int32 i = 0; i < this->CountSlices(); i++)
    this->SliceAt(i)->Window()->Show();
}


void
MoeWindow::Hide(void)
{
  if (--mShowCount != -1)
    return;

  for (int32 i = 0; i < this->CountSlices(); i++)
    this->SliceAt(i)->Window()->Hide();
}


void
MoeWindow::AddAnime(MoeAnime *anime)
{
  this->AddHandler(anime);
}


int32
MoeWindow::CountAnimes(void)
{
  return this->CountHandlers() - 1;
}


MoeAnime *
MoeWindow::AnimeAt(int32 index)
{
  return dynamic_cast<MoeAnime*>(this->HandlerAt(index + 1));
}


void
MoeWindow::FindAnimesMatch(BList *result, const char *prefixMatch)
{
  int32 i;
  size_t matchLength;

  result->MakeEmpty();
  matchLength = ::strlen(prefixMatch);

  for (i = 0; i < this->CountAnimes(); i++)
    {
      MoeAnime *anime = this->AnimeAt(i);
      if (::strncmp(prefixMatch, anime->Name(), matchLength) == 0)
	result->AddItem(anime);
    }
}


void
MoeWindow::MouseDown(MoeWindowSlice *slice,
		     BPoint point)
{
}


void
MoeWindow::MouseUp(MoeWindowSlice *slice,
		   BPoint point)
{
}


void
MoeWindow::MouseMoved(MoeWindowSlice *slice, 
		      BPoint point, uint32 transit, const BMessage *msg)
{
}


void
MoeWindow::MessageReceived(BMessage *msg)
{
  switch (msg->what)
    {
    default:
      BLooper::MessageReceived(msg);
      break;

    case MOE_INVALIDATE:
      {
	BRect frame;
	if (msg->FindRect("frame", &frame) != B_OK)
	  frame = this->Frame().OffsetToSelf(0, 0);
	this->Invalidate(frame);
	break;
      }
    }
}


