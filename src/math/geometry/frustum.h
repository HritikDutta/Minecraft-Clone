#pragma once

#include "plane.h"

union Frustum
{
    Plane planes[6];
    
    struct
    {
        Plane top, bottom;
        Plane left, right;
        Plane near, far;
    };

    Frustum() {}
};