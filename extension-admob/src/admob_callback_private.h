#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
#pragma once

#include "admob_private.h"
#include <dmsdk/sdk.h>

namespace dmAdmob {

// The same events and messages are in AdmobJNI.java
// If you change enums here, pls make sure you update the constants there as well

enum MessageId
{
    MSG_INTERSTITIAL =              1,
    MSG_REWARDED =                  2,
    MSG_BANNER =                    3,
    MSG_INITIALIZATION =            4,
    MSG_IDFA =                      5,
    MSG_REWARDED_INTERSTITIAL =     6,
    MSG_APPOPEN =                   7
};

enum MessageEvent
{
    EVENT_CLOSED =                  1,
    EVENT_FAILED_TO_SHOW =          2,
    EVENT_OPENING =                 3,
    EVENT_FAILED_TO_LOAD =          4,
    EVENT_LOADED =                  5,
    EVENT_NOT_LOADED =              6,
    EVENT_EARNED_REWARD =           7,
    EVENT_COMPLETE =                8,
    EVENT_CLICKED =                 9,
    EVENT_DESTROYED =               10,
    EVENT_JSON_ERROR =              11,
    EVENT_IMPRESSION_RECORDED =     12,
    EVENT_STATUS_AUTHORIZED =       13,
    EVENT_STATUS_DENIED =           14,
    EVENT_STATUS_NOT_DETERMINED =   15,
    EVENT_STATUS_RESTRICTED =       16,
    EVENT_NOT_SUPPORTED =           17
};

struct CallbackData
{
    MessageId msg;
    char* json;
};

void SetLuaCallback(lua_State* L, int pos);
void UpdateCallback();
void InitializeCallback();
void FinalizeCallback();

void AddToQueueCallback(MessageId type, const char*json);

} //namespace

#endif
