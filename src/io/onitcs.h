// OPENNI TRACKER Copyright (C) 2011 Robert Kooima
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.

#ifndef ONITCS_H
#define ONITCS_H

//------------------------------------------------------------------------------

struct onit_point
{
    float confidence;
    float world_p[3];
    float image_p[2];
};

typedef struct onit_point onit_point;

//------------------------------------------------------------------------------

int         onitcs_init(int, int, int);
void        onitcs_fini();

int         onitcs_naxes   ();
int         onitcs_nbuttons();
int         onitcs_npoints ();

float      *onitcs_acquire_axes   ();
int        *onitcs_acquire_buttons();
onit_point *onitcs_acquire_points ();

void        onitcs_release_axes   ();
void        onitcs_release_buttons();
void        onitcs_release_points ();

//------------------------------------------------------------------------------

#endif
