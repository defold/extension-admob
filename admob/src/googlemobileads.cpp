#define EXTENSION_NAME AdMob
#define MODULE_NAME "admob"
#define LIB_NAME "AdMob"

// Defold SDK
#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)

// Firebase sdk ref:
// https://firebase.google.com/docs/reference/cpp/namespace/firebase/admob
// https://firebase.google.com/docs/reference/cpp/struct/firebase/admob/ad-request
// https://firebase.google.com/docs/admob/cpp/rewarded-video

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "firebase/admob.h"
#include "firebase/admob/banner_view.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/native_express_ad_view.h"
#include "firebase/admob/rewarded_video.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"

#include "enums.h"
#include "listeners.h"

namespace AdMobExtension
{
typedef void (*PostCommandFn)(int id);
void QueueCommand(int id, int message, int firebase_result, const char* firebase_message, PostCommandFn fn);
void QueueRewardCommand(int id, int message, float reward, const char* reward_type);
}

static void DeleteAdRequest(firebase::admob::AdRequest& adrequest);
static void OnDestroyedCallback(const firebase::Future<void>& future, void* user_data);

namespace
{

struct LuaCallbackInfo
{
    LuaCallbackInfo() : m_L(0), m_Callback(LUA_NOREF), m_Self(LUA_NOREF) {}
    lua_State* m_L;
    int        m_Callback;
    int        m_Self;
};

struct MessageCommand
{
    AdMobExtension::PostCommandFn m_PostFn;     // A function to be called after the command was processed
    char* m_FirebaseMessage;    // Firebase error message or reward type
    int m_Id;
    int m_Message;
    int m_FirebaseResult;
    float m_Reward;
};

struct AdMobAd
{
    AdMobExtension::AdMobAdType m_Type;
    firebase::admob::AdRequest  m_AdRequest;
    LuaCallbackInfo             m_Callback;
    const char*                 m_AdUnit;
    uint8_t                     m_Initialized;
    uint8_t                     m_DelayedDelete;

    // Set to non zero depending on ad type
    firebase::admob::BannerView*            m_BannerView;
    firebase::admob::InterstitialAd*        m_InterstitialAd;
    firebase::admob::NativeExpressAdView*   m_NativeExpressAdView;
    // Listeners
    AdMobExtension::BannerViewListener*             m_BannerViewListener;
    AdMobExtension::InterstitialAdListener*         m_InterstitialAdListener;
    AdMobExtension::RewardedVideoListener*          m_RewardedVideoListener;
    AdMobExtension::NativeExpressAdViewListener*    m_NativeExpressAdViewListener;

    AdMobAd()
    {
        memset(this, 0, sizeof(*this));
        m_Callback.m_Callback = LUA_NOREF;
        m_Callback.m_Self = LUA_NOREF;
    }

    void Delete()
    {
        // Only because the Firebase SDK cannot delete the pointers correctly
        if( m_DelayedDelete == 1 )
        {
            return;
        }
        if( m_DelayedDelete == 2 )
        {
#if defined(DM_PLATFORM_ANDROID)
            if( m_BannerView )
                delete m_BannerView;

            if( m_NativeExpressAdView )
                delete m_NativeExpressAdView;
#endif
        }
        else if( m_BannerView != 0 )
        {
            m_BannerView->SetListener(0);
            delete m_BannerViewListener;

            //delete m_BannerView; // The Firebase C++ examples says: "delete ptr", but it crashes on iOS
            //m_BannerView = 0;

            m_DelayedDelete = 1;
#if defined(DM_PLATFORM_ANDROID) // Due to the non working functionality on iOS
            m_BannerView->Destroy();
            m_BannerView->DestroyLastResult().OnCompletion(OnDestroyedCallback, this);
#else
            m_DelayedDelete = 2;
            m_BannerView->Hide(); // Hack
#endif
            return;
        }
        else if( m_NativeExpressAdView != 0 )
        {
            //delete m_NativeExpressAdView; // The Firebase C++ examples says: "delete ptr", but it crashes on iOS
            //m_NativeExpressAdView = 0;

            m_NativeExpressAdView->SetListener(0);
            delete m_NativeExpressAdViewListener;

            m_DelayedDelete = 1;
#if defined(DM_PLATFORM_ANDROID) // Due to the non working functionality on iOS
            m_NativeExpressAdView->Destroy();
            m_NativeExpressAdView->DestroyLastResult().OnCompletion(OnDestroyedCallback, this);
#else
            m_DelayedDelete = 2;
            m_NativeExpressAdView->Hide(); // Hack
#endif
            return;
        }
        else if( m_InterstitialAd != 0 )
        {
            m_InterstitialAd->SetListener(0);
            delete m_InterstitialAd;

            if( m_InterstitialAdListener )
                delete m_InterstitialAdListener;
        }

        if( m_RewardedVideoListener )
        {
            firebase::admob::rewarded_video::SetListener(0);
            delete m_RewardedVideoListener;
        }


        if( m_AdUnit )
            free((void*)m_AdUnit);
        
        DeleteAdRequest(m_AdRequest);

        memset(this, 0, sizeof(*this));
        m_Callback.m_Callback = LUA_NOREF;
        m_Callback.m_Self = LUA_NOREF;
    }
};

const int ADMOB_MAX_ADS = 4; // 4 types

struct AdMobState
{
    AdMobAd         m_Ads[ADMOB_MAX_ADS];
    firebase::App*  m_App;
    int             m_CoveringUIAd;         // Which ad went fullscreen?

