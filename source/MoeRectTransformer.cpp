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
#include "MoeRectTransformer.h"


MoeRectTransformer::MoeRectTransformer(BRect *target, int32 transform)
  : mTarget(target)
  , mTransform(transform)
{
}


void
MoeRectTransformer::SetTarget(BRect *target)
{
  mTarget = target;
}


void
MoeRectTransformer::SetTransform(int32 transform)
{
  mTransform = transform;
}


float &
MoeRectTransformer::Left(void)
{
  switch (mTransform)
    {
    default:
    case MOE_LEFT_TOP_RIGHT_BOTTOM:
    case MOE_LEFT_BOTTOM_RIGHT_TOP:
      return mTarget->left;

    case MOE_TOP_LEFT_BOTTOM_RIGHT:
    case MOE_TOP_RIGHT_BOTTOM_LEFT:
      return mTarget->top;

    case MOE_RIGHT_TOP_LEFT_BOTTOM:
    case MOE_RIGHT_BOTTOM_LEFT_TOP:
      return mTarget->right;

    case MOE_BOTTOM_LEFT_TOP_RIGHT:
    case MOE_BOTTOM_RIGHT_TOP_LEFT:
      return mTarget->bottom;
    }
}


float &
MoeRectTransformer::Top(void)
{
  switch (mTransform)
    {
    default:
    case MOE_TOP_LEFT_BOTTOM_RIGHT:
    case MOE_BOTTOM_LEFT_TOP_RIGHT:
      return mTarget->left;

    case MOE_LEFT_TOP_RIGHT_BOTTOM:
    case MOE_RIGHT_TOP_LEFT_BOTTOM:
      return mTarget->top;

    case MOE_TOP_RIGHT_BOTTOM_LEFT:
    case MOE_BOTTOM_RIGHT_TOP_LEFT:
      return mTarget->right;

    case MOE_LEFT_BOTTOM_RIGHT_TOP:
    case MOE_RIGHT_BOTTOM_LEFT_TOP:
      return mTarget->bottom;
    }
}


float &
MoeRectTransformer::Right(void)
{
  switch (mTransform)
    {
    default:
    case MOE_RIGHT_TOP_LEFT_BOTTOM:
    case MOE_RIGHT_BOTTOM_LEFT_TOP:
      return mTarget->left;

    case MOE_BOTTOM_LEFT_TOP_RIGHT:
    case MOE_BOTTOM_RIGHT_TOP_LEFT:
      return mTarget->top;

    case MOE_LEFT_TOP_RIGHT_BOTTOM:
    case MOE_LEFT_BOTTOM_RIGHT_TOP:
      return mTarget->right;

    case MOE_TOP_LEFT_BOTTOM_RIGHT:
    case MOE_TOP_RIGHT_BOTTOM_LEFT:
      return mTarget->bottom;
    }
}


float &
MoeRectTransformer::Bottom(void)
{
  switch(mTransform)
    {
    default:
    case MOE_TOP_RIGHT_BOTTOM_LEFT:
    case MOE_BOTTOM_RIGHT_TOP_LEFT:
      return mTarget->left;

    case MOE_LEFT_BOTTOM_RIGHT_TOP:
    case MOE_RIGHT_BOTTOM_LEFT_TOP:
      return mTarget->top;

    case MOE_TOP_LEFT_BOTTOM_RIGHT:
    case MOE_BOTTOM_LEFT_TOP_RIGHT:
      return mTarget->right;

    case MOE_LEFT_TOP_RIGHT_BOTTOM:
    case MOE_RIGHT_TOP_LEFT_BOTTOM:
      return mTarget->bottom;
    }
}


float
MoeRectTransformer::Width(void)
{
  return ::fabs(this->Right() - this->Left());
}


float
MoeRectTransformer::Height(void)
{
  return ::fabs(this->Bottom() - this->Top());
}


int
MoeRectTransformer::XSign(void)
{
  float left, right;
  left = this->Left();
  right = this->Right();
  if (left < right)
    return 1;
  if (left > right)
    return -1;
  return 0;
}


int
MoeRectTransformer::YSign(void)
{
  float top, bottom;
  top = this->Top();
  bottom = this->Bottom();
  if (top < bottom)
    return 1;
  if (top > bottom)
    return -1;
  return 0;
}


MoePointTransformer::MoePointTransformer(BPoint *target, int32 transform)
  : mTarget(target)
  , mTransform(transform)
{
}


void
MoePointTransformer::SetTarget(BPoint *target)
{
  mTarget = target;
}


void
MoePointTransformer::SetTransform(int32 transform)
{
  mTransform = transform;
}


float &
MoePointTransformer::X(void)
{
  switch (mTransform)
    {
    default:
    case MOE_LEFT_BOTTOM_RIGHT_TOP:
    case MOE_RIGHT_BOTTOM_LEFT_TOP:
    case MOE_LEFT_TOP_RIGHT_BOTTOM:
    case MOE_RIGHT_TOP_LEFT_BOTTOM:
      return mTarget->x;

    case MOE_TOP_RIGHT_BOTTOM_LEFT:
    case MOE_BOTTOM_RIGHT_TOP_LEFT:
    case MOE_TOP_LEFT_BOTTOM_RIGHT:
    case MOE_BOTTOM_LEFT_TOP_RIGHT:
      return mTarget->y;
    }
}


float &
MoePointTransformer::Y(void)
{
  switch (mTransform)
    {
    default:
    case MOE_TOP_RIGHT_BOTTOM_LEFT:
    case MOE_BOTTOM_RIGHT_TOP_LEFT:
    case MOE_TOP_LEFT_BOTTOM_RIGHT:
    case MOE_BOTTOM_LEFT_TOP_RIGHT:
      return mTarget->x;

    case MOE_LEFT_BOTTOM_RIGHT_TOP:
    case MOE_RIGHT_BOTTOM_LEFT_TOP:
    case MOE_LEFT_TOP_RIGHT_BOTTOM:
    case MOE_RIGHT_TOP_LEFT_BOTTOM:
      return mTarget->y;
    }
}





