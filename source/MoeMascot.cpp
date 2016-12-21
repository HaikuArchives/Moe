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


#include <algorithm>
#include <cmath>
#include <fs_attr.h>
#include <Invoker.h>
#include <Alert.h>
#include <Bitmap.h>
#include <NodeMonitor.h>
#include <Node.h>
#include <Message.h>
#include <MessageRunner.h>
#include <Window.h>
#include <Application.h>
#include "MoeDefs.h"
#include "MoeUtils.h"
#include "MoeSurface.h"
#include "MoeActiveWindowWatcher.h"
#include "MoeForwardFilter.h"
#include "MoeProperty.h"
#include "MoeWindowSlice.h"
#include "MoeAnime.h"
#include "MoeRectTransformer.h"
#include "MoeMascotMenu.h"
#include "MoeMascot.h"


MoeMascot::MoeMascot(const entry_ref &ref, MoeSurface *surface)
  : MoeWindow(ref.name)
  , mEntry(ref)
  , mSize(1.0)
  , mSide(MOE_LEFT_TOP_RIGHT_BOTTOM)
  , mXRate(0.5)
  , mYRate(0.5)
  , mIsXLocked(false)
  , mIsYLocked(false)
  , mTargetFrame(-10000, -10000, -10000, -10000)
  , mPrevTargetFrame(-10000, -10000, -10000, -10000)
  , mIsReadyToDrag(false)
  , mIsDragging(false)
  , mTrigger(NULL)
  , mWinkTrigger(NULL)
{
  this->AttachSurface(surface);
  this->InitAttr();
  BLooper *activater = this->SliceAt(0)->Looper();
  activater->Lock();
  activater->AddCommonFilter(new MoeForwardFilter(B_WORKSPACE_ACTIVATED, this));
  activater->Unlock();

  this->Run();

  MoeActiveWindowWatcher::Watcher()->AddObserver(this);
  this->StartNodeMonitoring();
}


MoeMascot::~MoeMascot(void)
{
  this->EndNodeMonitoring();
  MoeActiveWindowWatcher::Watcher()->RemoveObserver(this);
  delete mTrigger;
  delete mWinkTrigger;
}


void
MoeMascot::StartNodeMonitoring(void)
{
  BEntry entry(&mEntry);
  
  if (entry.InitCheck() == B_OK)
    {
      node_ref ref;
      entry.GetNodeRef(&ref);
      ::watch_node(&ref, B_ENTRY_MOVED, this);
    }
}


void
MoeMascot::EndNodeMonitoring(void)
{
  ::stop_watching(this);
}


void
MoeMascot::PursuitNode(const BMessage *msg)
{
  ino_t dir;
  dev_t dev;
  const char *name;
  
  msg->FindInt64("to directory", &dir);
  msg->FindInt32("device", &dev);
  msg->FindString("name", &name);
  
  mEntry = entry_ref(dev, dir, name);
}


void
MoeMascot::InitAttr(void)
{
  BNode node;
  attr_info info;

  if (node.SetTo(&mEntry) != B_OK ||
      node.Lock() != B_OK)
    return;

  if (node.GetAttrInfo(MOE_ATTR_SIZE, &info) == B_OK &&
      info.type == B_FLOAT_TYPE &&
      info.size == sizeof(float))
    node.ReadAttr(MOE_ATTR_SIZE, B_FLOAT_TYPE, 0, &mSize, sizeof(float));

  if (node.GetAttrInfo(MOE_ATTR_SIDE, &info) == B_OK &&
      info.type == B_INT32_TYPE &&
      info.size == sizeof(int32))
    node.ReadAttr(MOE_ATTR_SIDE, B_INT32_TYPE, 0, &mSide, sizeof(int32));

  if (node.GetAttrInfo(MOE_ATTR_X_RATE, &info) == B_OK &&
      info.type == B_FLOAT_TYPE &&
      info.size == sizeof(float))
    node.ReadAttr(MOE_ATTR_X_RATE, B_FLOAT_TYPE, 0, &mXRate, sizeof(float));

  if (node.GetAttrInfo(MOE_ATTR_Y_RATE, &info) == B_OK &&
      info.type == B_FLOAT_TYPE &&
      info.size == sizeof(float))
    node.ReadAttr(MOE_ATTR_Y_RATE, B_FLOAT_TYPE, 0, &mYRate, sizeof(float));

  if (node.GetAttrInfo(MOE_ATTR_X_LOCKED, &info) == B_OK &&
      info.type == B_BOOL_TYPE &&
      info.size == sizeof(bool))
    node.ReadAttr(MOE_ATTR_X_LOCKED, B_BOOL_TYPE, 0, &mIsXLocked, sizeof(bool));

  if (node.GetAttrInfo(MOE_ATTR_Y_LOCKED, &info) == B_OK &&
      info.type == B_BOOL_TYPE &&
      info.size == sizeof(bool))
    node.ReadAttr(MOE_ATTR_Y_LOCKED, B_BOOL_TYPE, 0, &mIsYLocked, sizeof(bool));

  if (node.GetAttrInfo(MOE_ATTR_PREF_APP, &info) == B_OK &&
      info.type == B_STRING_TYPE)
    {
      char *buf = new char [info.size];
      node.ReadAttr(MOE_ATTR_PREF_APP, B_STRING_TYPE, 0,
		    buf, info.size);
      mPrefApp = buf;
      delete[] buf;
    }

  if (node.GetAttrInfo(MOE_ATTR_PREF_WIN, &info) == B_OK &&
      info.type == B_STRING_TYPE)
    {
      char *buf = new char [info.size];
      node.ReadAttr(MOE_ATTR_PREF_WIN, B_STRING_TYPE, 0,
		    buf, info.size);
      mPrefWin = buf;
      delete[] buf;
    }

  node.Unlock();
}