    dmArray<MessageCommand> m_CmdQueue;
};

} // namespace

::AdMobState* g_AdMob = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LUA helpers

static void DeleteAdRequest(firebase::admob::AdRequest& adrequest)
{
    for( uint32_t i = 0; i < adrequest.keyword_count; ++i)
    {
        free((void*)adrequest.keywords[i]);
    }
    if(adrequest.keywords != 0)
    {
        free((void*)adrequest.keywords);
        adrequest.keywords = 0;
    }

    for( uint32_t i = 0; i < adrequest.test_device_id_count; ++i)
    {
        free((void*)adrequest.test_device_ids[i]);
    }
    if(adrequest.test_device_ids != 0)
    {
        free((void*)adrequest.test_device_ids);
        adrequest.test_device_ids = 0;
    }

    for( uint32_t i = 0; i < adrequest.extras_count; ++i)
    {
        free((void*)adrequest.extras[i].key);
        free((void*)adrequest.extras[i].value);
    }
    if(adrequest.extras != 0)
    {
        free((void*)adrequest.extras);
        adrequest.extras = 0;
    }
    memset(&adrequest, 0, sizeof(adrequest));
}

// http://www.defold.com/ref/dmScript/#dmScript::GetMainThread
static void RegisterCallback(lua_State* L, int index, LuaCallbackInfo* cbk)
{
    if(cbk->m_Callback != LUA_NOREF)
    {
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
    }

    cbk->m_L = dmScript::GetMainThread(L);
    luaL_checktype(L, index, LUA_TFUNCTION);

    lua_pushvalue(L, index);
    cbk->m_Callback = dmScript::Ref(L, LUA_REGISTRYINDEX);

    dmScript::GetInstance(L);
    cbk->m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);
}

static void UnregisterCallback(LuaCallbackInfo* cbk)
{
    if(cbk->m_Callback != LUA_NOREF)
    {
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
        cbk->m_Callback = LUA_NOREF;
    }
}

