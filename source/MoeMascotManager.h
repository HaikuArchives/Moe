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


#ifndef MOE_MASCOT_MANAGER_H
#define MOE_MASCOT_MANAGER_H


#include <Entry.h>
#include <List.h>


class MoeMascot;


class MoeMascotManager
{
public:
  MoeMascotManager(int32 cellSize = 8, int32 maxWindowNum = 30);
  virtual ~MoeMascotManager(void);

  int32 CountMascots(void);
  MoeMascot *MascotAt(int32 index);

  MoeMascot *Open(const entry_ref &entry);
  void Close(MoeMascot *mascot);
  void Reopen(MoeMascot *mascot);

private:
  void CellReduction(void);
  int32 SumSlices(void);

  int32 mCellSize;
  int32 mMaxWindowNum;
  BList mMascots;
};


#endif
