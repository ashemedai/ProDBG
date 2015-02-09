#pragma once

#include "api/include/pd_ui.h"
#include "core/math.h"

struct PDReader;
struct PDWriter;
struct PDViewPlugin;
struct PDBackendPlugin;
struct PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDBackendInstance
{
    struct PDBackendPlugin* plugin;
    void* userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance
{
    PDUI ui;
    PDViewPlugin* plugin;
    Rect rect;
    void* userData;
    int count;
    bool markDeleted;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewPluginInstance* PluginInstance_createViewPlugin(PluginData* pluginData);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

