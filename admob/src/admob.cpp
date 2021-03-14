#define EXTENSION_NAME AdMobExt
#define LIB_NAME "Admob"
#define MODULE_NAME "admob"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)

#if defined(DM_PLATFORM_ANDROID)
#include "admob_android.h"
#endif //Android

namespace dmAdmob {

static int Lua_Initialize(lua_State* L) {

    DM_LUA_STACK_CHECK(L, 0);
    Initialize();
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"initialize", Lua_Initialize},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
}

static dmExtension::Result AppInitializeAdmob(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAdmob(dmExtension::Params* params)
{
    LuaInit(params->m_L);
#if defined(DM_PLATFORM_ANDROID)
    InitializeJNI();
#endif //Android
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeAdmob(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeAdmob(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateAdmob(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

} //namespace dmAdmob

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, dmAdmob::AppInitializeAdmob, dmAdmob::AppFinalizeAdmob, dmAdmob::InitializeAdmob, dmAdmob::UpdateAdmob, 0, dmAdmob::FinalizeAdmob)

#else

static  dmExtension::Result InitializeAdmob(dmExtension::Params* params)
{
    dmLogInfo("Registered extension Admob (null)");
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeAdmob(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, 0, 0, InitializeAdmob, 0, 0, FinalizeAdmob)

#endif // IOS/Android
