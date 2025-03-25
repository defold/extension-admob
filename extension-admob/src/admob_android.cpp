#if defined(DM_PLATFORM_ANDROID)

#include <dmsdk/dlib/android.h>
#include "admob_private.h"
#include "com_defold_admob_AdmobJNI.h"
#include "admob_callback_private.h"

JNIEXPORT void JNICALL Java_com_defold_admob_AdmobJNI_admobAddToQueue(JNIEnv * env, jclass cls, jint jmsg, jstring jjson)
{
    const char* json = env->GetStringUTFChars(jjson, 0);
    dmAdmob::AddToQueueCallback((dmAdmob::MessageId)jmsg, json);
    env->ReleaseStringUTFChars(jjson, json);
}

namespace dmAdmob {

struct Admob
{
    jobject        m_AdmobJNI;

    jmethodID      m_Initialize;
    jmethodID      m_LoadAppOpen;
    jmethodID      m_ShowAppOpen;
    jmethodID      m_LoadInterstitial;
    jmethodID      m_ShowInterstitial;
    jmethodID      m_LoadRewarded;
    jmethodID      m_ShowRewarded;
    jmethodID      m_LoadRewardedInterstitial;
    jmethodID      m_ShowRewardedInterstitial;
    jmethodID      m_LoadBanner;
    jmethodID      m_DestroyBanner;
    jmethodID      m_ShowBanner;
    jmethodID      m_HideBanner;
    jmethodID      m_IsAppOpenLoaded;
    jmethodID      m_IsRewardedLoaded;
    jmethodID      m_IsInterstitialLoaded;
    jmethodID      m_IsRewardedInterstitialLoaded;
    jmethodID      m_IsBannerLoaded;
    jmethodID      m_SetPrivacySettings;
    jmethodID      m_RequestIDFA;
    jmethodID      m_ShowAdInspector;
    jmethodID      m_UpdateBannerLayout;
    jmethodID      m_SetMaxAdContentRating;

};

static Admob       g_admob;

static void CallVoidMethod(jobject instance, jmethodID method)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    env->CallVoidMethod(instance, method);
}

static bool CallBoolMethod(jobject instance, jmethodID method)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jboolean return_value = (jboolean)env->CallBooleanMethod(instance, method);
    return JNI_TRUE == return_value;
}

static void CallVoidMethodChar(jobject instance, jmethodID method, const char* cstr)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr);
    env->DeleteLocalRef(jstr);
}

static void CallVoidMethodCharInt(jobject instance, jmethodID method, const char* cstr, int cint)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr, cint);
    env->DeleteLocalRef(jstr);
}

static void CallVoidMethodCharBoolean(jobject instance, jmethodID method, const char* cstr, bool cbool)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr, cbool);
    env->DeleteLocalRef(jstr);
}

static void CallVoidMethodCharCharChar(jobject instance, jmethodID method, const char* cstr, const char* cstr2, const char* cstr3)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jstring jstr = env->NewStringUTF(cstr);
    jstring jstr2 = env->NewStringUTF(cstr2);
    jstring jstr3 = env->NewStringUTF(cstr3);
    env->CallVoidMethod(instance, method, jstr, jstr2, jstr3);
    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(jstr2);
    env->DeleteLocalRef(jstr3);
}

static void CallVoidMethodInt(jobject instance, jmethodID method, int cint)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    env->CallVoidMethod(instance, method, cint);
}

static void CallVoidMethodBool(jobject instance, jmethodID method, bool cbool)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    env->CallVoidMethod(instance, method, cbool);
}

