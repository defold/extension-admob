#define EXTENSION_NAME AdMobExt
#define LIB_NAME "Admob"
#define MODULE_NAME "admob"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID)//|| defined(DM_PLATFORM_IOS)

#include "admob_private.h"
#include "admob_callback_private.h"

namespace dmAdmob {

static int Lua_Initialize(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Initialize();
    return 0;
}

static int Lua_SetCallback(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    SetLuaCallback(L, 1);
    return 0;
}

static int Lua_LoadInterstitial(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_type(L, 1) != LUA_TSTRING) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Expected string, got %s. Wrong type for Interstitial UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
        luaL_error(L, msg);
        return 0;
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    LoadInterstitial(unitId_lua);
    return 0;
}

static int Lua_ShowInterstitial(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowInterstitial();
    return 0;
}

static int Lua_LoadRewarded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_type(L, 1) != LUA_TSTRING) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Expected string, got %s. Wrong type for Interstitial UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
        luaL_error(L, msg);
        return 0;
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    LoadRewarded(unitId_lua);
    return 0;
}

static int Lua_ShowRewarded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowRewarded();
    return 0;
}

static int Lua_LoadBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
        if (lua_type(L, 1) != LUA_TSTRING) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Expected string, got %s. Wrong type for Interstitial UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
        luaL_error(L, msg);
        return 0;
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    BannerSize bannerSize_lua = SIZE_ADAPTIVE_BANNER;
    if (lua_type(L, 2) != LUA_TNONE) {
        bannerSize_lua = (BannerSize)luaL_checknumber(L, 2);
    }
    LoadBanner(unitId_lua, bannerSize_lua);
    return 0;
}

static int Lua_UnloadBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    UnloadBanner();
    return 0;
}

static int Lua_ShowBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowBanner();
    return 0;
}

static int Lua_HideBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    HideBanner();
    return 0;
}

static int Lua_IsRewardedLoaded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    bool is_loaded = IsRewardedLoaded();
    lua_pushboolean(L, is_loaded);
    return 1;
}

static int Lua_IsInterstitialLoaded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    bool is_loaded = IsInterstitialLoaded();
    lua_pushboolean(L, is_loaded);
    return 1;
}

static int Lua_IsBannerLoaded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    bool is_loaded = IsBannerLoaded();
    lua_pushboolean(L, is_loaded);
    return 1;
}

static const luaL_reg Module_methods[] =
{
    {"initialize", Lua_Initialize},
    {"set_callback", Lua_SetCallback},
    {"load_interstitial", Lua_LoadInterstitial},
    {"show_interstitial", Lua_ShowInterstitial},
    {"load_rewarded", Lua_LoadRewarded},
    {"show_rewarded", Lua_ShowRewarded},
    {"load_banner", Lua_LoadBanner},
    {"unload_banner", Lua_UnloadBanner},
    {"show_banner", Lua_ShowBanner},
    {"hide_banner", Lua_HideBanner},
    {"is_rewarded_loaded", Lua_IsRewardedLoaded},
    {"is_interstitial_loaded", Lua_IsInterstitialLoaded},
    {"is_banner_loaded", Lua_IsBannerLoaded},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_register(L, MODULE_NAME, Module_methods);

    #define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number) name); \
    lua_setfield(L, -2, #name); \

    SETCONSTANT(MSG_INTERSTITIAL)
    SETCONSTANT(MSG_REWARDED)
    SETCONSTANT(MSG_BANNER)
    SETCONSTANT(MSG_INITIALIZATION)

    SETCONSTANT(EVENT_CLOSED)
    SETCONSTANT(EVENT_FAILED_TO_SHOW)
    SETCONSTANT(EVENT_OPENING)
    SETCONSTANT(EVENT_FAILED_TO_LOAD)
    SETCONSTANT(EVENT_LOADED)
    SETCONSTANT(EVENT_NOT_LOADED)
    SETCONSTANT(EVENT_EARNED_REWARD)
    SETCONSTANT(EVENT_COMPLETE)
    SETCONSTANT(EVENT_CLICKED)
    SETCONSTANT(EVENT_UNLOADED)

    SETCONSTANT(SIZE_ADAPTIVE_BANNER)
    SETCONSTANT(SIZE_BANNER)
    SETCONSTANT(SIZE_FLUID)
    SETCONSTANT(SIZE_FULL_BANNER)
    SETCONSTANT(SIZE_LARGE_BANNER)
    SETCONSTANT(SIZE_LEADEARBOARD)
    SETCONSTANT(SIZE_MEDIUM_RECTANGLE)
    SETCONSTANT(SIZE_SEARH)
    SETCONSTANT(SIZE_SKYSCRAPER)
    SETCONSTANT(SIZE_SMART_BANNER)

    #undef SETCONSTANT

    lua_pop(L, 1);
}

static dmExtension::Result AppInitializeAdmob(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAdmob(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    Initialize_Ext();
    InitializeCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeAdmob(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeAdmob(dmExtension::Params* params)
{
    FinalizeCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateAdmob(dmExtension::Params* params)
{
    UpdateCallback();
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
