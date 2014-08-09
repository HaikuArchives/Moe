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


#include <cstdio>
#include <cstring>
#include <algorithm>
#include <Bitmap.h>
#include <View.h>
#include "MoeUtils.h"
#include "MoeSurface.h"


static bool
is_blank(const BBitmap *bitmap, BRect rect)
{
  float x, y;
  BRect bounds;
  
  bounds = bitmap->Bounds();
  
  for (y = rect.top; y <= rect.bottom; y++)
    {
      const rgb_color *pixel = 
	reinterpret_cast<const rgb_color*>(bitmap->Bits()) +
	bitmap->BytesPerRow() / sizeof(rgb_color) * int(y) + int(rect.left);

      for (x = rect.left; x <= rect.right; x++)
	if (*pixel++ != B_TRANSPARENT_32_BIT)
	  return false;
    }

  return true;
}


static BRect
auto_crop(const BBitmap *bitmap, BRect rect)
{
  // crop left side
  while (rect.left <= rect.right &&
	 is_blank(bitmap, BRect(rect.left, rect.top, rect.left, rect.bottom)))
    rect.left ++;

  // crop right side
  while (rect.left <= rect.right &&
	 is_blank(bitmap, BRect(rect.right, rect.top, rect.right, rect.bottom)))
    rect.right --;

  // crop top side
  while (rect.top <= rect.bottom &&
	 is_blank(bitmap, BRect(rect.left, rect.top, rect.right, rect.top)))
    rect.top ++;

  // crop bottom side
  while (rect.top <= rect.bottom &&
	 is_blank(bitmap, BRect(rect.left, rect.bottom, rect.right, rect.bottom)))
    rect.bottom --;

  return rect;
}


MoeSurface::MoeSurface(BBitmap *bitmap, int32 cellSize)
  : mBitmap(bitmap)
  , mCellSize(cellSize)
{
  MoeUtils::TransparentLeftTop(bitmap);
  this->ParseBitmap();
}


void
MoeSurface::ParseBitmap(void)
{
  BRect bounds;
  BRegion region;
  float x, y;
  int32 i;

  bounds = ::auto_crop(mBitmap, mBitmap->Bounds());

  for (y = bounds.top; y <= bounds.bottom; y += mCellSize)
    for (x = bounds.left; x <= bounds.right; x += mCellSize)
      {
	BRect rect(x, y, x + mCellSize - 1, y + mCellSize - 1);
	rect = rect & bounds;

	if (::is_blank(mBitmap, rect))
	  continue;

	region.Include(rect);
      }

  mRegion.MakeEmpty();

  for (i = 0; i < region.CountRects(); i++)
    mRegion.Include(::auto_crop(mBitmap, region.RectAt(i)));

  mRegion.Include(&mExtraRegion);
}


int32
MoeSurface::CountRects(void)
{
  return mRegion.CountRects();
}


BRect
MoeSurface::RectAt(int32 index)
{
  return mRegion.RectAt(index);
}


const BBitmap *
MoeSurface::Bitmap(void) const
{
  return mBitmap;
}


BRect
MoeSurface::Bounds(void) const
{
  return mBitmap->Bounds();
}


void
MoeSurface::SetCellSize(int32 cellSize)
{
  mCellSize = cellSize;
  this->ParseBitmap();
}


void
MoeSurface::IncludeExtraRect(BRect rect)
{
  mExtraRegion.Include(rect);
  mRegion.Include(rect);
}


