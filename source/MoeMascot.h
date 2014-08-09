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


#ifndef MOE_MASCOT_H
#define MOE_MASCOT_H


#include <String.h>
#include <Entry.h>
#include "MoeWindow.h"


class BMessageRunner;


class MoeMascot : public MoeWindow
{
public:
  friend class MoeMascotManager;

  float Size(void) const;
  int32 Side(void) const;
  bool IsXLocked(void) const;
  bool IsYLocked(void) const;
  const BString &PrefApp(void) const;
  const BString &PrefWin(void) const;

protected:
  virtual void MouseDown(MoeWindowSlice *slice, 
			 BPoint point);
  virtual void MouseUp(MoeWindowSlice *slice,
		       BPoint point);
  virtual void MouseMoved(MoeWindowSlice *slice,
			  BPoint point, uint32 transit, const BMessage *msg);

  virtual void MessageReceived(BMessage *msg);

private:
  MoeMascot(const entry_ref &ref, MoeSurface *surface);
  virtual ~MoeMascot(void);

  void Redraw(void);
  void MoveToPrefered(void);

  void InitAttr(void);
  void WriteAttr(const char *name, type_code type,
		 const void *buffer, size_t length);
  void SetSize(float size);
  void SetSide(int32 side);
  void SetXLock(bool data);
  void SetYLock(bool data);
  void SetXRate(float data);
  void SetYRate(float data);
  void SetPrefApp(const BString &data);
  void SetPrefWin(const BString &data);

  void StartNodeMonitoring(void);
  void PursuitNode(const BMessage *msg);
  void EndNodeMonitoring(void);

  void Wink(void);
  void WinkDone(void);

  entry_ref mEntry;
  float mSize;
  int32 mSide;
  float mXRate, mYRate;
  bool mIsXLocked, mIsYLocked;
  BString mPrefApp, mPrefWin;
  BRect mTargetFrame, mPrevTargetFrame;
  BPoint mOffset;
  bool mIsReadyToDrag, mIsDragging;
  BMessageRunner *mTrigger;
  BMessageRunner *mWinkTrigger;
};


#endif
