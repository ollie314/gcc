// 2003-02-24 Petur Runolfsson <peturr02 at ru dot is>

// Copyright (C) 2003-2016 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include <ostream>
#include <streambuf>
#include <testsuite_hooks.h>

// libstdc++/9827
class Buf : public std::streambuf
{
};

void test01()
{
  using namespace std;

  Buf buf;
  ostream stream(&buf);

  stream << 1;
  VERIFY(!stream.good());
}

int main()
{
  test01();
  return 0;
}
