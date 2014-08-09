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


#ifndef MOE_WINDOW_H
#define MOE_WINDOW_H


#include <List.h>
#include <Rect.h>
#include <Looper.h>


class MoeSurface;
class MoeWindowSlice;
class MoeAnime;
class BMessage;


class MoeWindow : public BLooper
{
public:
  static void SetDebugFrameVisible(bool visible);
  static bool IsDebugFrameVisible(void);

  friend class MoeWindowSlice;
  friend class MoeAnime;

  MoeWindow(const char *name);
  virtual ~MoeWindow(void);

  virtual void Quit(void);

  void MoveTo(float x, float y);
  void MoveTo(BPoint point);
	
  void Show(void);
  void Hide(void);
  bool IsHidden(void) const;

  void Sync(void);

  void AttachSurface(MoeSurface *surface);

  void SetCellSize(int32 cellSize);

  BRect Frame(void) const;

  int32 CountSlices(void) const;
  MoeWindowSlice *SliceAt(int32 index);

  MoeSurface *Surface(void);

  void Invalidate(BRect udpateRect);

  status_t LockBitmap(void);
  void UnlockBitmap(void);
  const BBitmap *Bitmap(void) const;

  void AddAnime(MoeAnime *anime);
  int32 CountAnimes(void);
  MoeAnime *AnimeAt(int32 index);
  void FindAnimesMatch(BList *result, const char *prefixMatch);

protected:
  virtual void MouseDown(MoeWindowSlice *slice, 
			 BPoint point);
  virtual void MouseUp(MoeWindowSlice *slice,
		       BPoint point);
  virtual void MouseMoved(MoeWindowSlice *slice,
			  BPoint point, uint32 transit, const BMessage *msg);

  virtual void MessageReceived(BMessage *msg);

private:
  void AllocSlices(void);

  void ComposeBitmap(bool captureScreen = true);

  MoeSurface *mSurface;
  BBitmap *mBackground, *mBitmap;
  BView *mOffscreenView;
  BList mSlices;
  BPoint mPoint;
  BRect mClipping;
  int32 mShowCount;
};


#endif
