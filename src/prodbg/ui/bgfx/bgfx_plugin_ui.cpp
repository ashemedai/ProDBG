#include "bgfx_plugin_ui.h"
#include "pd_ui.h"
#include "pd_view.h"
#include "api/plugin_instance.h"
#include "core/plugin_handler.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/math.h"
#include "core/input_state.h"
#include "core/plugin_io.h"
#include "core/service.h"
#include "ui_dock.h"
#include "ui_host.h"
#include "imgui_setup.h"
#include <imgui.h>
#include <assert.h>

#include <session/session.h>
#include <foundation/apple.h>
#include <foundation/string.h>
#include <bgfx.h>
#include "core/input_state.h"
#include "ui/bgfx/cursor.h"
#include <foundation/string.h>
#include "i3wm_docking.h"
#include "ui_render.h"
#include <jansson.h>

#ifdef _WIN32
#include <Windows.h>
#include <bgfxplatform.h>
#endif

struct ImGuiWindow;

// TODO: Move to settings
const int s_borderSize = 4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Context
{
    int width;
    int height;
    //InputState inputState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Context s_context;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivateData
{
    ImGuiWindow* window;
    const char* name;
    const char* title;
    bool showWindow;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImVec4 pdColorToImVec4(uint32_t color)
{
	float r = ((color >> 24) & 0xff) * 1.0f / 255.0f;
	float g = ((color >> 16) & 0xff) * 1.0f / 255.0f;
	float b = ((color >> 8) & 0xff) * 1.0f / 255.0f;
	float a = ((color >> 0) & 0xff) * 1.0f / 255.0f;
	return ImVec4(r, g, b, a);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSCFuncs
{
    intptr_t (*sendCommand)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
    void (*update)(void* privData);
    void (*draw)(void* privData);
    void* privateData;
} PDSCFuns;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static intptr_t scSendCommand(void* privData, unsigned int message, uintptr_t p0, intptr_t p1)
{
    ImScEditor* editor = (ImScEditor*)privData;
    return editor->SendCommand(message, p0, p1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void scUpdate(void* privData)
{
    ImScEditor* editor = (ImScEditor*)privData;
    editor->Draw();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void scDraw(void* privData)
{
    ImScEditor* editor = (ImScEditor*)privData;
    return editor->Update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setTitle(void* privateData, const char* title)
{
	PrivateData* data = (PrivateData*)privateData;

	(void)data;

	if (string_equal(data->title, title))
		return;

	if (data->title)
		free((void*)data->title);

	data->title = strdup(title); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getWindowSize()
{
    ImVec2 size = ImGui::GetWindowSize();
    PDVec2 r = { size.x, size.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getWindowPos()
{
    ImVec2 pos = ImGui::GetWindowPos();
    PDVec2 r = { pos.x, pos.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void beginChild(const char* stringId, PDVec2 size, bool border, int extraFlags)
{
    ImGui::BeginChild(stringId, ImVec2(size.x, size.y), border, ImGuiWindowFlags(extraFlags));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endChild()
{
    ImGui::EndChild();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getScrollY()
{
	return ImGui::GetScrollY();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getScrollMaxY()
{
	return ImGui::GetScrollMaxY();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setScrollY(float scrollY)
{
	ImGui::SetScrollY(scrollY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setScrollHere(float centerYratio)
{
	ImGui::SetScrollHere(centerYratio);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setScrollFromPosY(float posY, float centerYratio)
{
	ImGui::SetScrollFromPosY(posY, centerYratio);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setKeyboardFocusHere(int offset)
{
	ImGui::SetKeyboardFocusHere(offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushFont(PDUIFont font)
{
	ImGui::PushFont((ImFont*)font);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popFont()
{
	ImGui::PopFont();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushStyleColor(PDUICol idx, PDColor col)
{
	ImGui::PushStyleColor(idx, pdColorToImVec4(col));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popStyleColor(int count)
{
	ImGui::PopStyleVar(count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushStyleVar(PDUIStyleVar idx, float val)
{
	ImGui::PushStyleVar(ImGuiStyleVar(idx), val);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushStyleVarVec(PDUIStyleVar idx, PDVec2 val)
{
	ImGui::PushStyleVar(ImGuiStyleVar(idx), ImVec2(val.x, val.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popStyleVar(int count)
{
	ImGui::PopStyleVar(count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushItemWidth(float itemWidth)
{
    ImGui::PushItemWidth(itemWidth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popItemWidth()
{
    ImGui::PopItemWidth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float calcItemWidth()
{
	return ImGui::CalcItemWidth();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushAllowKeyboardFocus(bool v)
{
	ImGui::PushAllowKeyboardFocus(v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popAllowKeyboardFocus()
{
	ImGui::PopAllowKeyboardFocus();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushTextWrapPos(float wrapPosX)
{
	ImGui::PushTextWrapPos(wrapPosX);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popTextWrapPos()
{
	ImGui::PopTextWrapPos();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushButtonRepeat(bool repeat)
{
	ImGui::PushButtonRepeat(repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popButtonRepeat()
{
	ImGui::PopButtonRepeat();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void beginGroup()
{
	ImGui::BeginGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endGroup()
{
	ImGui::EndGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void separator()
{
	ImGui::Separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sameLine(int columnX, int spacingW)
{
    ImGui::SameLine(columnX, spacingW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void spacing()
{
    ImGui::Spacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummy(PDVec2 size)
{
	ImGui::Dummy(ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void indent()
{
	ImGui::Indent();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void unIndent()
{
	ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void columns(int count, const char* id, bool border)
{
    ImGui::Columns(count, id, border);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void nextColumn()
{
    ImGui::NextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getColumnIndex()
{
	return ImGui::GetColumnIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getColumnOffset(int columnIndex)
{
    return ImGui::GetColumnOffset(columnIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setColumnOffset(int columnIndex, float offset)
{
    return ImGui::SetColumnOffset(columnIndex, offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getColumnWidth(int columnIndex)
{
    return ImGui::GetColumnWidth(columnIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getColumnsCount()
{
	return ImGui::GetColumnsCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getCursorPos()
{
    ImVec2 t = ImGui::GetCursorPos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getCursorPosX()
{
	return ImGui::GetCursorPosX();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getCursorPosY()
{
	return ImGui::GetCursorPosY();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorPos(PDVec2 pos)
{
    ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorPosX(float x)
{
    ImGui::SetCursorPosX(x);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorPosY(float y)
{
    ImGui::SetCursorPosY(y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getCursorScreenPos()
{
    ImVec2 t = ImGui::GetCursorScreenPos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorScreenPos(PDVec2 pos)
{
	ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void alignFirstTextHeightToWidgets()
{
    ImGui::AlignFirstTextHeightToWidgets();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getTextLineHeight()
{
	return ImGui::GetTextLineHeight();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getTextLineHeightWithSpacing()
{
    return ImGui::GetTextLineHeightWithSpacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getItemsLineHeightWithSpacing()
{
	return ImGui::GetItemsLineHeightWithSpacing();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushIdStr(const char* strId)
{
	ImGui::PushID(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushIdStrRange(const char* strBegin, const char* strEnd)
{
	ImGui::PushID(strBegin, strEnd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushIdPtr(const void* ptrId)
{
	ImGui::PushID(ptrId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void pushIdInt(const int intId)
{
	ImGui::PushID(intId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void popId()
{
	ImGui::PopID();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDID getIdStr(const char* strId)
{
	return (PDID)ImGui::GetID(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDID getIdStrRange(const char* strBegin, const char* strEnd)
{
	return (PDID)ImGui::GetID(strBegin, strEnd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDID getIdPtr(const void* ptrId)
{
	return (PDID)ImGui::GetID(ptrId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void text(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    ImGui::TextV(format, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textV(const char* fmt, va_list args)
{
    ImGui::TextV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textColored(const PDColor col, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::TextColoredV(pdColorToImVec4(col), fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textColoredV(const PDColor col, const char* fmt, va_list args)
{
    ImGui::TextColoredV(pdColorToImVec4(col), fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textDisabled(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::TextDisabledV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textDisabledV(const char* fmt, va_list args)
{
    ImGui::TextDisabledV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textWrapped(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::TextWrappedV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textWrappedV(const char* fmt, va_list args)
{
    ImGui::TextWrappedV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void textUnformatted(const char* text, const char* text_end)
{
    ImGui::TextUnformatted(text, text_end);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void labelText(const char* label, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::LabelTextV(label, fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void labelTextV(const char* label, const char* fmt, va_list args)
{
    ImGui::LabelTextV(label, fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void bullet()
{
    ImGui::Bullet();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void bulletText(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::BulletTextV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void bulletTextV(const char* fmt, va_list args)
{
    ImGui::BulletTextV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool button(const char* label, const PDVec2 size)
{
    return ImGui::Button(label, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool smallButton(const char* label)
{
    return ImGui::SmallButton(label);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool invisibleButton(const char* strId, const PDVec2 size)
{
    return ImGui::InvisibleButton(strId, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void image(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, const PDColor tintColor, const PDColor borderColor)
{
    ImGui::Image((ImTextureID)user_texture_id, ImVec2(size.x, size.y), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y), pdColorToImVec4(tintColor), pdColorToImVec4(borderColor));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool imageButton(PDUITextureID user_texture_id, const PDVec2 size, const PDVec2 uv0, const PDVec2 uv1, int framePadding, const PDColor bgColor, const PDColor tintCol)
{
    return ImGui::ImageButton(user_texture_id, ImVec2(size.x, size.y), ImVec2(uv0.x, uv1.y), ImVec2(uv1.x, uv1.y), framePadding, pdColorToImVec4(bgColor), pdColorToImVec4(tintCol));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool collapsingHeader(const char* label, const char* strId, bool displayFrame, bool defaultOpen)
{
    return ImGui::CollapsingHeader(label, strId, displayFrame, defaultOpen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool checkbox(const char* label, bool* v)
{
    return ImGui::Checkbox(label, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool checkboxFlags(const char* label, unsigned int* flags, unsigned int flagsValue)
{
    return ImGui::CheckboxFlags(label, flags, flagsValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool radioButtonBool(const char* label, bool active)
{
    return ImGui::RadioButton(label, active);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool radioButton(const char* label, int* v, int v_button)
{
    return ImGui::RadioButton(label, v, v_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool combo(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems)
{
    return ImGui::Combo(label, currentItem, items, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool combo2(const char* label, int* currentItem, const char* itemsSeparatedByZeros, int heightInItems)
{
    return ImGui::Combo(label, currentItem, itemsSeparatedByZeros, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool combo3(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems)
{
    return ImGui::Combo(label, currentItem, itemsGetter, data, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool colorButton(const PDColor col, bool smallHeight, bool outlineBorder)
{
    return ImGui::ColorButton(pdColorToImVec4(col), smallHeight, outlineBorder);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool colorEdit3(const char* label, float col[3])
{
    return ImGui::ColorEdit3(label, col);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool colorEdit4(const char* label, float col[4], bool showAlpha)
{
    return ImGui::ColorEdit4(label, col, showAlpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void colorEditMode(PDUIColorEditMode mode)
{
    ImGui::ColorEditMode(mode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plotLines(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride)
{
    ImGui::PlotLines(label, values, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y), (int)stride);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plotLines2(const char* label, float (*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize)
{
	ImGui::PlotLines(label, valuesGetter, data, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y)); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plotHistogram(const char* label, const float* values, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize, size_t stride)
{
    ImGui::PlotHistogram(label, values, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y), (int)stride);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void plotHistogram2(const char* label, float (*valuesGetter)(void* data, int idx), void* data, int valuesCount, int valuesOffset, const char* overlayText, float scaleMin, float scaleMax, PDVec2 graphSize)
{
    ImGui::PlotHistogram(label, valuesGetter, data, valuesCount, valuesOffset, overlayText, scaleMin, scaleMax, ImVec2(graphSize.x, graphSize.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUISCInterface* scInputText(const char* label, float xSize, float ySize, void (*callback)(void*), void* userData)
{
    ImScEditor* ed = ImGui::ScInputText(label, xSize, ySize, callback, userData);

    if (!ed->userData)
    {
        PDUISCInterface* funcs = (PDUISCInterface*)malloc(sizeof(PDUISCInterface));
        funcs->sendCommand = scSendCommand;
        funcs->update = scUpdate;
        funcs->draw = scDraw;
        funcs->privateData = ed;

        ed->userData = (void*)funcs;
    }

    return (PDUISCInterface*)ed->userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderFloat(const char* label, float* v, float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::SliderFloat(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderFloat2(const char* label, float v[2], float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::SliderFloat2(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderFloat3(const char* label, float v[3], float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::SliderFloat3(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderFloat4(const char* label, float v[4], float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::SliderFloat4(label, v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderAngle(const char* label, float* v_rad, float vDegreesMin, float vDegreesMax)
{
    return ImGui::SliderAngle(label, v_rad, vDegreesMin, vDegreesMax);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderInt(const char* label, int* v, int vMin, int vMax, const char* displayFormat)
{
    return ImGui::SliderInt(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderInt2(const char* label, int v[2], int vMin, int vMax, const char* displayFormat)
{
    return ImGui::SliderInt2(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderInt3(const char* label, int v[3], int vMin, int vMax, const char* displayFormat)
{
    return ImGui::SliderInt3(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sliderInt4(const char* label, int v[4], int vMin, int vMax, const char* displayFormat)
{
    return ImGui::SliderInt4(label, v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vsliderFloat(const char* label, const PDVec2 size, float* v, float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::VSliderFloat(label, ImVec2(size.x, size.y), v, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vsliderInt(const char* label, const PDVec2 size, int* v, int vMin, int vMax, const char* displayFormat)
{
    return ImGui::VSliderInt(label, ImVec2(size.x, size.y), v, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragFloat(const char* label, float* v, float vSpeed, float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::DragFloat(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragFloat2(const char* label, float v[2], float vSpeed, float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::DragFloat2(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragFloat3(const char* label, float v[3], float vSpeed, float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::DragFloat3(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragFloat4(const char* label, float v[4], float vSpeed, float vMin, float vMax, const char* displayFormat, float power)
{
    return ImGui::DragFloat4(label, v, vSpeed, vMin, vMax, displayFormat, power);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragInt(const char* label, int* v, float vSpeed, int vMin, int vMax, const char* displayFormat)
{
    return ImGui::DragInt(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragInt2(const char* label, int v[2], float vSpeed, int vMin, int vMax, const char* displayFormat)
{
    return ImGui::DragInt2(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragInt3(const char* label, int v[3], float vSpeed, int vMin, int vMax, const char* displayFormat)
{
    return ImGui::DragInt3(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool dragInt4(const char* label, int v[4], float vSpeed, int vMin, int vMax, const char* displayFormat)
{
    return ImGui::DragInt4(label, v, vSpeed, vMin, vMax, displayFormat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*InputCallback)(PDUIInputTextCallbackData*);

struct PDInputTextUserData
{
    InputCallback callback;
    void* userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void inputTextDeleteChars(PDUIInputTextCallbackData* data, int pos, int byteCount)
{
    char* dst = data->buf + pos;
    const char* src = data->buf + pos + byteCount;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    data->bufDirty = true;
    if (data->cursorPos + byteCount >= pos)
        data->cursorPos -= byteCount;
    else if (data->cursorPos >= pos)
        data->cursorPos = pos;
    data->selectionStart = data->selectionEnd = data->cursorPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void inputTextInsertChars(PDUIInputTextCallbackData* data, int pos, const char* text, const char* textEnd = NULL)
{
    const int textLen = int(strlen(data->buf));
    if (!textEnd)
        textEnd = text + strlen(text);

    const int newTextLen = (int)(textEnd - text);

    if (newTextLen + textLen + 1 >= data->bufSize)
        return;

    size_t upos = (size_t)pos;
    if ((size_t)textLen != upos)
        memmove(data->buf + upos + newTextLen, data->buf + upos, (size_t)textLen - upos);
    memcpy(data->buf + upos, text, (size_t)newTextLen * sizeof(char));
    data->buf[textLen + newTextLen] = '\0';

    data->bufDirty = true;
    if (data->cursorPos >= pos)
        data->cursorPos += (int)newTextLen;
    data->selectionStart = data->selectionEnd = data->cursorPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int textEditCallbackStub(ImGuiTextEditCallbackData* data)
{
    PDInputTextUserData* wrappedUserData = (PDInputTextUserData*)data->UserData;
    PDUIInputTextCallbackData callbackData = { 0 };

    // Transfer over ImGui callback data into our generic wrapper version
    callbackData.userData = wrappedUserData->userData;
    callbackData.buf = data->Buf;
    callbackData.bufSize = int(data->BufSize);
    callbackData.bufDirty = data->BufDirty;
    callbackData.flags = PDUIInputTextFlags(data->Flags);
    callbackData.cursorPos = data->CursorPos;
    callbackData.selectionStart = data->SelectionStart;
    callbackData.selectionEnd  = data->SelectionEnd;
    callbackData.deleteChars = inputTextDeleteChars;
    callbackData.insertChars = inputTextInsertChars;

    // Translate ImGui event key into our own PDKey mapping
    ImGuiIO& io = ImGui::GetIO();
    callbackData.eventKey = io.KeyMap[data->EventKey];

    // Invoke the callback (synchronous)
    wrappedUserData->callback(&callbackData);

    // We need to mirror any changes to the callback wrapper into the actual ImGui version
    data->UserData = callbackData.userData;
    data->Buf = callbackData.buf;
    data->BufSize = (int)callbackData.bufSize;
    data->BufDirty = callbackData.bufDirty;
    data->Flags = ImGuiInputTextFlags(callbackData.flags);
    data->CursorPos = callbackData.cursorPos;
    data->SelectionStart = callbackData.selectionStart;
    data->SelectionEnd   = callbackData.selectionEnd;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputText(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDUIInputTextCallbackData*), void* userData)
{
    PDInputTextUserData wrappedUserData;
    wrappedUserData.callback = callback;
    wrappedUserData.userData = userData;
    return ImGui::InputText(label, buf, (size_t)buf_size, ImGuiInputTextFlags(flags), &textEditCallbackStub, &wrappedUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputTextMultiline(const char* label, char* buf, size_t buf_size, const PDVec2 size, PDUIInputTextFlags flags, void (*callback)(PDUIInputTextCallbackData*), void* userData)
{
    PDInputTextUserData wrappedUserData;
    wrappedUserData.callback = callback;
    wrappedUserData.userData = userData;
    return ImGui::InputTextMultiline(label, buf, buf_size, ImVec2(size.x, size.y), flags, &textEditCallbackStub, &wrappedUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputFloat(const char* label, float* v, float step, float step_fast, int decimal_precision, PDUIInputTextFlags extraFlags)
{
    return ImGui::InputFloat(label, v, step, step_fast, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputFloat2(const char* label, float v[2], int decimal_precision, PDUIInputTextFlags extraFlags)
{
    return ImGui::InputFloat2(label, v, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputFloat3(const char* label, float v[3], int decimal_precision, PDUIInputTextFlags extraFlags)
{
    return ImGui::InputFloat3(label, v, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputFloat4(const char* label, float v[4], int decimal_precision, PDUIInputTextFlags extraFlags)
{
    return ImGui::InputFloat4(label, v, decimal_precision, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputInt(const char* label, int* v, int step, int step_fast, PDUIInputTextFlags extraFlags)
{
    return ImGui::InputInt(label, v, step, step_fast, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputInt2(const char* label, int v[2], PDUIInputTextFlags extraFlags)
{
    return ImGui::InputInt2(label, v, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputInt3(const char* label, int v[3], PDUIInputTextFlags extraFlags)
{
    return ImGui::InputInt3(label, v, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool inputInt4(const char* label, int v[4], PDUIInputTextFlags extraFlags)
{
    return ImGui::InputInt4(label, v, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool treeNode(const char* str_label_id)
{
    return ImGui::TreeNode(str_label_id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool treeNodeStr(const char* strId, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    bool ret = ImGui::TreeNodeV(strId, fmt, ap);

    va_end(ap);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool treeNodePtr(const void* ptrId, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    bool ret = ImGui::TreeNodeV(ptrId, fmt, ap);

    va_end(ap);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool treeNodeStrV(const char* strId, const char* fmt, va_list args)
{
    return ImGui::TreeNodeV(strId, fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool treeNodePtrV(const void* ptrId, const char* fmt, va_list args)
{
    return ImGui::TreeNodeV(ptrId, fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void treePushStr(const char* strId)
{
    ImGui::TreePush(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void treePushPtr(const void* ptrId)
{
    ImGui::TreePush(ptrId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void treePop()
{
    ImGui::TreePop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setNextTreeNodeOpened(bool opened, PDUISetCond cond)
{
    ImGui::SetNextTreeNodeOpened(opened, cond);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool selectable(const char* label, bool selected, PDUISelectableFlags flags, const PDVec2 size)
{
    return ImGui::Selectable(label, selected, flags, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool selectableEx(const char* label, bool* p_selected, PDUISelectableFlags flags, const PDVec2 size)
{
    return ImGui::Selectable(label, p_selected, flags, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool listBox(const char* label, int* currentItem, const char** items, int itemsCount, int heightInItems)
{
    return ImGui::ListBox(label, currentItem, items, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool listBox2(const char* label, int* currentItem, bool(*itemsGetter)(void* data, int idx, const char** out_text), void* data, int itemsCount, int heightInItems)
{
	return ImGui::ListBox(label, currentItem, itemsGetter, data, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool listBoxHeader(const char* label, const PDVec2 size)
{
    return ImGui::ListBoxHeader(label, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool listBoxHeader2(const char* label, int itemsCount, int heightInItems)
{
    return ImGui::ListBoxHeader(label, itemsCount, heightInItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void listBoxFooter()
{
    ImGui::ListBoxFooter();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setTooltip(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::SetTooltipV(fmt, ap);

    va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setTooltipV(const char* fmt, va_list args)
{
    ImGui::SetTooltipV(fmt, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void beginTooltip()
{
    ImGui::BeginTooltip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endTooltip()
{
    ImGui::EndTooltip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginMainMenuBar()
{
    return ImGui::BeginMainMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endMainMenuBar()
{
    ImGui::EndMainMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginMenuBar()
{
    return ImGui::BeginMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endMenuBar()
{
    ImGui::EndMenuBar();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginMenu(const char* label, bool enabled)
{
    return ImGui::BeginMenu(label, enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endMenu()
{
    ImGui::EndMenu();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool menuItem(const char* label, const char* shortcut, bool selected, bool enabled)
{
    return ImGui::MenuItem(label, shortcut, selected, enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool menuItemPtr(const char* label, const char* shortcut, bool* p_selected, bool enabled)
{
    return ImGui::MenuItem(label, shortcut, p_selected, enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void openPopup(const char* strId)
{
    ImGui::OpenPopup(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginPopup(const char* strId)
{
    return ImGui::BeginPopup(strId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginPopupModal(const char* name, bool* p_opened, PDUIWindowFlags extraFlags)
{
    return ImGui::BeginPopupModal(name, p_opened, extraFlags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginPopupContextItem(const char* strId, int mouse_button)
{
    return ImGui::BeginPopupContextItem(strId, mouse_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginPopupContextWindow(bool also_over_items, const char* strId, int mouse_button)
{
    return ImGui::BeginPopupContextWindow(also_over_items, strId, mouse_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginPopupContextVoid(const char* strId, int mouse_button)
{
    return ImGui::BeginPopupContextVoid(strId, mouse_button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endPopup()
{
    ImGui::EndPopup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void closeCurrentPopup()
{
    ImGui::CloseCurrentPopup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void valueBool(const char* prefix, bool b)
{
    ImGui::Value(prefix, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void valueInt(const char* prefix, int v)
{
    ImGui::Value(prefix, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void valueUInt(const char* prefix, unsigned int v)
{
    ImGui::Value(prefix, v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void valueFloat(const char* prefix, float v, const char* float_format)
{
    ImGui::Value(prefix, v, float_format);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void color(const char* prefix, const PDColor col)
{
    ImGui::Color(prefix, pdColorToImVec4(col));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void logToTTY(int maxDepth)
{
    ImGui::LogToTTY(maxDepth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void logToFile(int maxDepth, const char* filename)
{
    ImGui::LogToFile(maxDepth, filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void logToClipboard(int maxDepth)
{
    ImGui::LogToClipboard(maxDepth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void logFinish()
{
    ImGui::LogFinish();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void logButtons()
{
    ImGui::LogButtons();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

// We need a LogTextV version here.

static void logText(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    ImGui::SetTooltipV(fmt, ap);
    ImGui::LogTextV(fmt, ...);

    va_end(ap);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isItemHovered()
{
    return ImGui::IsItemHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isItemHoveredRect()
{
    return ImGui::IsItemHoveredRect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isItemActive()
{
    return ImGui::IsItemActive();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isItemVisible()
{
    return ImGui::IsItemVisible();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isAnyItemHovered()
{
    return ImGui::IsAnyItemHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isAnyItemActive()
{
    return ImGui::IsAnyItemActive();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getItemRectMin()
{
	ImVec2 t = ImGui::GetItemRectMin();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getItemRectMax()
{
    ImVec2 t = ImGui::GetItemRectMax();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getItemRectSize()
{
    ImVec2 t = ImGui::GetItemRectSize();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isWindowHovered()
{
    return ImGui::IsWindowHovered();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isWindowFocused()
{
    return ImGui::IsWindowFocused();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isRootWindowFocused()
{
    return ImGui::IsRootWindowFocused();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isRootWindowOrAnyChildFocused()
{
    return ImGui::IsRootWindowOrAnyChildFocused();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isRectVisible(const PDVec2 itemSize)
{
    return ImGui::IsRectVisible(ImVec2(itemSize.x, itemSize.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isPosHoveringAnyWindow(const PDVec2 pos)
{
    return ImGui::IsPosHoveringAnyWindow(ImVec2(pos.x, pos.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float getTime()
{
    return ImGui::GetTime();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getFrameCount()
{
    return ImGui::GetFrameCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* getStyleColName(PDUICol idx)
{
    return ImGui::GetStyleColName(ImGuiCol(idx));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 calcItemRectClosestPoint(const PDVec2 pos, bool onEdge, float outward)
{
    ImVec2 t = ImGui::CalcItemRectClosestPoint(ImVec2(pos.x, pos.y), onEdge, outward);
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 calcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImVec2 t = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void calcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end)
{
    ImGui::CalcListClipping(items_count, items_height, out_items_display_start, out_items_display_end);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool beginChildFrame(PDID id, const struct PDVec2 size)
{
    return ImGui::BeginChildFrame(id, ImVec2(size.x, size.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void endChildFrame()
{
    ImGui::EndChildFrame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void colorConvertRGBtoHSV(float r, float g, float b, float* out_h, float* out_s, float* out_v)
{
    ImGui::ColorConvertRGBtoHSV(r, g, b, *out_h, *out_s, *out_v);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void colorConvertHSVtoRGB(float h, float s, float v, float* out_r, float* out_g, float* out_b)
{
    ImGui::ColorConvertHSVtoRGB(h, s, v, *out_r, *out_g, *out_b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isKeyDown(int key_index)
{
    return ImGui::IsKeyDown(key_index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isKeyPressed(int key_index, bool repeat)
{
    return ImGui::IsKeyPressed(key_index, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isKeyReleased(int key_index)
{
    return ImGui::IsKeyReleased(key_index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isKeyDownId(uint32_t keyId, int repeat)
{
	if (!ImGui::IsWindowFocused())
		return false;

	return !!InputState_isKeyDown(keyId >> 4, keyId & 0xf, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseDown(int button)
{
    return ImGui::IsMouseDown(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseClicked(int button, bool repeat)
{
    return ImGui::IsMouseClicked(button, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseDoubleClicked(int button)
{
    return ImGui::IsMouseDoubleClicked(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseReleased(int button)
{
    return ImGui::IsMouseReleased(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseHoveringWindow()
{
    return ImGui::IsMouseHoveringWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseHoveringAnyWindow()
{
    return ImGui::IsMouseHoveringAnyWindow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseHoveringRect(const struct PDVec2 rectMin, const struct PDVec2 rectMax)
{
    return ImGui::IsMouseHoveringRect(ImVec2(rectMin.x, rectMin.y), ImVec2(rectMax.x, rectMax.y));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isMouseDragging(int button, float lockThreshold)
{
    return ImGui::IsMouseDragging(button, lockThreshold);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getMousePos()
{
    ImVec2 t = ImGui::GetMousePos();
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDVec2 getMouseDragDelta(int button, float lockThreshold)
{
    ImVec2 t = ImGui::GetMouseDragDelta(button, lockThreshold);
    PDVec2 r = { t.x, t.y };
    return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void resetMouseDragDelta(int button)
{
    ImGui::ResetMouseDragDelta(button);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUIMouseCursor getMouseCursor()
{
    return (PDUIMouseCursor)ImGui::GetMouseCursor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setMouseCursor(PDUIMouseCursor type)
{
    ImGui::SetMouseCursor(type);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void fillRect(PDRect rect, PDColor color)
{
    ImGui::FillRect(ImVec2(rect.x, rect.y), ImVec2(rect.width, rect.height), color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* buildName(const char* pluginName, int id)
{
	char name[1024];

    sprintf(name, "%s %d ###%s%d", pluginName, id, pluginName, id);

    return strdup(name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUI s_uiFuncs[] = 
{
	// Windows

	setTitle,
    getWindowSize,
    getWindowPos,
    beginChild,
    endChild,

	getScrollY,
	getScrollMaxY,
	setScrollY,
	setScrollHere,
	setScrollFromPosY,
	setKeyboardFocusHere,

	// Parameters stacks (shared)
	
	pushFont,
	popFont,
	pushStyleColor,
	popStyleColor,
	pushStyleVar,
	pushStyleVarVec,
	popStyleVar,

	// Parameters stacks (current window)

    pushItemWidth,
    popItemWidth,
	calcItemWidth,
	pushAllowKeyboardFocus,
	popAllowKeyboardFocus,
	pushTextWrapPos,
	popTextWrapPos,
	pushButtonRepeat,
	popButtonRepeat,

	// Layout

	beginGroup,
	endGroup,
    separator,
    sameLine,
    spacing,
    dummy,
    indent,
    unIndent,
    columns,
    nextColumn,
    getColumnIndex,
    getColumnOffset,
    setColumnOffset,
    getColumnWidth,
    getColumnsCount,
    getCursorPos,
    getCursorPosX,
    getCursorPosY,
    setCursorPos,
    setCursorPosX,
    setCursorPosY,
    getCursorScreenPos,
	setCursorScreenPos,
    alignFirstTextHeightToWidgets,
    getTextLineHeight,
    getTextLineHeightWithSpacing,
    getItemsLineHeightWithSpacing,

	// ID scopes
	// If you are creating widgets in a loop you most likely want to push a unique identifier so PDUI can differentiate them
	// You can also use "##extra" within your widget name to distinguish them from each others (see 'Programmer Guide')

	pushIdStr,
	pushIdStrRange,
    pushIdPtr,
    pushIdInt,
    popId,
	getIdStr,
	getIdStrRange,
	getIdPtr,

    // Widgets

	text,
	textV,
	textColored,
	textColoredV,
	textDisabled,
	textDisabledV,
	textWrapped,
	textWrappedV,
	textUnformatted,
	labelText,
	labelTextV,
	bullet,
	bulletText,
	bulletTextV,
	button,
	smallButton,
	invisibleButton,
	image,
	imageButton,
	collapsingHeader,
	checkbox,
	checkboxFlags,
	radioButtonBool,
	radioButton,
	combo,
	combo2,
	combo3,
	colorButton,
	colorEdit3,
	colorEdit4,
	colorEditMode,
	plotLines,
	plotLines2,
	plotHistogram,
	plotHistogram2,

	// Widgets: Scintilla text interface
	scInputText,

	// Widgets: Sliders (tip: ctrl+click on a slider to input text)
	sliderFloat,
	sliderFloat2,
	sliderFloat3,
	sliderFloat4,
	sliderAngle,
	sliderInt,
	sliderInt2,
	sliderInt3,
	sliderInt4,
	vsliderFloat,
	vsliderInt,

	// Widgets: Drags (tip: ctrl+click on a drag box to input text)
	dragFloat,
	dragFloat2,
	dragFloat3,
	dragFloat4,
	dragInt,
	dragInt2,
	dragInt3,
	dragInt4,

	// Widgets: Input
	inputText,
	inputTextMultiline,
	inputFloat,
	inputFloat2,
	inputFloat3,
	inputFloat4,
	inputInt,
	inputInt2,
	inputInt3,
	inputInt4,

	// Widgets: Trees
	treeNode,
	treeNodeStr,
	treeNodePtr,
	treeNodeStrV,
	treeNodePtrV,
	treePushStr,
	treePushPtr,
	treePop,
	setNextTreeNodeOpened,

	// Widgets: Selectable / Lists
	selectable,
	selectableEx,
	listBox,
	listBox2,
	listBoxHeader,
	listBoxHeader2,
	listBoxFooter,

	// Tooltip
	setTooltip,
	setTooltipV,
	beginTooltip,
	endTooltip,

	// Widgets: Menus
	beginMainMenuBar,
	endMainMenuBar,
	beginMenuBar,
	endMenuBar,
	beginMenu,
	endMenu,
	menuItem,
	menuItemPtr,

	// Popup
	openPopup,
	beginPopup,
	beginPopupModal,
	beginPopupContextItem,
	beginPopupContextWindow,
	beginPopupContextVoid,
	endPopup,
	closeCurrentPopup,

	// Widgets: value() Helpers. Output single value in "name: value" format
	valueBool,
	valueInt,
	valueUInt,
	valueFloat,
	color,

	// Logging: all text output from interface is redirected to tty/file/clipboard. Tree nodes are automatically opened.
	logToTTY,
	logToFile,
	logToClipboard,
	logFinish,
	logButtons,
	//logText,

	// Utilities
	isItemHovered,
	isItemHoveredRect,
	isItemActive,
	isItemVisible,
	isAnyItemHovered,
	isAnyItemActive,
	getItemRectMin,
	getItemRectMax,
	getItemRectSize,
	isWindowHovered,
	isWindowFocused,
	isRootWindowFocused,
	isRootWindowOrAnyChildFocused,
	isRectVisible,
	isPosHoveringAnyWindow,
	getTime,
	getFrameCount,
	getStyleColName,
	calcItemRectClosestPoint,
	calcTextSize,
	calcListClipping,

	beginChildFrame,
	endChildFrame,

	colorConvertRGBtoHSV,
	colorConvertHSVtoRGB,
	isKeyDown,
	isKeyPressed,
	isKeyReleased,

	isKeyDownId,
	isMouseDown,
	isMouseClicked,
	isMouseDoubleClicked,
	isMouseReleased,
	isMouseHoveringWindow,
	isMouseHoveringAnyWindow,
	isMouseHoveringRect,
	isMouseDragging,
	getMousePos,
	getMouseDragDelta,
	resetMouseDragDelta,
	getMouseCursor,
	setMouseCursor,

/*

	text,
    textColored,
    textWrapped,
    inputText,
    scEditText,
    checkbox,
    button,
    buttonSmall,
    buttonSize,
    selectableFixed,
    selectable,

    // Misc
    
    // Mouse

    getMousePos,
    getMouseScreenPos,
    isMouseClicked,
    isMouseDoubleClicked,
    isMouseHoveringBox,
    isItemHovered,

    // Keyboard

    isKeyDownId,
    isKeyDown,
    getKeyModifier,
    setKeyboardFocusHere,

    // Styles

    pushStyleVarV,
    pushStyleVarF,
    popStyleVar,
*/

    // Rendering

    fillRect,

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::init(ViewPluginInstance* pluginInstance)
{
	PrivateData* data = 0;

    PDUI* uiInstance = &pluginInstance->ui;

	*uiInstance = *s_uiFuncs;

    uiInstance->privateData = alloc_zero(sizeof(PrivateData));

    data = (PrivateData*)uiInstance->privateData;

    data->name = buildName(pluginInstance->plugin->name, pluginInstance->count);
    data->window = 0;
    data->showWindow = true;
    data->title = 0; 

    pluginInstance->name = data->name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback from the docking system

void updateWindowSize(void* userData, int x, int y, int width, int height)
{
	ViewPluginInstance* instance = (ViewPluginInstance*)userData;

	instance->rect.x = x;
	instance->rect.y = y;
	instance->rect.width = width;
	instance->rect.height = height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCursorStyle(DockSysCursor cursor)
{
	switch (cursor)
	{
		case DockSysCursor_SizeHorizontal : Cunsor_setType(CursorType_SizeHorizontal); break;
		case DockSysCursor_SizeVertical : Cunsor_setType(CursorType_SizeVertical); break;
		default : Cunsor_setType(CursorType_Default); break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move this code?
	
static void saveUserData(struct json_t* item, void* userData)
{
	ViewPluginInstance* view = (ViewPluginInstance*)userData;

	if (!view->plugin)
		return;

    PDSaveState saveFuncs;
    PluginIO_initSaveJson(&saveFuncs);

	PluginData* pluginData = PluginHandler_getPluginData(view->plugin);

	assert(pluginData);

	const char* pluginName = view->plugin->name;
	const char* filename = pluginData->filename;

	json_object_set_new(item, "plugin_name", json_string(pluginName));
	json_object_set_new(item, "plugin_file", json_string(filename));

	PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

	if (!viewPlugin->saveState)
		return;

	json_t* array = json_array();

	saveFuncs.privData = array;

	viewPlugin->saveState(view->userData, &saveFuncs);

	json_object_set_new(item, "plugin_data", array);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* loadUserData(struct json_t* item)
{
	ViewPluginInstance* view = 0;

	const char* pluginName = json_string_value(json_object_get(item, "plugin_name"));
	const char* filename = json_string_value(json_object_get(item, "plugin_file"));

	// if this is the case we have no plugin created (empty window)

	if (!strcmp(pluginName, "") && !strcmp(filename, ""))
	{
		view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
	}
	else
	{
		PDLoadState loadFuncs;
		PluginIO_initLoadJson(&loadFuncs);

		PluginData* pluginData = PluginHandler_findPlugin(0, filename, pluginName, true);

		if (!pluginData)
			view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
		else
			view = g_pluginUI->createViewPlugin(pluginData);

		PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

		json_t* pluginJsonData = json_object_get(item, "plugin_data");

		if (pluginJsonData && viewPlugin && viewPlugin->loadState)
		{
			SessionLoadState loadState = { pluginJsonData, (int)json_array_size(pluginJsonData), 0 };
			loadFuncs.privData = &loadState;
			viewPlugin->loadState(view->userData, &loadFuncs);
		}
	}

	// TODO: Fi this: assuming one session

    Session** sessions = Session_getSessions();

    assert(sessions);
    assert(sessions[0]);

	Session_addViewPlugin(sessions[0], view);

	return view;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static DockSysCallbacks s_dockSysCallbacks =
{
	updateWindowSize,
	setCursorStyle,
	saveUserData,
	loadUserData,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginUI::State BgfxPluginUI::updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer)
{
    PDUI* uiInstance = &instance->ui;
    PrivateData* data = (PrivateData*)uiInstance->privateData;

    float x = (float)instance->rect.x;
    float y = (float)instance->rect.y;
    float w = (float)instance->rect.width;
    float h = (float)instance->rect.height;
    
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(w - s_borderSize, h - s_borderSize));

    // TODO: Cache this?

    char title[1024];

    if (!data->title)
   		strcpy(title, data->name);
	else
	{
		sprintf(title, "%s %d - %s###%s%d", 
				instance->plugin->name, instance->count, 
				data->title, instance->plugin->name, instance->count);
	}

    ImGui::Begin(title, &data->showWindow, ImVec2(0, 0), true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    instance->plugin->update(instance->userData, uiInstance, reader, writer);

    ImGui::End();

    // Draw border

    if  (!data->showWindow)
        return CloseView;

    return None;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BgfxPluginUI::getStatusBarSize()
{
    return m_statusSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderStatusBar(const char* text, float statusSize)
{
    const ImGuiIO& io = ImGui::GetIO();
    ImVec2 size = io.DisplaySize;
    float yPos = size.y - statusSize;

    ImGui::SetNextWindowPos(ImVec2(0.0f, yPos));
    ImGui::SetNextWindowSize(ImVec2(size.x, statusSize));

    bool show = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(40, 40, 40));

    ImGui::Begin("", &show, ImVec2(0, 0), true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetCursorPos(ImVec2(2.0f, 4.0f));
    ImGui::Text("Status: %s", text);
    ImGui::End();

    ImGui::PopStyleColor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::setStatusTextNoFormat(const char* text)
{
    string_copy(m_statusText, text, sizeof(m_statusText));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDock(UIDockingGrid* grid)
{
    switch (UIDock_getSizingState(grid))
    {
        case UIDockSizerDir_None:
        {
            Cunsor_setType(CursorType_Default);
            break;
        }

        case UIDockSizerDir_Horz:
        {
            Cunsor_setType(CursorType_SizeHorizontal);
            break;
        }

        case UIDockSizerDir_Vert:
        {
            Cunsor_setType(CursorType_SizeVertical);
            break;
        }

        case UIDockSizerDir_Both:
        {
            Cunsor_setType(CursorType_SizeAll);
            break;
        }
    }

    UIDock_update(grid, InputState_getState());
    UIDock_render(grid);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDocking(Session* session)
{
    InputState* state = InputState_getState();

    int mx = (int)state->mousePos.x;
    int my = (int)state->mousePos.y;

	struct ViewPluginInstance* view = Session_getViewAt(session, mx, my, 0); 

	docksys_set_mouse(view, mx, my, state->mouseDown[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::preUpdate()
{
	const float deltaTime = 1.0f / 60.f; // TODO: Calc correct dt

    bgfx::setViewRect(0, 0, 0, (uint16_t)s_context.width, (uint16_t)s_context.height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xf01010ff, 1.0f, 0);
    bgfx::submit(0);

    IMGUI_preUpdate(deltaTime);
    InputState_update(deltaTime);

    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i)
    {
        Session* session = sessions[i];
		updateDocking(session);
    }

	docksys_update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PosColorVertex* fillRectBorder(PosColorVertex* verts, IntRect* rect, uint32_t color)
{
    const float x0 = (float)rect->x;
    const float y0 = (float)rect->y;
    const float x1 = (float)rect->width + x0;
    const float y1 = (float)rect->height + y0;

    // First triangle

    verts[0].x = x0;
    verts[0].y = y0;
    verts[0].color = color;

    verts[1].x = x1;
    verts[1].y = y0;
    verts[1].color = color;

    verts[2].x = x1;
    verts[2].y = y1;
    verts[2].color = color;

    // Second triangle

    verts[3].x = x0;
    verts[3].y = y0;
    verts[3].color = color;

    verts[4].x = x1;
    verts[4].y = y1;
    verts[4].color = color;

    verts[5].x = x0;
    verts[5].y = y1;
    verts[5].color = color;

    verts += 6;

    return verts;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void renderBorders(Session* session)
{
	int count = 0;
	ViewPluginInstance** views = Session_getViewPlugins(session, &count);

    bgfx::TransientVertexBuffer tvb;

    const uint32_t vertexCount = (uint32_t)count * 2 * 6;

    UIRender_allocPosColorTb(&tvb, vertexCount);
    PosColorVertex* verts = (PosColorVertex*)tvb.data;

    // TODO: Use settings for colors

    const uint32_t colorDefalut = (0x40 << 16) | (0x40 << 8) | 0x40;
    const uint32_t colorHigh = (0x60 << 16) | (0x60 << 8) | 0x60;

    for (int i = 0; i < count; ++i)
    {
    	IntRect t = views[i]->rect; 

    	IntRect t0 = {{{ t.x + t.width - s_borderSize, t.y, s_borderSize, t.height }}}; 
    	IntRect t1 = {{{ t.x, t.y + t.height - s_borderSize, t.width, s_borderSize }}}; 

        verts = fillRectBorder(verts, &t0, colorDefalut);
        verts = fillRectBorder(verts, &t1, colorDefalut);
    }

    bgfx::setState(0
                   | BGFX_STATE_RGB_WRITE
                   | BGFX_STATE_ALPHA_WRITE
                   | BGFX_STATE_MSAA);

    UIRender_posColor(&tvb, 0, vertexCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::postUpdate()
{
    renderStatusBar(m_statusText, (float)m_statusSize);
    IMGUI_postUpdate();

    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i)
    {
        Session* session = sessions[i];
        renderBorders(session);
    }

    bgfx::frame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::create(void* windowHandle, int width, int height)
{
	docksys_set_callbacks(&s_dockSysCallbacks);

#ifdef PRODBG_WIN
	bgfx::winSetHwnd((HWND)windowHandle);
#endif
    bgfx::init();
    bgfx::reset((uint32_t)width, (uint32_t)height);
    bgfx::setViewSeq(0, true);
    IMGUI_setup(width, height);

    s_context.width = width;
    s_context.height = height;

	Service_register(&g_serviceMessageFuncs, PDMESSAGEFUNCS_GLOBAL);

    Cursor_init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BgfxPluginUI::destroy()
{
}

// It's a bit weird to have the code like this here. To be cleaned up

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMousePos(float x, float y)
{
    InputState* state = InputState_getState();

    state->mousePos.x = x;
    state->mousePos.y = y;

    IMGUI_setMousePos(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setMouseState(int button, int state)
{
    InputState* inputState = InputState_getState();
    inputState->mouseDown[button] = !!state;

    IMGUI_setMouseState(button, state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setScroll(float x, float y)
{
	(void)x;
	IMGUI_setScroll(y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyDown(int key, int modifier)
{
    InputState* state = InputState_getState();

    state->keysDown[key] = true;
    state->modifiers = modifier;

    IMGUI_setKeyDown(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyDownMods(int modifier)
{
    InputState* state = InputState_getState();
    state->modifiers = modifier;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_keyUp(int key, int modifier)
{
    InputState* state = InputState_getState();

    state->keysDown[key] = false;
    state->modifiers = modifier;

    IMGUI_setKeyUp(key, modifier);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_addChar(unsigned short c)
{
    IMGUI_addInputCharacter(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProDBG_setWindowSize(int width, int height)
{
    Context* context = &s_context;

    context->width = width;
    context->height = height;

    bgfx::reset((uint32_t)width, (uint32_t)height);
    IMGUI_updateSize(width, height);

    Session** sessions = Session_getSessions();

    for (int i = 0; i < array_size(sessions); ++i)
    {
        Session* session = sessions[i];
        docksys_update_size(width, height - (int)g_pluginUI->getStatusBarSize());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IMGUI_beginMainMenuBar()
{
    return ImGui::BeginMainMenuBar();
}
void IMGUI_endMainMenuBar()
{
    ImGui::EndMainMenuBar();
}

bool IMGUI_beginMenu(const char *menuName)
{
    return ImGui::BeginMenu(menuName);
}
bool IMGUI_menuItem(const char *itemName)
{
    return ImGui::MenuItem(itemName);
}
void IMGUI_endMenu()
{
    ImGui::EndMenu();
}

void IMGUI_openPopup(const char *popupId)
{
    ImGui::OpenPopup(popupId);
}
bool IMGUI_beginPopup(const char *popupName)
{
    return ImGui::BeginPopup(popupName);
}
void IMGUI_endPopup()
{
    ImGui::EndPopup();
}

bool IMGUI_isItemHovered()
{
    return ImGui::IsItemHovered();
}




