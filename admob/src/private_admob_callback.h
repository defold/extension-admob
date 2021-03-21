#pragma once

#include "private_admob.h"
#include <dmsdk/sdk.h>

namespace dmAdmob {

// The same events and messages are in AdmobJNI.java
// If you change enums here, pls nake sure you update them here as well

enum MESSAGE_ID
{
    MSG_INTERSTITIAL =              1,
    MSG_REWARDED =                  2
};

enum MESSAGE_EVENT
{
    EVENT_CLOSED =                  1,
    EVENT_FAILED_TO_SHOW =          2,
    EVENT_OPENING =                 3,
    EVENT_FAILED_TO_LOAD =          4,
    EVENT_LOADED =                  5,
    EVENT_NOT_LOADED =              6,
    EVENT_EARNED_REWARD =           7
};

struct CallbackData
{
    MESSAGE_ID msg;
    char* json;
};

void SetLuaCallback(lua_State* L, int pos);
void UpdateCallback();
void InitializeCallback();
void FinalizeCallback();

void AddToQueueCallback(MESSAGE_ID type, const char*json);

} //namespace
