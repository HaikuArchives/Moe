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


#ifndef MOE_SUFFACE_H
#define MOE_SURFACE_H

#include <SupportDefs.h>
#include <Rect.h>
#include <Region.h>


class BView;
class BBitmap;


class MoeSurface
{
public:
  MoeSurface(BBitmap *bitmap, int32 cellSize = 8);

  int32 CountRects(void);
  BRect RectAt(int32 index);
  const BBitmap *Bitmap(void) const;
  BRect Bounds(void) const;

  void SetCellSize(int32 cellSize);
  void IncludeExtraRect(BRect rect);

private:
  void ParseBitmap(void);

  BBitmap *mBitmap;
  int32 mCellSize;
  BRegion mRegion, mExtraRegion;
};


#endif
