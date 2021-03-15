#if defined(DM_PLATFORM_ANDROID)// || defined(DM_PLATFORM_IOS)

#include "private_admob_callback.h"
#include "utils/LuaUtils.h"
#include <stdlib.h>

namespace dmAdmob {

static dmScript::LuaCallbackInfo* m_luaCallback = 0x0;
static dmArray<CallbackData> m_callbacksQueue;
static dmMutex::HMutex m_mutex;

static void DestroyCallback()
{
    if (m_luaCallback != 0x0)
    {
        dmScript::DestroyCallback(m_luaCallback);
        m_luaCallback = 0x0;
    }
}

static void InvokeCallback(MESSAGE_ID type, char*json)
{
    if (!dmScript::IsCallbackValid(m_luaCallback))
    {
        dmLogError("Admob callback is invalid. Set new callback unsing `admob.setCallback()` funciton.");
        return;
    }

    lua_State* L = dmScript::GetCallbackLuaContext(m_luaCallback);
    int top = lua_gettop(L);

    if (!dmScript::SetupCallback(m_luaCallback))
    {
        return;
    }

    lua_pushnumber(L, type);
    bool is_fail = false;
    dmJson::Document doc;
    dmJson::Result r = dmJson::Parse(json, &doc);
    if (r == dmJson::RESULT_OK && doc.m_NodeCount > 0)
    {
        char error_str_out[128];
        if (dmScript::JsonToLua(L, &doc, 0, error_str_out, sizeof(error_str_out)) < 0)
        {
            dmLogError("Failed converting object JSON to Lua; %s", error_str_out);
            is_fail = true;
        }
    } 
    else
    {
        dmLogError("Failed to parse JSON object(%d): (%s)", r, json);
        is_fail = true;
    }
    dmJson::Free(&doc);
    if (is_fail)
    {
        lua_pop(L, 2);
        assert(top == lua_gettop(L));
        return;
    }

    int number_of_arguments = 3;

    int ret = dmScript::PCall(L, number_of_arguments, 0);
    (void)ret;
    dmScript::TeardownCallback(m_luaCallback);

    assert(top == lua_gettop(L));
}

void InitializeCallback()
{
    m_mutex = dmMutex::New();
}

void FinalizeCallback()
{
    dmMutex::Delete(m_mutex);
    DestroyCallback();
}

void SetLuaCallback(lua_State* L, int pos)
{
    int type = lua_type(L, pos);
    if (type == LUA_TNONE || type == LUA_TNIL)
    {
        DestroyCallback();
    }
    else
    {
        m_luaCallback = dmScript::CreateCallback(L, pos);
    }
}

void AddToQueueCallback(MESSAGE_ID type, const char*json)
{
    DM_MUTEX_SCOPED_LOCK(m_mutex);

    CallbackData data;
    data.msg = type;
    data.json = json ? strdup(json) : NULL;

    if(m_callbacksQueue.Full())
    {
        m_callbacksQueue.OffsetCapacity(2);
    }
    m_callbacksQueue.Push(data);
}

void UpdateCallback()
{
    if (m_callbacksQueue.Empty())
    {
        return;
    }

    DM_MUTEX_SCOPED_LOCK(m_mutex);
    
    for(uint32_t i = 0; i != m_callbacksQueue.Size(); ++i)
    {
        CallbackData* data = &m_callbacksQueue[i];
        InvokeCallback(data->msg, data->json);
        if(data->json)
        {
            free(data->json);
            data->json = 0;
        }
    }
    m_callbacksQueue.SetSize(0);
}

} //namespace

#endif
