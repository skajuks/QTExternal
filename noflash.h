#ifndef NOFLASH_H
#define NOFLASH_H


class Flash{
public:
    static void setEnabledNoFlash(bool state);
    static void noflash();
    static void setNightmodeAmount(float amount);
};


#endif // NOFLASH_H
