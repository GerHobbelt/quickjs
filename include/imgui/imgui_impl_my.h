#if !defined(IMGUI_IMPL_MY)
#define IMGUI_IMPL_MY

#include "imgui.h"

struct ImGui_ImplMy_Data
{
    bool                    InstalledCallbacks;
    ImGui_ImplMy_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

#endif // IMGUI_IMPL_MY
