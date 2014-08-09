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


#ifndef MOE_UTILS_H
#define MOE_UTILS_H


#include <Bitmap.h>


class BList;


class MoeUtils
{
public:
  static void TransparentLeftTop(BBitmap *bitmap);
  static void MapBitmap(BBitmap *dist, const BBitmap *src, 
			const BRect *srcRect = NULL);
  static int32 Rand(int32 max);
  static float Rand(void);
};


static inline bool
operator==(const rgb_color c1, const rgb_color c2)
{
  return *reinterpret_cast<const uint32*>(&c1) ==
    *reinterpret_cast<const uint32*>(&c2);
}


static inline bool
operator!=(const rgb_color c1, const rgb_color c2)
{
  return *reinterpret_cast<const uint32*>(&c1) !=
    *reinterpret_cast<const uint32*>(&c2);
}


static inline rgb_color *
pixel_at(const BBitmap *bitmap, int32 x, int32 y)
{
  return reinterpret_cast<rgb_color*>(bitmap->Bits()) +
    bitmap->BytesPerRow() / sizeof(rgb_color) * y + x;
}


#endif
