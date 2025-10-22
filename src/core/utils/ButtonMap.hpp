#pragma once

struct ButtonMap {
    bool W = false, A = false, S = false, D = false;
    bool Space = false;
    bool Up = false, Down = false, Left = false, Right = false;
    bool LeftCtrl = false;
    bool LeftShift = false;
    
    bool MousePointerActive = false;
    bool MouseLeft = false;
    bool MouseRight = false;
    float MousePosX = 0.0;
    float MousePosY = 0.0;
};