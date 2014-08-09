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


#include <cstdlib>
#include <View.h>
#include "MoeUtils.h"


static inline rgb_color
mix_color(const rgb_color *color, uint32 n)
{
  uint32 i, red, green, blue, alpha;
  rgb_color result;

  for  (i = red = green = blue = alpha = 0; i < n; i++)
    {
      red   += color[i].red;
      blue  += color[i].blue;
      green += color[i].green;
      alpha += color[i].alpha;
    }
  
  result.red   = red / n;
  result.blue  = blue / n;
  result.green = green / n;
  result.alpha = alpha / n;

  return result;
}


void
MoeUtils::TransparentLeftTop(BBitmap *bitmap)
{
  rgb_color leftTopColor, *color, *colorTop;
  uint32 i, n;

  colorTop = reinterpret_cast<rgb_color*>(bitmap->Bits());
  leftTopColor = *colorTop;
  n = bitmap->BitsLength() / sizeof(rgb_color);

  // avoid BMP Translator BUG (inconstancy alpha)
  for (i = 1, color = colorTop + 1; i < n; i++, color++)
    if (color->alpha != leftTopColor.alpha)
      break;

  if (i == n)
    {
      leftTopColor.alpha = 255;
      for (i = 0, color = colorTop; i < n; i++, color++)
	color->alpha = 255;
    }

  // fill transparent pixels.
  for (i = 0, color = colorTop; i < n; i++, color++)
    if (color->alpha == 0 || *color == leftTopColor)
      *color = B_TRANSPARENT_32_BIT;
}


void
MoeUtils::MapBitmap(BBitmap *dist, const BBitmap *src, 
		    const BRect *srcRect)
{
  BBitmap *buf;
  BView *bufView;
  int x, y;
  rgb_color c[4];

  buf = new BBitmap(BRect(0, 0,
			  (dist->Bounds().right + 1) * 2 - 1,
			  (dist->Bounds().bottom + 1) * 2 - 1),
		    B_RGB32, true);
  
  bufView = new BView(buf->Bounds(), "", 0, B_WILL_DRAW);
  buf->Lock();
  buf->AddChild(bufView);
  if (srcRect)
    bufView->DrawBitmap(src, *srcRect, buf->Bounds());
  else
    bufView->DrawBitmap(src, src->Bounds(), buf->Bounds());
  bufView->Sync();
  buf->Unlock();

  for (x = 0; x <= dist->Bounds().right; x++)
    for (y = 0; y <= dist->Bounds().bottom; y++)
      {
	c[0] = *pixel_at(buf, x * 2    , y * 2    );
	c[1] = *pixel_at(buf, x * 2 + 1, y * 2    );
	c[2] = *pixel_at(buf, x * 2    , y * 2 + 1);
	c[3] = *pixel_at(buf, x * 2 + 1, y * 2 + 1);

	*pixel_at(dist, x, y) = mix_color(c, 4);
      }

  delete buf;
}


float
MoeUtils::Rand(void)
{
  static bool isInitiarized;

  if (! isInitiarized)
    {
      ::srand(::system_time());
      isInitiarized = true;
    }
  
  return float(::rand()) / RAND_MAX;
}


int32
MoeUtils::Rand(int32 max)
{
  return int(Rand() * max);
}



