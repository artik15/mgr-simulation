/*
 * ssdi - ssdi.h
 *
 * Copyright (C) 2016..2020  Krzysztof Mazur <krzysiek@podlesie.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef SSDI_H_INCLUDED
#define SSDI_H_INCLUDED

#define CONTROL_POS             0x1
#define CONTROL_NEG             0x2

unsigned int ssdim(int v);
unsigned int ssdiv(int v);

#endif