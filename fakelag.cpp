#include "fakelag.h"
#include "Functions.h"
#include <windows.h>
#include "offsets.hpp"
#include <thread>

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

int lag_delay = 0;
uintptr_t localPlayer;
int shots_fired;

void FakeLag::setFakelag(int delay){
    lag_delay = delay * 15.625f;
}

void FakeLag::fakeLag(){
    while(true){
        if (GetKeyState(VK_HOME) & 1){ // check if fakelag is enabled
            localPlayer = Functions::getLocalPlayer();
            shots_fired = Memory.readMem<int>(localPlayer + m_iShotsFired);
            if (shots_fired > 0){  // set a check if player is shooting a gun
                Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 0); // disable packet sending
                std::this_thread::sleep_for(std::chrono::milliseconds(lag_delay)); // sleep of ticks
                Memory.writeMem<BYTE>(engineModule + dwbSendPackets, 1); // enable packet sending
                std::this_thread::sleep_for(std::chrono::milliseconds(3));

           }
        }
    }
}
