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


#include <cmath>
#include <String.h>
#include <Roster.h>
#include <Application.h>
#include <MenuItem.h>
#include <Message.h>
#include <Catalog.h>
#include "MoeDefs.h"
#include "MoeProperty.h"
#include "MoeActiveWindowWatcher.h"
#include "MoeRectTransformer.h"
#include "MoeMascot.h"
#include "MoeMascotMenu.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MascotMenu"

static const char *
sSpeed[5] =
{
  B_TRANSLATE("Fastest"),
  B_TRANSLATE("Fast"),
  B_TRANSLATE("Slow"),
  B_TRANSLATE("Very Slow"),
  B_TRANSLATE("Slowest"),
};


static const struct
{
  const char *label;
  int32 side;
  const char *lock_x;
  const char *lock_y;
}
sSide[4] =
{
  { B_TRANSLATE("left"),   MOE_TOP_LEFT_BOTTOM_RIGHT, B_TRANSLATE("Lock Y"), B_TRANSLATE("Lock X") },
  { B_TRANSLATE("top"),    MOE_LEFT_TOP_RIGHT_BOTTOM, B_TRANSLATE("Lock X"), B_TRANSLATE("Lock Y") },
  { B_TRANSLATE("right"),  MOE_TOP_RIGHT_BOTTOM_LEFT, B_TRANSLATE("Lock Y"), B_TRANSLATE("Lock X") },
  { B_TRANSLATE("bottom"), MOE_LEFT_BOTTOM_RIGHT_TOP, B_TRANSLATE("Lock X"), B_TRANSLATE("Lock Y") },
};


MoeMascotMenu::MoeMascotMenu(MoeMascot *target, bool advanced)
  : BPopUpMenu("mascot menu", false, false)
{
  this->InitItems(target, advanced);
}


