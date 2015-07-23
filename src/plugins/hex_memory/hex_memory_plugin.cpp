#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HexMemoryData
{
    unsigned char* data;
    unsigned char* oldData;
    int dataSize;
    int addressSize;
    char startAddress[64];
    char endAddress[64];
    bool requestData;
    uint64_t sa;
    uint64_t ea;
    uint64_t exceptionLocation;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;

    HexMemoryData* userData = (HexMemoryData*)malloc(sizeof(HexMemoryData));
    memset(userData, 0, sizeof(sizeof(HexMemoryData)));

    strcpy(userData->startAddress, "0x00000000");
    strcpy(userData->endAddress, "0x00001000");

    userData->sa = 0;
    userData->ea = 0x00000fff;

    userData->data = (unsigned char*)malloc(1024 * 1024);
    userData->addressSize = 2;

    userData->oldData = (unsigned char*)malloc(1024 * 1024);
    userData->addressSize = 2;

    // clear

    memset(userData->data, 0xff, 1024 * 1024);
    memset(userData->oldData, 0xff, 1024 * 1024);

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getAddressLine(char* adressText, uint64_t address, int adressSize)
{
    switch (adressSize)
    {
        case 1:
            sprintf(adressText, "0x%02x", (uint8_t)address); break;
        case 2:
            sprintf(adressText, "0x%04x", (uint16_t)address); break;
        case 4:
            sprintf(adressText, "0x%08x", (uint32_t)address); break;
        case 8:
            sprintf(adressText, "0x%16llx", (int64_t)address); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawData(HexMemoryData* data, PDUI* uiFuncs, int lineCount, int charsPerLine)
{
    uint64_t address = (uint64_t)strtol(data->startAddress, 0, 16);
    int adressSize = data->addressSize;
    uint8_t* memoryData = data->data + address;
    uint8_t* oldMemoryData = data->oldData + address;

    if (charsPerLine > 1024)
        charsPerLine = 1024;

    for (int i = 0; i < lineCount; ++i)
    {
        char addressText[64] = { 0 };

        // Get Address

        getAddressLine(addressText, address, adressSize);

        // Get Hex and chars

        uiFuncs->text("%s: ", addressText); uiFuncs->sameLine(0, -1);

        PDColor color = PDUI_COLOR(255, 0, 0, 255); 

        // Print hex values

        for (int p = 0; p < charsPerLine; ++p)
        {
            uint8_t c = memoryData[p];
            uint8_t co = oldMemoryData[p];

            if (c == co)
                uiFuncs->text("%02x", c);
            else
                uiFuncs->textColored(color, "%02x", c);

            uiFuncs->sameLine(0, -1);
        }

        // print characters

        for (int p = 0; p < charsPerLine; ++p)
        {
            uint8_t c = memoryData[p];
            uint8_t co = oldMemoryData[p];
            char wc = 0;

            if (c >= 32 && c < 128)
                wc = (char)c;
            else
                wc = '.';

            if (c == co)
                uiFuncs->text("%c", wc);
            else
                uiFuncs->textColored(color, "%c", wc);

            uiFuncs->sameLine(0, 0);
        }

        uiFuncs->text("\n");

        address += (uint32_t)charsPerLine;
        memoryData += charsPerLine;
        oldMemoryData += charsPerLine;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawUI(HexMemoryData* data, PDUI* uiFuncs)
{
    uiFuncs->pushItemWidth(100);
    uiFuncs->inputText("Start Address", data->startAddress, sizeof(data->startAddress), PDUIInputTextFlags_CharsHexadecimal, 0, 0);
    uiFuncs->sameLine(0, -1);
    uiFuncs->inputText("End Address", data->endAddress, sizeof(data->endAddress), 0, 0, 0);
    uiFuncs->popItemWidth();

    PDVec2 size = { 0.0f, 0.0f };

    long startAddress = strtol(data->startAddress, 0, 16);
    long endAddress = strtol(data->endAddress, 0, 16);

    if ((endAddress >= startAddress) && !data->data)
        return;

    if (data->sa != (uint64_t)startAddress)
    {
        data->requestData = true;
        data->sa = (uint64_t)startAddress;
    }

    if (data->ea != (uint64_t)endAddress)
    {
        data->requestData = true;
        data->ea = (uint64_t)endAddress;
    }

    //PDVec2 textStart = uiFuncs->getCursorPos();
    PDVec2 windowSize = uiFuncs->getWindowSize();

    uiFuncs->beginChild("child", size, false, 0);

    //PDRect rect = uiFuncs->getCurrentClipRect();
    //PDVec2 pos = uiFuncs->getWindowPos();

    //printf("pos %f %f\n", pos.x, pos.y);
    //printf("rect %f %f %f %f\n", rect.x, rect.y, rect.width, rect.height);

	// TODO: Fix me
    const float fontWidth = 13.0f; // uiFuncs->getFontWidth();

    float drawableChars = (float)(int)(windowSize.x / (fontWidth + 23));

    int drawableLineCount = (int)((endAddress - startAddress) / (int)drawableChars);

    //printf("%d %d %d %d\n", drawableLineCount, (int)endAddress, (int)startAddress, (int)drawableChars);

    drawData(data, uiFuncs, drawableLineCount, (int)drawableChars);

    uiFuncs->endChild();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateMemory(HexMemoryData* userData, PDReader* reader)
{
    void* data;
    uint64_t address = 0;
    uint64_t size = 0;

    PDRead_findU64(reader, &address, "address", 0);

    if (PDRead_findData(reader, &data, &size, "data", 0) == PDReadStatus_notFound)
        return;

    //printf("%s(%d) update memory\n", __FILE__, __LINE__);

    // TODO: Currently we just copy the memory into a predefined memory range. This needs to really be fixed
    // We need some overlapping memory ranges and such here to handle this correctly

    if (address + size >= 1024 * 1024)
    {
        printf("%s(%d) address (0x%16llx %x) + size larger than 1 mb which isn't supported right now\n", __FILE__, __LINE__, address, (int)size);
        return;
    }

    // TODO: VirtualMemory manager that can requestmemory on a per 4k page or something similar instead of this

    // save the old data that is used for showing the changes
    memcpy(userData->oldData + address, userData->data + address, (size_t)size);

    // And update with the new data
    memcpy(userData->data + address, data, (size_t)size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateExceptionLocation(HexMemoryData* data, PDReader* reader)
{
    uint64_t address = 0;

    PDRead_findU64(reader, &address, "address", 0);

    if (data->exceptionLocation == address)
        return;

    data->requestData = true;
    data->exceptionLocation = address;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
    uint32_t event;

    HexMemoryData* data = (HexMemoryData*)userData;

    data->requestData = false;

    // Loop over all the in events

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setMemory:
            {
                updateMemory(data, inEvents);
                break;
            }

            case PDEventType_setExceptionLocation:
            {
                updateExceptionLocation(data, inEvents);
                break;
            }
        }
    }

    drawUI(data, uiFuncs);

    if (data->requestData)
    {
        //printf("requesting memory range %04x - %04x\n", (uint16_t)data->sa, (uint16_t)data->ea);
        PDWrite_eventBegin(writer, PDEventType_getMemory);
        PDWrite_u64(writer, "address_start", data->sa);
        PDWrite_u64(writer, "size", data->ea - data->sa);
        PDWrite_eventEnd(writer);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int saveState(void* userData, struct PDSaveState* saveState)
{
    HexMemoryData* data = (HexMemoryData*)userData;

    PDIO_writeString(saveState, data->startAddress);
    PDIO_writeString(saveState, data->endAddress);

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int loadState(void* userData, struct PDLoadState* loadState)
{
    HexMemoryData* data = (HexMemoryData*)userData;

    PDIO_readString(loadState, data->startAddress, sizeof(data->startAddress));
    PDIO_readString(loadState, data->endAddress, sizeof(data->endAddress));

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Hex Memory View",
    createInstance,
    destroyInstance,
    update,
    saveState,
    loadState,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
    {
        registerPlugin(PD_VIEW_API_VERSION, &plugin, privateData);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

