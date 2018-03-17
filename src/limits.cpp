#include "limits.h"

#include "patch/patchmanager.h"
#include "draw.h"
#include "offsets_hooks.h"
#include "offsets.h"

Common::PatchManager *patch_mgr;

void PatchDraw(Common::PatchContext *patch)
{
    patch->Hook(bw::SDrawLockSurface, SDrawLockSurface_Hook);
    patch->Hook(bw::SDrawUnlockSurface, SDrawUnlockSurface_Hook);

    patch->Hook(bw::DrawScreen, DrawScreen);
}

void RemoveLimits(Common::PatchContext *patch)
{
    if (UseConsole)
    {
        patch->Hook(bw::GenerateFog, GenerateFog);
    }
}
