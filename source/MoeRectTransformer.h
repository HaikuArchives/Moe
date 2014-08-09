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


#ifndef MOE_RECT_TRANSFORMER_H
#define MOE_RECT_TRANSFORMER_H


#include <Rect.h>


enum
{
  MOE_LEFT_TOP_RIGHT_BOTTOM,
  MOE_RIGHT_TOP_LEFT_BOTTOM,
  MOE_LEFT_BOTTOM_RIGHT_TOP,
  MOE_RIGHT_BOTTOM_LEFT_TOP,
  MOE_TOP_LEFT_BOTTOM_RIGHT,
  MOE_TOP_RIGHT_BOTTOM_LEFT,
  MOE_BOTTOM_LEFT_TOP_RIGHT,
  MOE_BOTTOM_RIGHT_TOP_LEFT,
};


class MoeRectTransformer
{
public:
  MoeRectTransformer(BRect *target,
		  int32 transform = MOE_LEFT_TOP_RIGHT_BOTTOM);
  
  void SetTarget(BRect *target);
  void SetTransform(int32 transform);

  float &Left(void);
  float &Top(void);
  float &Right(void);
  float &Bottom(void);

  float Width(void);
  float Height(void);
  int XSign(void);
  int YSign(void);

private:
  BRect *mTarget;
  int32 mTransform;
};


class MoePointTransformer
{
public:
  MoePointTransformer(BPoint *target,
		      int32 transform = MOE_LEFT_TOP_RIGHT_BOTTOM);
  
  void SetTarget(BPoint *target);
  void SetTransform(int32 transform);

  float &X(void);
  float &Y(void);

private:
  BPoint *mTarget;
  int32 mTransform;
};

#endif
