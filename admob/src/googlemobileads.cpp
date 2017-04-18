
/*
https://github.com/yinjimmy/firebase
*/


#define EXTENSION_NAME AdMob
#define MODULE_NAME "admobex"
#define LIB_NAME "AdMobExample"

// Defold SDK
#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)

// https://github.com/googleads/googleads-mobile-ios-examples
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#include <dmsdk/script/script.h>  // DM_LUA_STACK_CHECK, CheckHashOrString, PushBuffer, Ref, Unref"


#include "firebase/admob.h"
#include "firebase/admob/banner_view.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/native_express_ad_view.h"
#include "firebase/admob/rewarded_video.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"


firebase::App* g_App = NULL;
firebase::admob::BannerView* g_BannerView = NULL;
firebase::admob::InterstitialAd* g_InterstitialAd = NULL;
firebase::admob::AdRequest g_AdRequest = {0};


static void SetupAdRequest()
{
    // If the app is aware of the user's gender, it can be added to the
    // targeting information. Otherwise, "unknown" should be used.
    g_AdRequest.gender = firebase::admob::kGenderMale;

    // The user's birthday, if known. Note that months are indexed from one.
    g_AdRequest.birthday_day = 10;
    g_AdRequest.birthday_month = 11;
    g_AdRequest.birthday_year = 1986;

    // Additional keywords to be used in targeting.
    static const char* kKeywords[] = {"development", "games", "fun"};
    g_AdRequest.keyword_count = sizeof(kKeywords) / sizeof(kKeywords[0]);
    g_AdRequest.keywords = kKeywords;

    // Uncomment this to use test ads:
    static const char* kTestDeviceIDs[] =
    {   "199e2e182e2f89a2c22a47e03e048772",
        "ea71229c04c0e221e30a2be8a076f1aa",
        "d6fafe1dc659201e741c1dfdf00223fc", //DVS-1190,
        "8497A7462C631CD09AA9DDD6C1D23B0F", //DVS-1004,
        "ED455496C7B5BC4ADF3137A45385199B", //Nexus 6P, Android 7.0
    };
    g_AdRequest.test_device_id_count = sizeof(kTestDeviceIDs) / sizeof(kTestDeviceIDs[0]);
    g_AdRequest.test_device_ids = kTestDeviceIDs;
}

static void BannerOnCompletionCallback(const firebase::Future<void>& future, void* user_data) {
    printf("BannerOnCompletionCallback\n");
    firebase::admob::BannerView *banner_view = static_cast<firebase::admob::BannerView*>(user_data);
    if (future.Error() == firebase::admob::kAdMobErrorNone) {
        printf("No Error!\n");
        banner_view->LoadAd(g_AdRequest);
    }
}

static void InterstitialOnCompletionCallback(const firebase::Future<void>& future, void* user_data) {
    printf("InterstitialOnCompletionCallback\n");
    firebase::admob::InterstitialAd *interstitial_ad = static_cast<firebase::admob::InterstitialAd*>(user_data);
    if (future.Error() == firebase::admob::kAdMobErrorNone) {
        printf("No Error!\n");
        interstitial_ad->LoadAd(g_AdRequest);
    }
}

static int LoadBannerAd(lua_State* L)
{
    if (!g_App) {
        return luaL_error(L, "AdMob not initialized.");
    }

    SetupAdRequest();

    const char* ad_unit = luaL_checkstring(L, 1);

    firebase::admob::AdSize ad_size;
    ad_size.ad_size_type = firebase::admob::kAdSizeStandard;
    ad_size.width = luaL_checknumber(L, 2);
    ad_size.height = luaL_checknumber(L, 3);

    g_BannerView = new firebase::admob::BannerView();
    g_BannerView->Initialize(static_cast<firebase::admob::AdParent>((id)dmGraphics::GetNativeiOSUIView()), ad_unit, ad_size);
    g_BannerView->InitializeLastResult().OnCompletion(BannerOnCompletionCallback, g_BannerView);

    return 0;
}

static int LoadInterstitialAd(lua_State* L)
{
    if (!g_App) {
        return luaL_error(L, "AdMob not initialized.");
    }

    SetupAdRequest();

    const char* ad_unit = luaL_checkstring(L, 1);

    g_InterstitialAd = new firebase::admob::InterstitialAd();
    g_InterstitialAd->Initialize(static_cast<firebase::admob::AdParent>((id)dmGraphics::GetNativeiOSUIView()), ad_unit);
    g_InterstitialAd->InitializeLastResult().OnCompletion(InterstitialOnCompletionCallback, g_InterstitialAd);

    return 0;
}

static int ShowBannerAd(lua_State* L)
{
    g_BannerView->Show();
    return 0;
}

static int ShowInterstitialAd(lua_State* L)
{
    g_InterstitialAd->Show();
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"load_banner_ad", LoadBannerAd},
    {"show_banner_ad", ShowBannerAd},
    {"load_interstitial_ad", LoadInterstitialAd},
    {"show_interstitial_ad", ShowInterstitialAd},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

 
#if defined(DM_PLATFORM_ANDROID)
static JNIEnv* GetJNIEnv()
{
    JNIEnv* env = 0;
    dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
    return env;
}
#endif

static dmExtension::Result AppInitializeExtension(dmExtension::AppParams* params)
{
    if (g_App) {
        dmLogError("g_App already initialized.");
        return dmExtension::RESULT_OK;
    }

    const char* app_id = dmConfigFile::GetString(params->m_ConfigFile, "admob.app_id", 0);
    if( !app_id )
    {
        dmLogError("No admob.app_id set in game.project!");
        return dmExtension::RESULT_OK;
    }

    if (g_App) {
        dmLogError("AdMob already initialized.");
        return dmExtension::RESULT_OK;
    } else {
#if defined(__ANDROID__)
dmLogWarning("MAWE: ::firebase::App::Create()\n");
        g_App = ::firebase::App::Create(::firebase::AppOptions(), GetJNIEnv(), dmGraphics::GetNativeAndroidActivity());
#else
        g_App = ::firebase::App::Create(::firebase::AppOptions());
#endif  // defined(__ANDROID__)

dmLogWarning("MAWE: firebase::admob::Initialize()\n");
        firebase::InitResult res = firebase::admob::Initialize(*g_App, app_id);
        if (!g_App || res != firebase::kInitResultSuccess)
        {
            g_App = NULL;
            dmLogError("Could not initialize AdMob, result: %d", res);
            return dmExtension::RESULT_OK;
        }
    }

    dmLogInfo("AdMob fully initialized!");

    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeExtension(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    printf("Registered %s Lua extension\n", MODULE_NAME);
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


#else // DM_PLATFORM_IOS


static dmExtension::Result AppInitializeExtension(dmExtension::AppParams* params)
{
    dmLogInfo("Registered %s (null) Extension\n", MODULE_NAME);
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

#endif


DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeExtension, AppFinalizeExtension, InitializeExtension, 0, 0, FinalizeExtension)