static void InvokeCallback(LuaCallbackInfo* cbk, MessageCommand* cmd)
{
    if(cbk->m_Callback == LUA_NOREF)
    {
        return;
    }

    lua_State* L = cbk->m_L;
    DM_LUA_STACK_CHECK(L, 0);

    lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Callback);

    // Setup self (the script instance)
    lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Self);
    lua_pushvalue(L, -1);

    dmScript::SetInstance(L);

    ::AdMobAd* ad = &g_AdMob->m_Ads[cmd->m_Id];

    lua_newtable(L);

        lua_pushnumber(L, ad->m_Type);
        lua_setfield(L, -2, "type");

        lua_pushstring(L, ad->m_AdUnit);
        lua_setfield(L, -2, "ad_unit");

        lua_pushnumber(L, cmd->m_Message);
        lua_setfield(L, -2, "message");

        if( cmd->m_Message != AdMobExtension::ADMOB_MESSAGE_REWARD )
        {        
            lua_pushnumber(L, cmd->m_FirebaseResult);
            lua_setfield(L, -2, "result");

            lua_pushstring(L, cmd->m_FirebaseMessage ? cmd->m_FirebaseMessage : "");
            lua_setfield(L, -2, "result_string");
        }
        else
        {
            lua_pushnumber(L, cmd->m_Reward);
            lua_setfield(L, -2, "reward");

            lua_pushstring(L, cmd->m_FirebaseMessage ? cmd->m_FirebaseMessage : "");
            lua_setfield(L, -2, "reward_type");
        }

    int number_of_arguments = 2; // instance + 1
    int ret = lua_pcall(L, number_of_arguments, 0, 0);
    if(ret != 0) {
        dmLogError("Error running callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

// Gets a number (or a default value) from a table
static int CheckTableNumber(lua_State* L, int index, const char* name, int default_value)
{
    DM_LUA_STACK_CHECK(L, 0);

    int result = -1;
    lua_pushstring(L, name);
    lua_gettable(L, index);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return default_value;
    }
    else if (lua_isnumber(L, -1)) {
        result = lua_tointeger(L, -1);
    } else {
        return DM_LUA_ERROR("Wrong type for table attribute '%s'. Expected number, got %s", name, luaL_typename(L, -1));
    }
    lua_pop(L, 1);
    return result;
}

// Gets a list of strings from a table
static void CheckTableStringList(lua_State* L, int index, char*** outlist, uint32_t* length )
{
    DM_LUA_STACK_CHECK(L, 0);
    if( !lua_istable(L, index) )
    {
        *outlist = 0;
        *length = 0;
        return;
    }
    
    int len = lua_objlen(L, index);
    char** list = (char**)malloc(sizeof(char*) * len);

    lua_pushvalue(L, index); // push table
    lua_pushnil(L);  // first key

    int i = 0;
    while (lua_next(L, -2) != 0)
    {
        const char* s = lua_tostring(L, -1);
        if (!s) {
            DM_LUA_ERROR("Wrong type for table attribute '%s'. Expected string, got %s", lua_tostring(L, -2), luaL_typename(L, -1) );
            return;
        }
        list[i++] = strdup(s);

        // removes 'value'; keeps 'key' for next iteration
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop table

    *outlist = list;
    *length = (uint32_t)len;
}


// Gets a list of strings from a table
static void CheckTableKeyValueList(lua_State* L, int index, firebase::admob::KeyValuePair** outlist, uint32_t* length )
{
    DM_LUA_STACK_CHECK(L, 0);
    if( !lua_istable(L, index) )
    {
        *outlist = 0;
        *length = 0;
        return;
    }
    
    int len = lua_objlen(L, index);
    firebase::admob::KeyValuePair* list = (firebase::admob::KeyValuePair*)malloc(sizeof(firebase::admob::KeyValuePair) * len);

    lua_pushvalue(L, index); // push table
    lua_pushnil(L);  // first key

    int i = 0;
    while (lua_next(L, -2) != 0)
    {
        const char* k = lua_tostring(L, -2);
        const char* s = lua_tostring(L, -1);
        if (!s) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Wrong type for table attribute '%s'. Expected string, got %s", lua_tostring(L, -2), luaL_typename(L, -1) );
            luaL_error(L, msg);
            return;
        }
        list[i].key = strdup(k);
        list[i].value = strdup(s);
        i++;

        // removes 'value'; keeps 'key' for next iteration
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop table

    *outlist = list;
    *length = (uint32_t)len;
}

static void SetupAdRequest(lua_State* L, int index, firebase::admob::AdRequest& adrequest)
{
    DM_LUA_STACK_CHECK(L, 0);

    memset(&adrequest, 0, sizeof(adrequest));

    adrequest.birthday_day = CheckTableNumber(L, index, "birthday_day", 1);
    adrequest.birthday_month = CheckTableNumber(L, index, "birthday_month", 1);
    adrequest.birthday_year = CheckTableNumber(L, index, "birthday_year", 1970);
    adrequest.gender = (firebase::admob::Gender)CheckTableNumber(L, index, "gender", AdMobExtension::ADMOB_GENDER_UNKNOWN);
    adrequest.tagged_for_child_directed_treatment = (firebase::admob::ChildDirectedTreatmentState)CheckTableNumber(L, index,
                                                            "tagged_for_child_directed_treatment", AdMobExtension::ADMOB_CHILDDIRECTED_TREATMENT_STATE_NOT_TAGGED);

    lua_pushvalue(L, index); // push table
    lua_pushnil(L);  // first key
    while (lua_next(L, -2) != 0)
    {
        const char* key = lua_tostring(L, -2);

        if( strcmp("keywords", key) == 0 ) {
            CheckTableStringList(L, -1, (char***)&adrequest.keywords, &adrequest.keyword_count);
        }
        else if( strcmp("testdevices", key) == 0 ) {
            CheckTableStringList(L, -1, (char***)&adrequest.test_device_ids, &adrequest.test_device_id_count);
        }
        else if( strcmp("extras", key) == 0 ) {
            CheckTableKeyValueList(L, -1, (firebase::admob::KeyValuePair**)&adrequest.extras, &adrequest.extras_count);
        }

        // removes 'value'; keeps 'key' for next iteration
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop table
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace AdMobExtension
{

void QueueRewardCommand(int id, int message, float reward, const char* reward_type)
{
    MessageCommand cmd;
    cmd.m_Id = id;
    cmd.m_Message = message;
    cmd.m_FirebaseResult = 0;
    cmd.m_FirebaseMessage = reward_type ? strdup(reward_type) : 0;
    cmd.m_PostFn = 0;
    cmd.m_Reward = reward;

    // TODO: Add mutex here
    if(g_AdMob->m_CmdQueue.Full())
    {
        g_AdMob->m_CmdQueue.OffsetCapacity(8);
    }
    g_AdMob->m_CmdQueue.Push(cmd);
}

void QueueCommand(int id, int message, int firebase_result, const char* firebase_message, PostCommandFn fn)
{
    MessageCommand cmd;
    cmd.m_Id = id;
    cmd.m_Message = message;
    cmd.m_FirebaseResult = firebase_result;
    cmd.m_FirebaseMessage = firebase_message ? strdup(firebase_message) : 0;
    cmd.m_PostFn = fn;
    cmd.m_Reward = 0;

    // TODO: Add mutex here
    if(g_AdMob->m_CmdQueue.Full())
    {
        g_AdMob->m_CmdQueue.OffsetCapacity(8);
    }
    g_AdMob->m_CmdQueue.Push(cmd);
}

static void FlushCommandQueue()
{
    for(uint32_t i = 0; i != g_AdMob->m_CmdQueue.Size(); ++i)
    {
        MessageCommand* cmd = &g_AdMob->m_CmdQueue[i];
        ::AdMobAd& ad = g_AdMob->m_Ads[cmd->m_Id];

        InvokeCallback(&ad.m_Callback, cmd);

        if( cmd->m_PostFn )
        {
            cmd->m_PostFn(cmd->m_Id);
        }

        if( cmd->m_FirebaseMessage )
            free(cmd->m_FirebaseMessage);
        cmd->m_FirebaseMessage = 0;
    }
    g_AdMob->m_CmdQueue.SetSize(0);
}

} // AdMobExtension

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void DeleteCommandCallback(int type)
{
    ::AdMobAd* ad = &g_AdMob->m_Ads[type];
    UnregisterCallback(&ad->m_Callback);
    ad->Delete();
}

static void OnDestroyedCallback(const firebase::Future<void>& future, void* user_data)
{
    ::AdMobAd* ad = (AdMobAd*)user_data;

    switch(ad->m_Type)
    {
    case AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS:
    case AdMobExtension::ADMOB_TYPE_BANNER:
        ad->m_DelayedDelete = 2;
        break;
    default:
        return;
    }
}

static void OnLoadedCallback(const firebase::Future<void>& future, void* user_data)
{
    ::AdMobAd* ad = (AdMobAd*)user_data;
    if (future.error() != firebase::admob::kAdMobErrorNone)
    {
        QueueCommand(ad->m_Type, AdMobExtension::ADMOB_MESSAGE_FAILED_TO_LOAD, future.error(), future.error_message(), DeleteCommandCallback);
        return;
    }

    ad->m_Initialized = 1;
    switch(ad->m_Type)
    {
    case AdMobExtension::ADMOB_TYPE_BANNER:
        ad->m_BannerViewListener = new AdMobExtension::BannerViewListener(&g_AdMob->m_CoveringUIAd, ad->m_Type);
        ad->m_BannerView->SetListener(ad->m_BannerViewListener);
        break;
    case AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS:
        ad->m_NativeExpressAdViewListener = new AdMobExtension::NativeExpressAdViewListener(&g_AdMob->m_CoveringUIAd, ad->m_Type);
        ad->m_NativeExpressAdView->SetListener(ad->m_NativeExpressAdViewListener);
        break;
    case AdMobExtension::ADMOB_TYPE_INTERSTITIAL:
        ad->m_InterstitialAdListener = new AdMobExtension::InterstitialAdListener(&g_AdMob->m_CoveringUIAd, ad->m_Type);
        ad->m_InterstitialAd->SetListener(ad->m_InterstitialAdListener);
        break;
    case AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO:
        ad->m_RewardedVideoListener = new AdMobExtension::RewardedVideoListener(&g_AdMob->m_CoveringUIAd, ad->m_Type);
        firebase::admob::rewarded_video::SetListener(ad->m_RewardedVideoListener);
        break;
    default:
        return;
    }

    QueueCommand(ad->m_Type, AdMobExtension::ADMOB_MESSAGE_LOADED, future.error(), future.error_message(), 0);
}

static void OnCompletionCallback(const firebase::Future<void>& future, void* user_data)
{
    ::AdMobAd* ad = (AdMobAd*)user_data;

    if (future.error() != firebase::admob::kAdMobErrorNone)
    {
        QueueCommand(ad->m_Type, AdMobExtension::ADMOB_MESSAGE_FAILED_TO_LOAD, future.error(), future.error_message(), DeleteCommandCallback);
        return;
    }

    switch(ad->m_Type)
    {
    case AdMobExtension::ADMOB_TYPE_BANNER:
            ad->m_BannerView->LoadAd(ad->m_AdRequest);
            ad->m_BannerView->LoadAdLastResult().OnCompletion(OnLoadedCallback, ad);
            return;

    case AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS:
            ad->m_NativeExpressAdView->LoadAd(ad->m_AdRequest);
            ad->m_NativeExpressAdView->LoadAdLastResult().OnCompletion(OnLoadedCallback, ad);
            return;

    case AdMobExtension::ADMOB_TYPE_INTERSTITIAL:
            ad->m_InterstitialAd->LoadAd(ad->m_AdRequest);
            ad->m_InterstitialAd->LoadAdLastResult().OnCompletion(OnLoadedCallback, ad);
            return;

    case AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO:
            firebase::admob::rewarded_video::LoadAd(ad->m_AdUnit, ad->m_AdRequest);
            firebase::admob::rewarded_video::LoadAdLastResult().OnCompletion(OnLoadedCallback, ad);
            return;
    default:
        break;
    }
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Glue functions

#if defined(DM_PLATFORM_IOS)
static inline firebase::admob::AdParent GetAdParent()
{
    return (firebase::admob::AdParent)(id)dmGraphics::GetNativeiOSUIView();
}
#else
static inline firebase::admob::AdParent GetAdParent()
{
    return (firebase::admob::AdParent)(jobject)dmGraphics::GetNativeAndroidActivity();
}
static JNIEnv* GetJNIEnv()
{
    JNIEnv* env = 0;
    dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
    return env;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lua implementation

////////////////////////////////////////////////////////
// BANNER

static int BannerLoad(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER];
    if(ad->m_Initialized != 0)
        return luaL_error(L, "Ad is still loaded! Call admob.banner_unload() first");

    const char* ad_unit = luaL_checkstring(L, 1);
    ad->m_AdUnit = strdup(ad_unit);
    SetupAdRequest(L, 2, ad->m_AdRequest);
    RegisterCallback(L, 3, &ad->m_Callback);

    firebase::admob::AdSize ad_size;
    ad_size.ad_size_type = firebase::admob::kAdSizeStandard;
    ad_size.width = CheckTableNumber(L, 2, "width", 320);
    ad_size.height = CheckTableNumber(L, 2, "height", 100);

    ad->m_Type = AdMobExtension::ADMOB_TYPE_BANNER;
    ad->m_BannerView = new firebase::admob::BannerView();
    ad->m_BannerView->Initialize(GetAdParent(), ad->m_AdUnit, ad_size);
    ad->m_BannerView->InitializeLastResult().OnCompletion(OnCompletionCallback, ad);
    return 0;
}

static int BannerShow(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    ad->m_BannerView->Show();
    return 0;
}

static int BannerHide(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    ad->m_BannerView->Hide();
    return 0;
}

static int BannerMoveTo(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");

    assert( (int)firebase::admob::BannerView::kPositionTop == (int)firebase::admob::NativeExpressAdView::kPositionTop );
    assert( (int)firebase::admob::BannerView::kPositionBottom == (int)firebase::admob::NativeExpressAdView::kPositionBottom );
    assert( (int)firebase::admob::BannerView::kPositionTopLeft == (int)firebase::admob::NativeExpressAdView::kPositionTopLeft );
    assert( (int)firebase::admob::BannerView::kPositionTopRight == (int)firebase::admob::NativeExpressAdView::kPositionTopRight );
    assert( (int)firebase::admob::BannerView::kPositionBottomLeft == (int)firebase::admob::NativeExpressAdView::kPositionBottomLeft );
    assert( (int)firebase::admob::BannerView::kPositionBottomRight == (int)firebase::admob::NativeExpressAdView::kPositionBottomRight );

    if(lua_gettop(L) == 1)
    {
        int _pos = luaL_checkint(L, 1);
        if( _pos < AdMobExtension::ADMOB_POSITION_TOP || _pos > AdMobExtension::ADMOB_POSITION_BOTTOMRIGHT )
            return luaL_error(L, "Invalid position: %d", _pos);

        ad->m_BannerView->MoveTo((firebase::admob::BannerView::Position)_pos);
    }
    else
    {
        int x = luaL_checkint(L, 1);
        int y = luaL_checkint(L, 2);
        ad->m_BannerView->MoveTo(x, y);
    }
    return 0;
}

static int BannerUnload(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    QueueCommand(AdMobExtension::ADMOB_TYPE_BANNER, AdMobExtension::ADMOB_MESSAGE_UNLOADED, 0, 0, DeleteCommandCallback);
    return 0;
}

////////////////////////////////////////////////////////
// NATIVE EXPRESS

static int NativeExpressLoad(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS];
    if(ad->m_Initialized != 0)
        return luaL_error(L, "Ad is still loaded! Call admob.nativeexpress_unload() first");

    const char* ad_unit = luaL_checkstring(L, 1);
    ad->m_AdUnit = strdup(ad_unit);
    SetupAdRequest(L, 2, ad->m_AdRequest);
    RegisterCallback(L, 3, &ad->m_Callback);

    firebase::admob::AdSize ad_size;
    ad_size.ad_size_type = firebase::admob::kAdSizeStandard;
    ad_size.width = CheckTableNumber(L, 2, "width", 320);
    ad_size.height = CheckTableNumber(L, 2, "height", 100);

    ad->m_Type = AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS;
    ad->m_NativeExpressAdView = new firebase::admob::NativeExpressAdView();
    ad->m_NativeExpressAdView->Initialize(GetAdParent(), ad->m_AdUnit, ad_size);
    ad->m_NativeExpressAdView->InitializeLastResult().OnCompletion(OnCompletionCallback, ad);
    return 0;
}

static int NativeExpressShow(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    ad->m_NativeExpressAdView->Show();
    return 0;
}

static int NativeExpressHide(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    ad->m_NativeExpressAdView->Hide();
    return 0;
}

static int NativeExpressMoveTo(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");

    assert( (int)firebase::admob::BannerView::kPositionTop == (int)firebase::admob::NativeExpressAdView::kPositionTop );
    assert( (int)firebase::admob::BannerView::kPositionBottom == (int)firebase::admob::NativeExpressAdView::kPositionBottom );
    assert( (int)firebase::admob::BannerView::kPositionTopLeft == (int)firebase::admob::NativeExpressAdView::kPositionTopLeft );
    assert( (int)firebase::admob::BannerView::kPositionTopRight == (int)firebase::admob::NativeExpressAdView::kPositionTopRight );
    assert( (int)firebase::admob::BannerView::kPositionBottomLeft == (int)firebase::admob::NativeExpressAdView::kPositionBottomLeft );
    assert( (int)firebase::admob::BannerView::kPositionBottomRight == (int)firebase::admob::NativeExpressAdView::kPositionBottomRight );

    if(lua_gettop(L) == 1)
    {
        int _pos = luaL_checkint(L, 1);
        if( _pos < AdMobExtension::ADMOB_POSITION_TOP || _pos > AdMobExtension::ADMOB_POSITION_BOTTOMRIGHT )
            return luaL_error(L, "Invalid position: %d", _pos);

        ad->m_NativeExpressAdView->MoveTo((firebase::admob::NativeExpressAdView::Position)_pos);
    }
    else
    {
        int x = luaL_checkint(L, 1);
        int y = luaL_checkint(L, 2);
        ad->m_NativeExpressAdView->MoveTo(x, y);
    }
    return 0;
}

static int NativeExpressUnload(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    QueueCommand(AdMobExtension::ADMOB_TYPE_NATIVEEXPRESS, AdMobExtension::ADMOB_MESSAGE_UNLOADED, 0, 0, DeleteCommandCallback);
    return 0;
}

////////////////////////////////////////////////////////
// INTERSTITIAL

static int InterstitialLoad(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_INTERSTITIAL];
    if(ad->m_Initialized != 0)
        return luaL_error(L, "Ad is still loaded! Call admob.nativeexpress_unload() first");

    const char* ad_unit = luaL_checkstring(L, 1);
    ad->m_AdUnit = strdup(ad_unit);
    SetupAdRequest(L, 2, ad->m_AdRequest);
    RegisterCallback(L, 3, &ad->m_Callback);

    ad->m_Type = AdMobExtension::ADMOB_TYPE_INTERSTITIAL;
    ad->m_InterstitialAd = new firebase::admob::InterstitialAd();
    ad->m_InterstitialAd->Initialize(GetAdParent(), ad->m_AdUnit);
    ad->m_InterstitialAd->InitializeLastResult().OnCompletion(OnCompletionCallback, ad);
    return 0;
}

static int InterstitialShow(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_INTERSTITIAL];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    ad->m_InterstitialAd->Show();
    return 0;
}

static int InterstitialUnload(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_INTERSTITIAL];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    QueueCommand(AdMobExtension::ADMOB_TYPE_INTERSTITIAL, AdMobExtension::ADMOB_MESSAGE_UNLOADED, 0, 0, DeleteCommandCallback);
    return 0;
}

////////////////////////////////////////////////////////
// REWARDED VIDEO

static int RewardedVideoLoad(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO];
    if(ad->m_Initialized != 0)
        return luaL_error(L, "Ad is still loaded! Call admob.nativeexpress_unload() first");

    const char* ad_unit = luaL_checkstring(L, 1);
    ad->m_AdUnit = strdup(ad_unit);
    SetupAdRequest(L, 2, ad->m_AdRequest);
    RegisterCallback(L, 3, &ad->m_Callback);

    ad->m_Type = AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO;
    firebase::admob::rewarded_video::InitializeLastResult().OnCompletion(OnCompletionCallback, ad);
    return 0;
}

