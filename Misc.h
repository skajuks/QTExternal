#pragma once

class _Bhop
{
public:
    static void initialize();
    static bool bhopEnabled();
    static void setBhopEnabled(bool state);

};

class Misc
{
public:
    static void changeFov(int fov);
};