void 
MoeMascot::WriteAttr(const char *name, type_code type,
		     const void *buffer, size_t length)
{
  BNode node;

  if (node.SetTo(&mEntry) != B_OK ||
      node.Lock() != B_OK)
    return;

  node.WriteAttr(name, type, 0, buffer, length);

  node.Unlock();
}


float
MoeMascot::Size(void) const
{
  return mSize;
}


int32
MoeMascot::Side(void) const
{
  return mSide;
}


bool
MoeMascot::IsXLocked(void) const
{
  return mIsXLocked;
}


bool
MoeMascot::IsYLocked(void) const
{
  return mIsYLocked;
}


const BString &
MoeMascot::PrefApp(void) const
{
  return mPrefApp;
}


const BString &
MoeMascot::PrefWin(void) const
{
  return mPrefWin;
}


void
MoeMascot::SetSize(float data)
{
  if (mSize == data)
    return;

  mSize = data;

  this->WriteAttr(MOE_ATTR_SIZE, B_FLOAT_TYPE, &data, sizeof(float));

  BMessage msg(MOE_MASCOT_REOPEN_REQUESTED);
  msg.AddPointer("data", this);
  be_app->PostMessage(&msg);
}


void
MoeMascot::SetSide(int32 data)
{
  if (mSide == data)
    return;

  mSide = data;

  this->WriteAttr(MOE_ATTR_SIDE, B_INT32_TYPE, &data, sizeof(int32));

  this->MoveToPrefered();
}


void
MoeMascot::SetXLock(bool data)
{
  if (mIsXLocked == data)
    return;

  mIsXLocked = data;

  this->WriteAttr(MOE_ATTR_X_LOCKED, B_BOOL_TYPE, &data, sizeof(bool));
}


void
MoeMascot::SetYLock(bool data)
{
  if (mIsYLocked == data)
    return;

  mIsYLocked = data;

  this->WriteAttr(MOE_ATTR_Y_LOCKED, B_BOOL_TYPE, &data, sizeof(bool));
}


void
MoeMascot::SetXRate(float data)
{
  if (mXRate == data)
    return;

  mXRate = data;

  this->WriteAttr(MOE_ATTR_X_RATE, B_FLOAT_TYPE, &data, sizeof(float));
}


void
MoeMascot::SetYRate(float data)
{
  if (mYRate == data)
    return;

  mYRate = data;

  this->WriteAttr(MOE_ATTR_Y_RATE, B_FLOAT_TYPE, &data, sizeof(float));
}


void
MoeMascot::SetPrefApp(const BString &data)
{
  if (mPrefApp == data)
    return;

  mPrefApp = data;

  this->WriteAttr(MOE_ATTR_PREF_APP, B_STRING_TYPE,
		  data.String(), data.Length() + 1);

  this->MoveToPrefered();
}


void
MoeMascot::SetPrefWin(const BString &data)
{
  if (mPrefWin == data)
    return;

  mPrefWin = data;

  this->WriteAttr(MOE_ATTR_PREF_WIN, B_STRING_TYPE,
		  data.String(), data.Length() + 1);

  this->MoveToPrefered();
}


