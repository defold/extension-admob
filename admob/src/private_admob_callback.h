#pragma once

#include "private_admob.h"
#include <dmsdk/sdk.h>

namespace dmAdmob {

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
