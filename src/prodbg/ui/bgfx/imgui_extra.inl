
#include "ui_sc_editor.h"

namespace ImGui
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FillRect(ImVec2 pos, ImVec2 size, unsigned int color)
{
	ImGuiWindow* window = GetCurrentWindow();
	ImVec2 currentPos = window->Pos + pos;
    window->DrawList->AddRectFilled(currentPos, currentPos + size, color, 0.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float GetTextWidth(const char* textStart, const char* textEnd)
{
	ImVec2 size = CalcTextSize(textStart, textEnd);
	return size.x; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImVec2 GetRelativeMousePos()
{
    const ImGuiState* g = GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    ImVec2 pos = g->IO.MousePos - window->Pos;
    ImVec2 zero = ImVec2(0.0f, 0.0f);
    return ImClamp(pos, zero, window->Size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsFocusWindowKeyDown(int key, bool repeat)
{
	if (!GetWindowIsFocused())
		return false;

    //ImGuiState& g = GImGui;
    //ImGuiWindow* window = GetCurrentWindow();

    // Only send keyboard events to selected window

    //if (g.FocusedWindow != window)
     //   return false;

    return IsKeyPressed(key, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GetWindowRect(ImGuiWindow* window, ImVec2* pos, ImVec2* size)
{
    *pos = window->Pos;
    *size = window->Size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetWindowRect(ImGuiWindow* window, const ImVec2 pos, const ImVec2 size)
{
    window->PosFloat = pos;
    window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);
    window->Size = size;
    window->SizeFull = size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsActiveWindow(ImGuiWindow* window)
{
    const ImGuiState* g = GImGui;
    return g->FocusedWindow == window;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImScEditor* ScInputText(const char* label, float xSize, float ySize, void (*callback)(void*), void* userData)
{
    ImGuiState& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return 0;

    (void)callback;
    (void)userData;

    const ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);
    //const float w = window->DC.ItemWidth.back();

    const ImVec2 text_size = CalcTextSize(label, NULL, true);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + window->Size); 
    const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(text_size.x > 0.0f ? (style.ItemInnerSpacing.x + text_size.x) : 0.0f, 0.0f));

    ItemSize(bb);

    if (!ItemAdd(frame_bb, &id))
        return 0;

	ImGuiStorage* storage = GetStateStorage();
	ScEditor* editor = (ScEditor*)storage->GetVoidPtr(id);

	if (!editor)
	{
		(void)xSize;
		(void)ySize;
		editor = ScEditor_create((int)frame_bb.Max.x, (int)frame_bb.Max.y);
		storage->SetVoidPtr(id, (void*)editor);
	}

    // NB: we are only allowed to access 'edit_state' if we are the active widget.
    //ImGuiTextEditState& edit_state = g.InputTextState;

    const bool hovered = IsHovered(frame_bb, id);

    if (hovered)
        g.HoveredId = id;

    if (hovered && io.MouseClicked[0])
    {
        if (g.ActiveId != id)
        {
            // Start edition

        }

        g.ActiveId = id;
        FocusWindow(window);
    }
    else if (io.MouseClicked[0])
    {
        // Release focus when we click outside
        if (g.ActiveId == id)
        {
            g.ActiveId = 0;
        }
    }

    ScEditor_setDrawList(GetWindowDrawList());
    ScEditor_setFont(GetWindowFont());
	ScEditor_setPos(frame_bb.Min.x, frame_bb.Min.y);

	ScEditor_resize(editor, 0, 0, (int)(frame_bb.Max.x - frame_bb.Min.x) , (int)(frame_bb.Max.y - frame_bb.Min.y));
	
	return ScEditor_getInterface(editor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}

