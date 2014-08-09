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
#include <String.h>
#include <Autolock.h>
#include <Locker.h>
#include <File.h>
#include <Directory.h>
#include <Invoker.h>
#include <Alert.h>
#include "MoeDefs.h"
#include "MoeWindow.h"
#include "MoeProperty.h"


#define IGNORE_APP       "ignore_app"
#define POLLING_INTERVAL "polling_interval"
#define REDRAW_INTERVAL  "redraw_interval"
#define WINK_INTERVAL    "wink_interval"
#define DEBUG_FRAME_VISIBLE "debug_frame_visible"


MoeProperty *
MoeProperty::Property(void)
{
  static BLocker locker;
  static MoeProperty *property;
  BAutolock lock(locker);

  if (! property)
    {
      property = new MoeProperty();
      property->Run();
    }

  return property;
}


MoeProperty::MoeProperty(void)
  : BLooper("property")
{
  this->Read();
  this->InitContainer();
}


void
MoeProperty::InitContainer(void)
{
  type_code type;
  int32 count;
  int64 dummy64;

  if (mContainer.GetInfo(IGNORE_APP, &type, &count) != B_OK)
    {
      mContainer.AddString(IGNORE_APP, MOE_APP_SIGNATURE);
      mContainer.AddString(IGNORE_APP, DESKBAR_APP_SIGNATURE);
    }

  if (mContainer.FindInt64(POLLING_INTERVAL, &mPollingInterval) != B_OK)
    {
      mPollingInterval = MOE_FASTEST_POLLING_INTERVAL * 2;
      mContainer.AddInt64(POLLING_INTERVAL, mPollingInterval);
    }

  if (mContainer.FindInt64(REDRAW_INTERVAL, &mRedrawInterval) != B_OK)
    {
      mRedrawInterval = mPollingInterval * 2;
      mContainer.AddInt64(REDRAW_INTERVAL, mRedrawInterval);
    }

  if (mContainer.FindInt64(WINK_INTERVAL, &dummy64) != B_OK)
    {
      mContainer.AddInt64(WINK_INTERVAL, MOE_DEFAULT_WINK_INTERVAL);
    }

  if (mContainer.FindBool(DEBUG_FRAME_VISIBLE, &mIsDebugFrameVisible) != B_OK)
    {
      mIsDebugFrameVisible = false;
      mContainer.AddBool(DEBUG_FRAME_VISIBLE, mIsDebugFrameVisible);
    }
  MoeWindow::SetDebugFrameVisible(mIsDebugFrameVisible);
}


void
MoeProperty::Read(void)
{
  BAutolock lock(this);
  BFile file;

  if (file.SetTo(MOE_CONFIG_FILE, B_READ_ONLY) == B_OK)
    mContainer.Unflatten(&file);
}


void
MoeProperty::Write(void)
{
  BAutolock lock(this);
  BFile file;

  ::create_directory(MOE_CONFIG_DIRECTORY, 0755);

  if (file.SetTo(MOE_CONFIG_FILE, B_WRITE_ONLY | B_CREATE_FILE) == B_OK)
    mContainer.Flatten(&file);
}


void
MoeProperty::AddIgnoreApp(const char *signature)
{
  BAutolock lock(this);

  if (! this->IsIgnoreApp(signature))
    {
      mContainer.AddString(IGNORE_APP, signature);
      this->Write();
    }
}


void
MoeProperty::RemoveIgnoreApp(const char *signature)
{
  BAutolock lock(this);
  const char *ignore_app;
  int32 i;

  for (i = 0; mContainer.FindString(IGNORE_APP, i, &ignore_app) == B_OK; i++)
    if (strcmp(signature, ignore_app) == 0)
      {
	mContainer.RemoveData(IGNORE_APP, i);
	this->Write();
	return;
      }
}


bool
MoeProperty::IsIgnoreApp(const char *signature)
{
  BAutolock lock(this);
  const char *ignore_app;
  int32 i;

  for (i = 0; mContainer.FindString(IGNORE_APP, i, &ignore_app) == B_OK; i++)
    if (strcmp(signature, ignore_app) == 0)
      return true;

  return false;
}


int32
MoeProperty::CountIgnoreApps(void)
{
  BAutolock lock(this);
  type_code type;
  int32 count;

  mContainer.GetInfo(IGNORE_APP, &type, &count);

  return count;
}


const char *
MoeProperty::IgnoreAppAt(int32 index)
{
  BAutolock lock(this);
  const char *result;

  mContainer.FindString(IGNORE_APP, index, &result);

  return result;
}