void
MoeMascot::Redraw(void)
{
  if (! this->IsHidden())
    {
      this->Hide();
      this->Sync();
    }

  delete mTrigger;

  if (mIsDragging)
    mTrigger = NULL;
  else
    {
      BMessage msg(MOE_WINDOW_SHOW);
      mTrigger = new BMessageRunner(BMessenger(this),
				    &msg,
				    MoeProperty::Property()->GetRedrawInterval(),
				    1);
    }
}


void
MoeMascot::MoveToPrefered(void)
{
  BPoint point;
  BRect frame(this->Frame());
  MoeActiveWindowWatcher *watcher = MoeActiveWindowWatcher::Watcher();

  watcher->Lock();
  if (watcher->IsActive() &&
      (mPrefApp.Length() == 0 || mPrefApp == watcher->CurrentApp()) &&
      (mPrefWin.Length() == 0 || mPrefWin == watcher->CurrentWin()))
    mTargetFrame = watcher->CurrentFrame();
  else
    mTargetFrame.Set(-10000, -10000, -10000, -10000);
  watcher->Unlock();

  MoeRectTransformer frameAccessor(&frame, mSide),
    targetAccessor(&mTargetFrame, mSide);
  MoePointTransformer pointAccessor(&point, mSide);
  pointAccessor.X() = 
    int(targetAccessor.Left() +
	mXRate * std::max(0.0f, targetAccessor.Width() - frameAccessor.Width()));
  pointAccessor.Y() = 
    int(targetAccessor.Top() - 
	targetAccessor.YSign() * ((targetAccessor.YSign() > 0) - mYRate) *
	frameAccessor.Height());

  if (point != frame.LeftTop() ||
      this->IsHidden())
    {
      this->Redraw();
      this->MoveTo(point);
    }
}


void
MoeMascot::Wink(void)
{
  BList winkAnimes;

  delete mWinkTrigger;
  mWinkTrigger = NULL;
  BInvoker *reply = new BInvoker(new BMessage(MOE_WINK_DONE), this);
  this->FindAnimesMatch(&winkAnimes, "WINK");
  reinterpret_cast<MoeAnime*>
    (winkAnimes.ItemAt(MoeUtils::Rand(winkAnimes.CountItems())))
    ->Play(reply);
}


void
MoeMascot::WinkDone(void)
{
  bigtime_t interval =
  MoeUtils::Rand(MoeProperty::Property()->GetWinkInterval()) + 
    MoeUtils::Rand(MoeProperty::Property()->GetWinkInterval());
  BMessage msg(MOE_WINK);
  mWinkTrigger = new BMessageRunner(BMessenger(this),
				    &msg,
				    interval,
				    1);
}


void
MoeMascot::MouseDown(MoeWindowSlice *slice,
		     BPoint point)
{
  point = slice->ConvertToScreen(point);

  int32 buttons, modifiers;
  slice->Looper()->CurrentMessage()->FindInt32("buttons", &buttons);
  if (buttons & B_SECONDARY_MOUSE_BUTTON)
    {
      slice->Looper()->CurrentMessage()->FindInt32("modifiers", &modifiers);
      bool advanced = 
	(modifiers & B_SHIFT_KEY) &&
	(modifiers & B_CONTROL_KEY) &&
	(modifiers & B_COMMAND_KEY);
      MoeMascotMenu *menu = new MoeMascotMenu(this, advanced);
      menu->SetAsyncAutoDestruct(true);
      menu->Go(point, true, true, BRect(-5, -5, 5, 5).OffsetBySelf(point), true);
      return;
    }

  if (mIsXLocked && mIsYLocked)
    return;

  slice->SetMouseEventMask(B_POINTER_EVENTS);
  mOffset = point - this->Frame().LeftTop();
  mIsReadyToDrag = true;
  mPrevTargetFrame = mTargetFrame;
}


void
MoeMascot::MouseMoved(MoeWindowSlice *slice,
		      BPoint point, uint32 transit, const BMessage *msg)
{
  if (! mIsReadyToDrag)
    return;

  BPoint offset = slice->ConvertToScreen(point) - this->Frame().LeftTop();
  if (offset == mOffset)
    return;

  BMessage dummy(MOE_DUMMY_MESSAGE);
  slice->DragMessage(&dummy,
		     new BBitmap(this->Surface()->Bitmap()),
		     B_OP_ALPHA,
		     mOffset);

  this->PostMessage(uint32(MOE_WINDOW_REDRAW));

  mIsReadyToDrag = false;
  mIsDragging = true;
}


