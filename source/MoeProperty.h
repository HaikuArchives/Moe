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


#ifndef MOE_PROPERTY_H
#define MOE_PROPERTY_H


#include <Message.h>
#include <Looper.h>


class MoeProperty : public BLooper
{
private:
  MoeProperty(void);

public:
  static MoeProperty *Property(void);

  void Read(void);
  void Write(void);

  void AddIgnoreApp(const char *signature);
  void RemoveIgnoreApp(const char *signature);
  int32 CountIgnoreApps(void);
  const char *IgnoreAppAt(int32 index);
  bool IsIgnoreApp(const char *signature);

  void SetPollingInterval(bigtime_t interval);
  bigtime_t GetPollingInterval(void);

  void SetRedrawInterval(bigtime_t interval);
  bigtime_t GetRedrawInterval(void);

  void SetWinkInterval(bigtime_t interval);
  bigtime_t GetWinkInterval(void);

  void SetDebugFrameVisible(bool visible);
  bool IsDebugFrameVisible(void);

protected:
  virtual void MessageReceived(BMessage *msg);

private:
  void InitContainer(void);

  BMessage mContainer;
  bigtime_t mPollingInterval, mRedrawInterval;
  bool mIsDebugFrameVisible;
};


#endif