void 
MoeProperty::SetPollingInterval(bigtime_t interval)
{
  BAutolock lock(this);

  mPollingInterval = interval;
  mContainer.ReplaceInt64(POLLING_INTERVAL, interval);

  if (mRedrawInterval < mPollingInterval * 2)
    this->SetRedrawInterval(mPollingInterval * 2);
  else
    this->Write();
}


bigtime_t
MoeProperty::GetPollingInterval(void)
{
  return mPollingInterval;
}


void 
MoeProperty::SetRedrawInterval(bigtime_t interval)
{
  BAutolock lock(this);

  mRedrawInterval = interval;
  mContainer.ReplaceInt64(REDRAW_INTERVAL, interval);
  this->Write();
}


bigtime_t
MoeProperty::GetRedrawInterval(void)
{
  return mRedrawInterval;
}


void
MoeProperty::SetWinkInterval(bigtime_t interval)
{
  BAutolock lock(this);

  mContainer.ReplaceInt64(WINK_INTERVAL, interval);
  this->Write();
}


bigtime_t
MoeProperty::GetWinkInterval(void)
{
  BAutolock lock(this);
  bigtime_t interval;

  mContainer.FindInt64(WINK_INTERVAL, &interval);

  return interval;
}


void 
MoeProperty::SetDebugFrameVisible(bool visible)
{
  BAutolock lock(this);

  mIsDebugFrameVisible = visible;
  mContainer.ReplaceBool(DEBUG_FRAME_VISIBLE, visible);
  this->Write();

  MoeWindow::SetDebugFrameVisible(mIsDebugFrameVisible);
}


bool
MoeProperty::IsDebugFrameVisible(void)
{
  return mIsDebugFrameVisible;
}


void
MoeProperty::MessageReceived(BMessage *msg)
{
  switch (msg->what)
    {
    default:
      BLooper::MessageReceived(msg);
      break;

    case MOE_SET_POLLING_INTERVAL:
      {
	bigtime_t interval;

	msg->FindInt64("data", &interval);
	this->SetPollingInterval(interval);

	break;
      }

    case MOE_SET_REDRAW_INTERVAL:
      {
	bigtime_t interval;

	msg->FindInt64("data", &interval);
	this->SetRedrawInterval(interval);

	break;
      }

    case MOE_SET_WINK_INTERVAL:
      {
	bigtime_t interval;

	msg->FindInt64("data", &interval);
	this->SetWinkInterval(interval);

	break;
      }

    case MOE_SET_DEBUG_FRAME_VISIBLE:
      {
	bool visible;

	msg->FindBool("data", &visible);
	this->SetDebugFrameVisible(visible);

	break;
      }

    case MOE_ADD_IGNORE_APP_REQUESTED:
      {
	const char *name;
	BString buf;

	msg->FindString("name", &name);
	buf << "Really Ignore \"" << name << "\"?";

	BMessage *execMsg = new BMessage(*msg);
	execMsg->what = MOE_ADD_IGNORE_APP;

	BInvoker *invoker = new BInvoker(execMsg, this);

	BAlert *alert = new BAlert("Ignore This App",
				   buf.String(),
				   "Ok",
				   "Cancel",
				   NULL,
				   B_WIDTH_AS_USUAL,
				   B_WARNING_ALERT);
	alert->SetFlags(alert->Flags() | B_AVOID_FOCUS);

	alert->Go(invoker);

	break;
      }

    case MOE_ADD_IGNORE_APP:
      {
	const char *signature;
	int32 which;

	msg->FindInt32("which", &which);
	if (which != 0)
	  break;

	msg->FindString("signature", &signature);
	this->AddIgnoreApp(signature);

	break;
      }

    case MOE_REMOVE_IGNORE_APP_REQUESTED:
      {
	const char *name;
	BString buf;

	msg->FindString("name", &name);
	buf << "Avoid \"" << name << "\" from Ignore List?";

	BMessage *execMsg = new BMessage(*msg);
	execMsg->what = MOE_REMOVE_IGNORE_APP;

	BInvoker *invoker = new BInvoker(execMsg, this);

	BAlert *alert = new BAlert("Ignore This App",
				   buf.String(),
				   "Ok",
				   "Cancel",
				   NULL,
				   B_WIDTH_AS_USUAL,
				   B_WARNING_ALERT);
	alert->SetFlags(alert->Flags() | B_AVOID_FOCUS);

	alert->Go(invoker);

	break;
      }

    case MOE_REMOVE_IGNORE_APP:
      {
	const char *signature;
	int32 which;

	msg->FindInt32("which", &which);
	if (which != 0)
	  break;

	msg->FindString("signature", &signature);
	this->RemoveIgnoreApp(signature);

	break;
      }
    }
}


