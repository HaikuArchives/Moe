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


#ifndef MOE_DEFS_H
#define MOE_DEFS_H


#define TRACKER_APP_SIGNATURE "application/x-vnd.Be-TRAK"
#define DESKBAR_APP_SIGNATURE "application/x-vnd.Be-TSKB"

#define MOE_APP_SIGNATURE "application/x-vnd.Kamnagi.Moe"
#define MOE_CONFIG_DIRECTORY "/boot/home/config/settings/Kamnagi/Moe/"
#define MOE_CONFIG_FILE MOE_CONFIG_DIRECTORY "property"

#define MOE_MAX_SLICE_NUM 30

#define MOE_ATTR_SIZE      "MOE:SIZE"
#define MOE_ATTR_SIDE      "MOE:SIDE"
#define MOE_ATTR_X_RATE    "MOE:X_RATE"
#define MOE_ATTR_Y_RATE    "MOE:Y_RATE"
#define MOE_ATTR_X_LOCKED  "MOE:X_LOCKED"
#define MOE_ATTR_Y_LOCKED  "MOE:Y_LOCKED"
#define MOE_ATTR_PREF_APP  "MOE:PREF_APP"
#define MOE_ATTR_PREF_WIN  "MOE:PREF_WIN"
#define MOE_ATTR_ANIME     "MOE:ANIME:"

enum
{
  MOE_DUMMY_MESSAGE               = 'Mdmy',
  MOE_EXAMINE_QUIT_REQUESTED      = 'Meqr',
  MOE_MASCOT_QUIT_REQUESTED       = 'Mror',
  MOE_MASCOT_REOPEN_REQUESTED     = 'Mmqr',
  MOE_WINDOW_MODIFIED             = 'Mwmd',
  MOE_WINDOW_SHOW                 = 'Mwsw',
  MOE_WINDOW_REDRAW               = 'Mwrd',
  MOE_RATE_MODIFIED               = 'Mrmd',
  MOE_SET_SIZE                    = 'Mssz',
  MOE_SET_SIDE                    = 'Mssd',
  MOE_SET_PREF_APP                = 'Mspa',
  MOE_SET_PREF_APP_REQUESTED      = 'Mpar',
  MOE_SET_PREF_APP_WIN            = 'Mspw',
  MOE_SET_PREF_APP_WIN_REQUESTED  = 'Mpwr',
  MOE_LOCK_X                      = 'Mlcx',
  MOE_LOCK_Y                      = 'Mlcy',
  MOE_SET_POLLING_INTERVAL        = 'Mspi',
  MOE_SET_REDRAW_INTERVAL         = 'Msri',
  MOE_SET_WINK_INTERVAL           = 'Mswi',
  MOE_SET_DEBUG_FRAME_VISIBLE     = 'Msdf',
  MOE_ADD_IGNORE_APP              = 'Maia',
  MOE_ADD_IGNORE_APP_REQUESTED    = 'Mair',
  MOE_REMOVE_IGNORE_APP           = 'Mria',
  MOE_REMOVE_IGNORE_APP_REQUESTED = 'Mrir',
  MOE_WINK                        = 'Mwnk',
  MOE_WINK_DONE                   = 'Mwkd',
};


#define MOE_FASTEST_POLLING_INTERVAL 10000
#define MOE_DEFAULT_WINK_INTERVAL    3000000

#endif