static int RewardedVideoShow(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    firebase::admob::rewarded_video::Show(GetAdParent());
    return 0;
}

static int RewardedVideoUnload(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    ::AdMobAd* ad = &g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO];
    if(ad->m_Initialized == 0)
        return luaL_error(L, "Ad is not loaded!");
    QueueCommand(AdMobExtension::ADMOB_TYPE_REWARDEDVIDEO, AdMobExtension::ADMOB_MESSAGE_UNLOADED, 0, 0, DeleteCommandCallback);
    return 0;
}

////////////////////////////////////////////////////////

static const luaL_reg Module_methods[] =
{
    {"load_banner", BannerLoad},
    {"show_banner", BannerShow},
    {"hide_banner", BannerHide},
    {"move_banner", BannerMoveTo},
    {"unload_banner", BannerUnload},

    {"load_nativeexpress", NativeExpressLoad},
    {"show_nativeexpress", NativeExpressShow},
    {"hide_nativeexpress", NativeExpressHide},
    {"move_nativeexpress", NativeExpressMoveTo},
    {"unload_nativeexpress", NativeExpressUnload},

    {"load_interstitial", InterstitialLoad},
    {"show_interstitial", InterstitialShow},
    {"unload_interstitial", InterstitialUnload},

    {"load_rewardedvideo", RewardedVideoLoad},
    {"show_rewardedvideo", RewardedVideoShow},
    {"unload_rewardedvideo", RewardedVideoUnload},

    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name) \
        lua_pushnumber(L, (lua_Number) AdMobExtension::ADMOB_ ## name); \
        lua_setfield(L, -2, #name);\

    SETCONSTANT(ERROR_NONE);
    SETCONSTANT(ERROR_UNINITIALIZED);
    SETCONSTANT(ERROR_ALREADYINITIALIZED);
    SETCONSTANT(ERROR_LOADINPROGRESS);
    SETCONSTANT(ERROR_INTERNALERROR);
    SETCONSTANT(ERROR_INVALIDREQUEST);
    SETCONSTANT(ERROR_NETWORKERROR);
    SETCONSTANT(ERROR_NOFILL);
    SETCONSTANT(ERROR_NOWINDOWTOKEN);

    SETCONSTANT(TYPE_BANNER);
    SETCONSTANT(TYPE_INTERSTITIAL);
    SETCONSTANT(TYPE_REWARDEDVIDEO);
    SETCONSTANT(TYPE_NATIVEEXPRESS);

    SETCONSTANT(CHILDDIRECTED_TREATMENT_STATE_NOT_TAGGED);
    SETCONSTANT(CHILDDIRECTED_TREATMENT_STATE_TAGGED);
    SETCONSTANT(CHILDDIRECTED_TREATMENT_STATE_UNKNOWN);

    SETCONSTANT(GENDER_UNKNOWN);
    SETCONSTANT(GENDER_FEMALE);
    SETCONSTANT(GENDER_MALE);

    SETCONSTANT(POSITION_TOP);
    SETCONSTANT(POSITION_BOTTOM);
    SETCONSTANT(POSITION_TOPLEFT);
    SETCONSTANT(POSITION_TOPRIGHT);
    SETCONSTANT(POSITION_BOTTOMLEFT);
    SETCONSTANT(POSITION_BOTTOMRIGHT);

    SETCONSTANT(MESSAGE_LOADED);
    SETCONSTANT(MESSAGE_FAILED_TO_LOAD);
    SETCONSTANT(MESSAGE_SHOW);
    SETCONSTANT(MESSAGE_HIDE);
    SETCONSTANT(MESSAGE_REWARD);
    SETCONSTANT(MESSAGE_APP_LEAVE);
    SETCONSTANT(MESSAGE_UNLOADED);

#undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Extension interface functions

static dmExtension::Result AppInitializeExtension(dmExtension::AppParams* params)
{
    if (g_AdMob) {
        dmLogError("AdMob already initialized.");
        return dmExtension::RESULT_OK;
    }

    // Of course, it's possible to have an "admob.init(appid)" function, but quite often
    // it's nice to have the game projects change depending on what build you're using (DEV, QA, RELEASE etc)
#if defined(__ANDROID__)
    const char* app_id = dmConfigFile::GetString(params->m_ConfigFile, "admob.app_id_android", 0);
#else
    const char* app_id = dmConfigFile::GetString(params->m_ConfigFile, "admob.app_id_ios", 0);
#endif
    if( !app_id )
    {
        dmLogError("No admob.app_id set in game.project!");
        return dmExtension::RESULT_OK;
    }

#if defined(__ANDROID__)
    firebase::App* app = firebase::App::Create(firebase::AppOptions(), GetJNIEnv(), dmGraphics::GetNativeAndroidActivity());
#else
    firebase::App* app = firebase::App::Create(firebase::AppOptions());
#endif

    if(!app)
    {
        dmLogError("firebase::App::Create failed");
        return dmExtension::RESULT_OK;
    }

    firebase::InitResult res = firebase::admob::Initialize(*app, app_id);
    if (res != firebase::kInitResultSuccess)
    {
        delete app;
        dmLogError("Could not initialize AdMob, result: %d", res);
        return dmExtension::RESULT_OK;
    }

    firebase::admob::rewarded_video::Initialize();

    g_AdMob = new ::AdMobState;
    g_AdMob->m_App = app;
    g_AdMob->m_CoveringUIAd = -1;
    g_AdMob->m_CmdQueue.SetCapacity(8);

    dmLogInfo("AdMob fully initialized!");

    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeExtension(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    dmLogInfo("Registered %s Lua extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeExtension(dmExtension::AppParams* params)
{
    if( !g_AdMob )
        return dmExtension::RESULT_OK;

    for( uint32_t i = 0; i < ADMOB_MAX_ADS; ++i)
    {
        g_AdMob->m_Ads[i].Delete();
    }

    if(g_AdMob->m_App)
    {
        firebase::admob::rewarded_video::Destroy();
        firebase::admob::Terminate();
        delete g_AdMob->m_App;
    }

    delete g_AdMob;
    g_AdMob = 0;
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateExtension(dmExtension::Params* params)
{
    if( g_AdMob )
    {
        AdMobExtension::FlushCommandQueue();

        for(uint32_t i = 0; i < ADMOB_MAX_ADS; ++i)
        {
            ::AdMobAd* ad = &g_AdMob->m_Ads[i];
            if( ad->m_DelayedDelete )
            {
                ad->Delete();
            }
        }
    }
    return dmExtension::RESULT_OK;
}

static void OnEventExtension(dmExtension::Params* params, const dmExtension::Event* event)
{
    if( !g_AdMob )
        return;

    if( event->m_Event == dmExtension::EVENT_ID_ACTIVATEAPP )
    {
        g_AdMob->m_CoveringUIAd = -1;

        if(g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_BannerView)
            g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_BannerView->Resume();
        if(g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_NativeExpressAdView)
            g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_NativeExpressAdView->Resume();

        firebase::admob::rewarded_video::Resume();
    }
    else if(event->m_Event == dmExtension::EVENT_ID_DEACTIVATEAPP)
    {
        if( g_AdMob->m_CoveringUIAd != -1 )
        {
            AdMobExtension::FlushCommandQueue();
        }

        if(g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_BannerView)
            g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_BannerView->Pause();
        if(g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_NativeExpressAdView)
            g_AdMob->m_Ads[AdMobExtension::ADMOB_TYPE_BANNER].m_NativeExpressAdView->Pause();

        firebase::admob::rewarded_video::Pause();
    }
}

#else // DM_PLATFORM_IOS


static dmExtension::Result AppInitializeExtension(dmExtension::AppParams* params)
{
    dmLogWarning("Registered %s (null) Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static void OnEventExtension(dmExtension::Params* params, const dmExtension::Event* event)
{
}

#endif


DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeExtension, AppFinalizeExtension, InitializeExtension, UpdateExtension, OnEventExtension, FinalizeExtension)
