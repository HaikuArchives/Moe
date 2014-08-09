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


#ifndef MOE_COMPRESIION_UTILS_H
#define MOE_COMPRESIION_UTILS_H


#include <OS.h>
#include <zlib.h>


class BDataIO;


class MoeCompressionUtils
{
public:
  static status_t Compress(BDataIO *in, BDataIO *out,
			   int level = Z_DEFAULT_COMPRESSION); 

  static status_t Decompress(BDataIO *in, BDataIO *out);
};


#endif
