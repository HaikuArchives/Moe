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


#ifndef MOE_CONSOLE_H
#define MOE_CONSOLE_H


#include <Window.h>


class BTextView;


class MoeConsole : public BWindow
{
public:
  static int Printf(const char *fmt, ...);

private:
  MoeConsole(void);
  virtual ~MoeConsole(void);

  void Print(const char *text);

  virtual void DispatchMessage(BMessage *msg, BHandler *handler);

  BTextView *mConsole;
};


#endif