void
MoeMascotMenu::InitItems(MoeMascot *target, bool advanced)
{
  MoeProperty *property;
  MoeActiveWindowWatcher *watcher;
  entry_ref entry;
  int32 i;
  bigtime_t interval;
  BMenu *menu;
  BMenuItem *item;
  BMessage *msg;

  property = MoeProperty::Property();
  watcher = MoeActiveWindowWatcher::Watcher();

  // add mascot lock setting items.
  for (i = 0; i < 4; i++)
    if (target->Side() == sSide[i].side)
      break;
  msg = new BMessage(MOE_LOCK_X);
  msg->AddBool("data", ! target->IsXLocked());
  item = new BMenuItem(sSide[i].lock_x, msg);
  item->SetMarked(target->IsXLocked());
  item->SetTarget(target);
  this->AddItem(item);

  if (! advanced && target->IsYLocked())
    item->SetLabel(B_TRANSLATE_COMMENT("Lock", "Text when Y is locked."));
  else
    {
      msg = new BMessage(MOE_LOCK_Y);
      msg->AddBool("data", ! target->IsYLocked());
      item = new BMenuItem(sSide[i].lock_y, msg);
      item->SetMarked(target->IsYLocked());
      item->SetTarget(target);
      this->AddItem(item);
    }

  // add size menu.
  menu = new BMenu(B_TRANSLATE("Size"));
  for (i = 0; i > -4; i--)
    {
      float size = ::pow(2.0, i);
      BString buf;
      msg = new BMessage(MOE_SET_SIZE);
      msg->AddFloat("data", size);
      buf << int(100.0 * size) << "%";
      item = new BMenuItem(buf.String(), msg);
      item->SetMarked(target->Size() == size);
      item->SetTarget(target);
      menu->AddItem(item);
    }
  item = new BMenuItem(menu);
  this->AddItem(item);

  // add side menu.
  if (advanced)
    {
      menu = new BMenu(B_TRANSLATE("Side"));
      for (i = 0; i < 4; i++)
	{
	  msg = new BMessage(MOE_SET_SIDE);
	  msg->AddInt32("data", sSide[i].side);
	  item = new BMenuItem(sSide[i].label, msg);
	  item->SetMarked(target->Side() == sSide[i].side);
	  item->SetTarget(target);
	  menu->AddItem(item);
	}
      item = new BMenuItem(menu);
      this->AddItem(item);
    }

  // add prefered app/win menu.
  if (advanced)
    {
      watcher->Lock();

      menu = new BMenu(B_TRANSLATE("Show only on"));

      if (target->PrefWin().Length() == 0)
	{
	  if (target->PrefApp().Length() == 0)
	    {
	      BString label;
	      //label << "Show Only on \"" << watcher->CurrentAppName() << "\"?";
	      label << B_TRANSLATE("Show only on \"%curappname%\"?");
	      label.ReplaceAll("%curappname%", watcher->CurrentAppName());
	      msg = new BMessage(MOE_SET_PREF_APP_REQUESTED);
	      msg->AddString("data", watcher->CurrentApp());
	      msg->AddString("label", label);
	      item = new BMenuItem(B_TRANSLATE("This application"), msg);
	    }
	  else
	    {
	      msg = new BMessage(MOE_SET_PREF_APP);
	      msg->AddString("data", "");
	      item = new BMenuItem(B_TRANSLATE("This application"), msg);
	      item->SetMarked(true);
	    }
	  item->SetTarget(target);
	  menu->AddItem(item);
	}

      if (target->PrefWin().Length() == 0)
	{
	  BString label;
	  //label << "Show Only on \"" << watcher->CurrentWin() 
		//<< "\" of \"" << watcher->CurrentAppName() << "\"?";
	  label << B_TRANSLATE("Show only on \"%curwin%\" of \"%curappname%\"?");
	  label.ReplaceAll("%curwin%", watcher->CurrentWin());
	  label.ReplaceAll("%curappname%", watcher->CurrentAppName());
	  msg = new BMessage(MOE_SET_PREF_APP_WIN_REQUESTED);
	  msg->AddString("data", watcher->CurrentApp());
	  msg->AddString("data", watcher->CurrentWin());
	  msg->AddString("label", label);
	  item = new BMenuItem(B_TRANSLATE("This window of this application"), msg);
	}
      else
	{
	  msg = new BMessage(MOE_SET_PREF_APP_WIN);
	  msg->AddString("data", "");
	  msg->AddString("data", "");
	  item = new BMenuItem(B_TRANSLATE("This window of this application"), msg);
	  item->SetMarked(true);
	}
      item->SetTarget(target);
      menu->AddItem(item);

      watcher->Unlock();
      this->AddItem(menu);
    }

  this->AddSeparatorItem();

  // add redraw interval setting menu.
  menu = new BMenu(B_TRANSLATE("Wink"));
  for (i = 0; i < 5; i++)
    {
      BString buf;
      buf << i + 1 << "s";
      interval = (i + 1) * 1000000;
      msg = new BMessage(MOE_SET_WINK_INTERVAL);
      msg->AddInt64("data", interval);
      item = new BMenuItem(buf.String(), msg);
      item->SetMarked(property->GetWinkInterval() == interval);
      item->SetTarget(property);
      menu->AddItem(item);
    }
  item = new BMenuItem(menu);
  this->AddItem(item);

  menu = new BMenu(B_TRANSLATE("Polling"));
  for (i = 0, interval = MOE_FASTEST_POLLING_INTERVAL;
       i < 5;
       i++, interval *= 2)
    {
      msg = new BMessage(MOE_SET_POLLING_INTERVAL);
      msg->AddInt64("data", interval);
      item = new BMenuItem(sSpeed[i], msg);
      item->SetMarked(property->GetPollingInterval() == interval);
      item->SetTarget(property);
      menu->AddItem(item);
    }
  item = new BMenuItem(menu);
  this->AddItem(item);

  menu = new BMenu(B_TRANSLATE("Redraw"));
  for (i = 0, interval = MOE_FASTEST_POLLING_INTERVAL * 2;
       i < 5;
       i++, interval *= 2)
    {
      msg = new BMessage(MOE_SET_REDRAW_INTERVAL);
      msg->AddInt64("data", interval);
      item = new BMenuItem(sSpeed[i], msg);
      item->SetMarked(property->GetRedrawInterval() == interval);
      item->SetEnabled(property->GetPollingInterval() * 2 <= interval);
      item->SetTarget(property);
      menu->AddItem(item);
    }
  item = new BMenuItem(menu);
  this->AddItem(item);

  this->AddSeparatorItem();

  // add ignore app items.
  msg = new BMessage(MOE_ADD_IGNORE_APP_REQUESTED);
  watcher->Lock();
  msg->AddString("name", watcher->CurrentAppName());
  msg->AddString("signature", watcher->CurrentApp());
  watcher->Unlock();
  item = new BMenuItem(B_TRANSLATE("Ignore this app"), msg);
  item->SetTarget(property);
  this->AddItem(item);

  if (advanced)
    {
      BMenu *menu;
      const char *name, *signature;

      property->Lock();

      menu = new BMenu(B_TRANSLATE("Remove from ignore list"));

      for (i = 1; i < property->CountIgnoreApps(); i++)
	{
	  signature = property->IgnoreAppAt(i);
	  if (be_roster->FindApp(signature, &entry) == B_OK)
	    name = entry.name;
	  else
	    name = signature;
	  msg = new BMessage(MOE_REMOVE_IGNORE_APP_REQUESTED);
	  msg->AddString("name", name);
	  msg->AddString("signature", signature);
	  item = new BMenuItem(name, msg);
	  item->SetTarget(property);
	  menu->AddItem(item);
	}

      property->Unlock();

      this->AddItem(new BMenuItem(menu));
    }
  
  this->AddSeparatorItem();

  // add debug item.
  if (advanced)
    {
      msg = new BMessage(MOE_SET_DEBUG_FRAME_VISIBLE);
      msg->AddBool("data", ! property->IsDebugFrameVisible());
      item = new BMenuItem(B_TRANSLATE("Debug frame visible"), msg);
      item->SetMarked(property->IsDebugFrameVisible());
      item->SetTarget(property);
      this->AddItem(item);

      this->AddSeparatorItem();
    }

  // add close item.
  msg = new BMessage(MOE_MASCOT_QUIT_REQUESTED);
  msg->AddPointer("data", target);
  item = new BMenuItem(B_TRANSLATE("Close"), msg);
  item->SetTarget(be_app);
  this->AddItem(item);

  this->AddSeparatorItem();

  // add about item.
  msg = new BMessage(B_ABOUT_REQUESTED);
  item = new BMenuItem(B_TRANSLATE("About"), msg);
  item->SetTarget(be_app);
  this->AddItem(item);

  this->AddSeparatorItem();

  // add quit item.
  msg = new BMessage(B_QUIT_REQUESTED);
  item = new BMenuItem(B_TRANSLATE("Quit"), msg);
  item->SetTarget(be_app);
  this->AddItem(item);
}

