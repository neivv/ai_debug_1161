#include "limits.h"

#include "patch/patchmanager.h"
#include "draw.h"
#include "offsets_hooks.h"
#include "offsets.h"

Common::PatchManager *patch_mgr;

void ResetGameSpeedWaits()
{
    for (int i = 0; i < 7; i++)
    {
        bw::wait_times_msec[i] = 42;
    }
}

void PatchDraw(Common::PatchContext *patch)
{
    patch->Hook(bw::SDrawLockSurface, SDrawLockSurface_Hook);
    patch->Hook(bw::SDrawUnlockSurface, SDrawUnlockSurface_Hook);

    patch->Hook(bw::DrawScreen, DrawScreen);
}

void RemoveLimits(Common::PatchContext *patch)
{
    patch->CallHook(bw::WaitTimesSet, ResetGameSpeedWaits);
    if (UseConsole)
    {
        patch->Hook(bw::GenerateFog, GenerateFog);
    }
}
