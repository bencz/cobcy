// This file is part of cobcy, a COBOL-to-C compiler.
//
// Copyright (C) 1995-2008 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include <curses.h>
#include "../coblib/cobfunc.h"

void ViewBin (FILE* df);
void DisplayOpen (void);
void DisplayClose (void);
