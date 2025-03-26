#define EXTENSION_NAME AdMobExt
#define LIB_NAME "Admob"
#define MODULE_NAME "admob"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#include "admob_private.h"
#include "admob_callback_private.h"
#include "utils/LuaUtils.h"

namespace dmAdmob {

static const char DEFOLD_USERAGENT[] = "defold-3.6.0";

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

static int Lua_LoadAppOpen(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_type(L, 1) != LUA_TSTRING) {
        return DM_LUA_ERROR("Expected string, got %s. Wrong type for App Open Ad UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    LoadAppOpen(unitId_lua, false);
    return 0;
}

static int Lua_ShowAppOpen(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowAppOpen();
    return 0;
}

static int Lua_LoadInterstitial(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_type(L, 1) != LUA_TSTRING) {
        return DM_LUA_ERROR("Expected string, got %s. Wrong type for Interstitial UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
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
        return DM_LUA_ERROR("Expected string, got %s. Wrong type for Rewarded UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
    }
    if (lua_type(L, 2) != LUA_TSTRING) {
        return DM_LUA_ERROR("Expected string, got %s. Wrong type for Rewarded UserId variable '%s'.", luaL_typename(L, 2), lua_tostring(L, 2));
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    const char* userId_lua =  luaL_checkstring(L, 2);
    const char* customData_lua = lua_type(L,3) == LUA_TNONE ? nullptr : luaL_checkstring(L, 3);
    LoadRewarded(unitId_lua, userId_lua, customData_lua);
    return 0;
}

static int Lua_ShowRewarded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowRewarded();
    return 0;
}

static int Lua_LoadRewardedInterstitial(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_type(L, 1) != LUA_TSTRING) {
        return DM_LUA_ERROR("Expected string, got %s. Wrong type for Rewarded Interstitial UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    LoadRewardedInterstitial(unitId_lua);
    return 0;
}

static int Lua_ShowRewardedInterstitial(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowRewardedInterstitial();
    return 0;
}

static int Lua_LoadBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_type(L, 1) != LUA_TSTRING) {
        return DM_LUA_ERROR("Expected string, got %s. Wrong type for Banner UnitId variable '%s'.", luaL_typename(L, 1), lua_tostring(L, 1));
    }
    const char* unitId_lua = luaL_checkstring(L, 1);
    BannerSize bannerSize_lua = SIZE_ADAPTIVE_BANNER;
    if (lua_type(L, 2) != LUA_TNONE) {
        bannerSize_lua = (BannerSize)luaL_checknumber(L, 2);
    }
    LoadBanner(unitId_lua, bannerSize_lua);
    return 0;
}

static int Lua_DestroyBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    DestroyBanner();
    return 0;
}

static int Lua_ShowBanner(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    BannerPosition bannerPos_lua = POS_NONE;
    if (lua_type(L, 1) != LUA_TNONE) {
        bannerPos_lua = (BannerPosition)luaL_checknumber(L, 1);
    }
    ShowBanner(bannerPos_lua);
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

static int Lua_IsRewardedInterstitialLoaded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    bool is_loaded = IsRewardedInterstitialLoaded();
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

static int Lua_IsAppOpenLoaded(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    bool is_loaded = IsAppOpenLoaded();
    lua_pushboolean(L, is_loaded);
    return 1;
}

static int Lua_SetPrivacySettings(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    bool enable_rdp = luaL_checkbool(L, 1);
    SetPrivacySettings(enable_rdp);
    return 0;
}

static int Lua_RequestIDFA(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    RequestIDFA();
    return 0;
}

static int Lua_ShowAdInspector(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ShowAdInspector();
    return 0;
}

static int Lua_SetMaxAdContentRating(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    MaxAdRating max_ad_rating = (MaxAdRating)luaL_checknumber(L, 1);
    SetMaxAdContentRating(max_ad_rating);
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"initialize", Lua_Initialize},
    {"set_callback", Lua_SetCallback},
    {"load_appopen", Lua_LoadAppOpen},
    {"show_appopen", Lua_ShowAppOpen},
    {"load_interstitial", Lua_LoadInterstitial},
    {"show_interstitial", Lua_ShowInterstitial},
    {"load_rewarded", Lua_LoadRewarded},
    {"show_rewarded", Lua_ShowRewarded},
    {"load_rewarded_interstitial", Lua_LoadRewardedInterstitial},
    {"show_rewarded_interstitial", Lua_ShowRewardedInterstitial},
    {"load_banner", Lua_LoadBanner},
    {"destroy_banner", Lua_DestroyBanner},
    {"show_banner", Lua_ShowBanner},
    {"hide_banner", Lua_HideBanner},
    {"is_rewarded_loaded", Lua_IsRewardedLoaded},
    {"is_interstitial_loaded", Lua_IsInterstitialLoaded},
    {"is_rewarded_interstitial_loaded", Lua_IsRewardedInterstitialLoaded},
    {"is_banner_loaded", Lua_IsBannerLoaded},
    {"is_appopen_loaded", Lua_IsAppOpenLoaded},
    {"set_privacy_settings", Lua_SetPrivacySettings},
    {"request_idfa", Lua_RequestIDFA},
    {"show_ad_inspector", Lua_ShowAdInspector},
    {"set_max_ad_content_rating", Lua_SetMaxAdContentRating},
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
    SETCONSTANT(MSG_IDFA)
    SETCONSTANT(MSG_REWARDED_INTERSTITIAL)
    SETCONSTANT(MSG_APPOPEN)

    SETCONSTANT(EVENT_CLOSED)
    SETCONSTANT(EVENT_FAILED_TO_SHOW)
    SETCONSTANT(EVENT_OPENING)
    SETCONSTANT(EVENT_FAILED_TO_LOAD)
    SETCONSTANT(EVENT_LOADED)
    SETCONSTANT(EVENT_NOT_LOADED)
    SETCONSTANT(EVENT_EARNED_REWARD)
    SETCONSTANT(EVENT_COMPLETE)
    SETCONSTANT(EVENT_CLICKED)
    SETCONSTANT(EVENT_DESTROYED)
    SETCONSTANT(EVENT_JSON_ERROR)
    SETCONSTANT(EVENT_IMPRESSION_RECORDED)
    SETCONSTANT(EVENT_STATUS_AUTHORIZED)
    SETCONSTANT(EVENT_STATUS_DENIED)
    SETCONSTANT(EVENT_STATUS_NOT_DETERMINED)
    SETCONSTANT(EVENT_STATUS_RESTRICTED)
    SETCONSTANT(EVENT_NOT_SUPPORTED)

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

    SETCONSTANT(POS_NONE)
    SETCONSTANT(POS_TOP_LEFT)
    SETCONSTANT(POS_TOP_CENTER)
    SETCONSTANT(POS_TOP_RIGHT)
    SETCONSTANT(POS_BOTTOM_LEFT)
    SETCONSTANT(POS_BOTTOM_CENTER)
    SETCONSTANT(POS_BOTTOM_RIGHT)
    SETCONSTANT(POS_CENTER)

    SETCONSTANT(MAX_AD_CONTENT_RATING_G)
    SETCONSTANT(MAX_AD_CONTENT_RATING_PG)
    SETCONSTANT(MAX_AD_CONTENT_RATING_T)
    SETCONSTANT(MAX_AD_CONTENT_RATING_MA)

    #undef SETCONSTANT

    // for backwards compatibility (Issue #18)
    lua_pushnumber(L, EVENT_STATUS_AUTHORIZED);
    lua_setfield(L, -2, "EVENT_STATUS_AUTORIZED");

    lua_pop(L, 1);
}

static dmExtension::Result AppInitializeAdmob(dmExtension::AppParams* params)
{
    dmLogInfo("AppInitializeAdmob");
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAdmob(dmExtension::Params* params)
{
    dmLogInfo("InitializeAdmob");
    LuaInit(params->m_L);
    Initialize_Ext(params, DEFOLD_USERAGENT);
    InitializeCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeAdmob(dmExtension::AppParams* params)
{
    Finalize_Ext();
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

static void OnEventAdmob(dmExtension::Params* params, const dmExtension::Event* event)
 {
    switch(event->m_Event)
    {
        case dmExtension::EVENT_ID_ACTIVATEAPP:
            ActivateApp();
            break;
    }
 }

} //namespace dmAdmob

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, dmAdmob::AppInitializeAdmob, dmAdmob::AppFinalizeAdmob, dmAdmob::InitializeAdmob, dmAdmob::UpdateAdmob, dmAdmob::OnEventAdmob, dmAdmob::FinalizeAdmob)

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