static void InitJNIMethods(JNIEnv* env, jclass cls)
{
    g_admob.m_Initialize = env->GetMethodID(cls, "initialize", "()V");
    g_admob.m_LoadAppOpen = env->GetMethodID(cls, "loadAppOpen", "(Ljava/lang/String;)V");
    g_admob.m_ShowAppOpen = env->GetMethodID(cls, "showAppOpen", "()V");
    g_admob.m_LoadInterstitial = env->GetMethodID(cls, "loadInterstitial", "(Ljava/lang/String;)V");
    g_admob.m_ShowInterstitial = env->GetMethodID(cls, "showInterstitial", "()V");
    g_admob.m_LoadRewarded = env->GetMethodID(cls, "loadRewarded", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    g_admob.m_ShowRewarded = env->GetMethodID(cls, "showRewarded", "()V");
    g_admob.m_LoadRewardedInterstitial  = env->GetMethodID(cls, "loadRewardedInterstitial", "(Ljava/lang/String;)V");
    g_admob.m_ShowRewardedInterstitial  = env->GetMethodID(cls, "showRewardedInterstitial", "()V");
    g_admob.m_LoadBanner = env->GetMethodID(cls, "loadBanner", "(Ljava/lang/String;I)V");
    g_admob.m_DestroyBanner = env->GetMethodID(cls, "destroyBanner", "()V");
    g_admob.m_ShowBanner = env->GetMethodID(cls, "showBanner", "(I)V");
    g_admob.m_HideBanner = env->GetMethodID(cls, "hideBanner", "()V");
    g_admob.m_SetPrivacySettings = env->GetMethodID(cls, "setPrivacySettings", "(Z)V");
    g_admob.m_RequestIDFA = env->GetMethodID(cls, "requestIDFA", "()V");
    g_admob.m_ShowAdInspector = env->GetMethodID(cls, "showAdInspector", "()V");
    g_admob.m_UpdateBannerLayout= env->GetMethodID(cls, "updateBannerLayout", "()V");

    g_admob.m_IsAppOpenLoaded = env->GetMethodID(cls, "isAppOpenLoaded", "()Z");
    g_admob.m_IsRewardedLoaded = env->GetMethodID(cls, "isRewardedLoaded", "()Z");
    g_admob.m_IsInterstitialLoaded = env->GetMethodID(cls, "isInterstitialLoaded", "()Z");
    g_admob.m_IsRewardedInterstitialLoaded = env->GetMethodID(cls, "isRewardedInterstitialLoaded", "()Z");
    g_admob.m_IsBannerLoaded = env->GetMethodID(cls, "isBannerLoaded", "()Z");
    g_admob.m_SetMaxAdContentRating = env->GetMethodID(cls, "setMaxAdContentRating", "(I)V");
}

void Initialize_Ext(dmExtension::Params* params, const char* defoldUserAgent)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jclass cls = dmAndroid::LoadClass(env, "com.defold.admob.AdmobJNI");

    InitJNIMethods(env, cls);

    const char* appOpenAdUnitId = dmConfigFile::GetString(params->m_ConfigFile, "admob.app_open_android", 0);
    jstring jappOpenAdUnitId = env->NewStringUTF(appOpenAdUnitId);
    jstring jdefoldUserAgent = env->NewStringUTF(defoldUserAgent);
    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;)V");
    g_admob.m_AdmobJNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, threadAttacher.GetActivity()->clazz, jappOpenAdUnitId, jdefoldUserAgent));
    env->DeleteLocalRef(jappOpenAdUnitId);
    env->DeleteLocalRef(jdefoldUserAgent);
}

void Finalize_Ext()
{
}

void Initialize()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_Initialize);
}

void LoadAppOpen(const char* unitId, bool showImmediately)
{
    CallVoidMethodCharBoolean(g_admob.m_AdmobJNI, g_admob.m_LoadAppOpen, unitId, showImmediately);
}

void ShowAppOpen()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowAppOpen);
}

bool IsAppOpenLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsAppOpenLoaded);
}

void LoadInterstitial(const char* unitId)
{
    CallVoidMethodChar(g_admob.m_AdmobJNI, g_admob.m_LoadInterstitial, unitId);
}

void ShowInterstitial()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowInterstitial);
}

bool IsInterstitialLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsInterstitialLoaded);
}

void LoadRewarded(const char* unitId, const char* userId, const char* customData)
{
    CallVoidMethodCharCharChar(g_admob.m_AdmobJNI, g_admob.m_LoadRewarded, unitId, userId, customData);
}

void ShowRewarded()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowRewarded);
}

bool IsRewardedLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsRewardedLoaded);
}

void LoadRewardedInterstitial(const char* unitId)
{
    CallVoidMethodChar(g_admob.m_AdmobJNI, g_admob.m_LoadRewardedInterstitial, unitId);
}

void ShowRewardedInterstitial()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowRewardedInterstitial);
}

bool IsRewardedInterstitialLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsRewardedInterstitialLoaded);
}

void LoadBanner(const char* unitId, BannerSize bannerSize)
{
    CallVoidMethodCharInt(g_admob.m_AdmobJNI, g_admob.m_LoadBanner, unitId, (int)bannerSize);
}

void DestroyBanner()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_DestroyBanner);
}

void ShowBanner(BannerPosition bannerPos)
{
    CallVoidMethodInt(g_admob.m_AdmobJNI, g_admob.m_ShowBanner, (int)bannerPos);
}

void HideBanner()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_HideBanner);
}

bool IsBannerLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsBannerLoaded);
}

void SetPrivacySettings(bool enable_rdp)
{
    CallVoidMethodBool(g_admob.m_AdmobJNI, g_admob.m_SetPrivacySettings, enable_rdp);
}

void RequestIDFA()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_RequestIDFA);
}

void ShowAdInspector()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowAdInspector);
}

void ActivateApp()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_UpdateBannerLayout);
}

void SetMaxAdContentRating(MaxAdRating max_ad_rating)
{
    CallVoidMethodInt(g_admob.m_AdmobJNI, g_admob.m_SetMaxAdContentRating, (int)max_ad_rating);
}

}//namespace dmAdmob

#endif
