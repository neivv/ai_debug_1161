#include "mainpatch.h"

#include <time.h>
#include "console/windows_wrap.h"
#include "patch/patchmanager.h"

#include "offsets_hooks.h"
#include "offsets.h"
#include "memory.h"
#include "unit.h"
#include "text.h"
#include "scconsole.h"
#include "order.h"
#include "limits.h"
#include "draw.h"
#include "yms.h"

namespace bw
{
    namespace storm
    {
        intptr_t base_diff;
    }
}

void WindowCreatedPatch()
{
    Common::console->HookWndProc(*bw::main_window_hwnd);
    ScConsole::HookWndProc(*bw::main_window_hwnd);
}

void WinMainPatch()
{
    Common::PatchContext patch = patch_mgr->BeginPatch(0, bw::base::starcraft);
    PatchDraw(&patch);
    PatchConsole();
    Common::console->HookTranslateAccelerator(&patch, bw::base::starcraft);
}

void InitialPatch()
{
    InitSystemInfo();

    patch_mgr = new Common::PatchManager;
    InitBwFuncs_1161(patch_mgr, (uintptr_t)GetModuleHandle(NULL));
    InitStormFuncs_1161(patch_mgr, (uintptr_t)GetModuleHandle("storm"));
    Common::PatchContext patch = patch_mgr->BeginPatch(nullptr, bw::base::starcraft);

    patch.CallHook(bw::WinMain, WinMainPatch);
    patch.CallHook(bw::WindowCreated, WindowCreatedPatch);

    RemoveLimits(&patch);
}