void
MoeMascot::MouseUp(MoeWindowSlice *slice,
		   BPoint point)
{
  mIsReadyToDrag = false;

  if (! mIsDragging)
    return;

  mIsDragging = false;

  point = slice->ConvertToScreen(point) - mOffset;

  BRect frame(this->Frame());
  MoeRectTransformer frameAccessor(&frame, mSide),
    targetAccessor(&mPrevTargetFrame, mSide);
  MoePointTransformer pointAccessor(&point, mSide);
  float width, xrate, yrate;

  if (mIsXLocked)
    xrate = mXRate;
  else
    {
      width = targetAccessor.Width() - frameAccessor.Width();
      if (width == 0)
	xrate = 0;
      else
	xrate = (pointAccessor.X() - targetAccessor.Left()) / width;
    }

  if (mIsYLocked)
    yrate = mYRate;
  else
    yrate = (targetAccessor.YSign() > 0) +
      targetAccessor.YSign() * (pointAccessor.Y() - targetAccessor.Top())
      / frameAccessor.Height();

  BMessage msg(MOE_RATE_MODIFIED);
  msg.AddPoint("data", BPoint(xrate, yrate));
  this->PostMessage(&msg);
}


void
MoeMascot::MessageReceived(BMessage *msg)
{
  switch (msg->what)
    {
    default:
      MoeWindow::MessageReceived(msg);
      break;

    case MOE_WINDOW_MODIFIED:
      {
	this->MoveToPrefered();
	break;
      }

    case MOE_RATE_MODIFIED:
      {
	BPoint data;
	msg->FindPoint("data", &data);
	this->SetXRate(data.x);
	this->SetYRate(data.y);
	this->MoveToPrefered();
	break;
      }

    case MOE_WINDOW_REDRAW:
      this->Redraw();
      break;

    case MOE_WINDOW_SHOW:
      if (this->IsHidden())
	this->Show();
      break;

    case B_WORKSPACE_ACTIVATED:
      this->Redraw();
      break;

    case MOE_SET_SIZE:
      {
	float data;
	msg->FindFloat("data", &data);
	this->SetSize(data);
	break;
      }

    case MOE_SET_SIDE:
      {
	int32 data;
	msg->FindInt32("data", &data);
	this->SetSide(data);
	break;
      }

    case MOE_LOCK_X:
      {
	bool data;
	msg->FindBool("data", &data);
	this->SetXLock(data);
	break;
      }

    case MOE_LOCK_Y:
      {
	bool data;
	msg->FindBool("data", &data);
	this->SetYLock(data);
	break;
      }

    case MOE_SET_PREF_APP_REQUESTED:
      {
	const char *label;

	msg->FindString("label", &label);

	BMessage *execMsg = new BMessage(*msg);
	execMsg->what = MOE_SET_PREF_APP;

	BInvoker *invoker = new BInvoker(execMsg, this);

	BAlert *alert = new BAlert("Set Prefered App",
				   label,
				   "Ok",
				   "Cancel",
				   NULL,
				   B_WIDTH_AS_USUAL,
				   B_WARNING_ALERT);
	alert->SetFlags(alert->Flags() | B_AVOID_FOCUS);

	alert->Go(invoker);

	break;
      }

    case MOE_SET_PREF_APP:
      {
	BString data;
	int32 which;

	msg->FindInt32("which", &which);
	if (which != 0)
	  break;

	msg->FindString("data", &data);
	this->SetPrefApp(data);
	break;
      }

    case MOE_SET_PREF_APP_WIN_REQUESTED:
      {
	const char *label;

	msg->FindString("label", &label);

	BMessage *execMsg = new BMessage(*msg);
	execMsg->what = MOE_SET_PREF_APP_WIN;

	BInvoker *invoker = new BInvoker(execMsg, this);

	BAlert *alert = new BAlert("Set Prefered Win",
				   label,
				   "Ok",
				   "Cancel",
				   NULL,
				   B_WIDTH_AS_USUAL,
				   B_WARNING_ALERT);
	alert->SetFlags(alert->Flags() | B_AVOID_FOCUS);

	alert->Go(invoker);

	break;
      }

    case MOE_SET_PREF_APP_WIN:
      {
	BString data;
	int32 which;

	msg->FindInt32("which", &which);
	if (which != 0)
	  break;

	msg->FindString("data", 0, &data);
	this->SetPrefApp(data);
	msg->FindString("data", 1, &data);
	this->SetPrefWin(data);
	break;
      }

    case MOE_WINK:
      this->Wink();
      break;

    case MOE_WINK_DONE:
      this->WinkDone();
      break;

    case B_NODE_MONITOR:
      this->PursuitNode(msg);
      break;
    }
}
