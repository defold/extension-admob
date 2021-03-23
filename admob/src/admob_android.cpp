#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>

#include "admob_jni.h"
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
    jmethodID      m_LoadInterstitial;
    jmethodID      m_ShowInterstitial;
    jmethodID      m_LoadRewarded;
    jmethodID      m_ShowRewarded;
    jmethodID      m_LoadBanner;
    jmethodID      m_UnloadBanner;
    jmethodID      m_ShowBanner;
    jmethodID      m_HideBanner;
    jmethodID      m_IsRewardedLoaded;
    jmethodID      m_IsInterstitialLoaded;
    jmethodID      m_IsBannerLoaded;

};

static Admob       g_admob;

static void CallVoidMethod(jobject instance, jmethodID method)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

    env->CallVoidMethod(instance, method);
}

static bool CallBoolMethod(jobject instance, jmethodID method)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

    jboolean return_value = (jboolean)env->CallBooleanMethod(instance, method);
    return JNI_TRUE == return_value;
}

static void CallVoidMethodChar(jobject instance, jmethodID method, const char* cstr)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr);
    env->DeleteLocalRef(jstr);
}

static void CallVoidMethodCharInt(jobject instance, jmethodID method, const char* cstr, BannerSize bannerSize)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr, (int)bannerSize);
    env->DeleteLocalRef(jstr);
}

static void InitJNIMethods(JNIEnv* env, jclass cls)
{
    g_admob.m_Initialize = env->GetMethodID(cls, "initialize", "()V");
    g_admob.m_LoadInterstitial = env->GetMethodID(cls, "loadInterstitial", "(Ljava/lang/String;)V");
    g_admob.m_ShowInterstitial = env->GetMethodID(cls, "showInterstitial", "()V");
    g_admob.m_LoadRewarded = env->GetMethodID(cls, "loadRewarded", "(Ljava/lang/String;)V");
    g_admob.m_ShowRewarded = env->GetMethodID(cls, "showRewarded", "()V");
    g_admob.m_LoadBanner = env->GetMethodID(cls, "loadBanner", "(Ljava/lang/String;I)V");
    g_admob.m_UnloadBanner = env->GetMethodID(cls, "unloadBanner", "()V");
    g_admob.m_ShowBanner = env->GetMethodID(cls, "showBanner", "()V");
    g_admob.m_HideBanner = env->GetMethodID(cls, "hideBanner", "()V");

    g_admob.m_IsRewardedLoaded = env->GetMethodID(cls, "isRewardedLoaded", "()Z");
    g_admob.m_IsInterstitialLoaded = env->GetMethodID(cls, "isInterstitialLoaded", "()Z");
    g_admob.m_IsBannerLoaded = env->GetMethodID(cls, "isBannerLoaded", "()Z");
}

void Initialize_Ext()
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    ClassLoader class_loader = ClassLoader(env);
    jclass cls = class_loader.load("com.defold.admob.AdmobJNI");

    InitJNIMethods(env, cls);

    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");

    g_admob.m_AdmobJNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, dmGraphics::GetNativeAndroidActivity()));
}

void Initialize()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_Initialize);
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

void LoadRewarded(const char* unitId)
{
    CallVoidMethodChar(g_admob.m_AdmobJNI, g_admob.m_LoadRewarded, unitId);
}

void ShowRewarded()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowRewarded);
}

bool IsRewardedLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsRewardedLoaded);
}

void LoadBanner(const char* unitId, BannerSize bannerSize)
{
    CallVoidMethodCharInt(g_admob.m_AdmobJNI, g_admob.m_LoadBanner, unitId, bannerSize);
}

void UnloadBanner()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_UnloadBanner);
}

void ShowBanner()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowBanner);
}

void HideBanner()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_HideBanner);
}

bool IsBannerLoaded()
{
    return CallBoolMethod(g_admob.m_AdmobJNI, g_admob.m_IsBannerLoaded);
}

}//namespace dmAdmob

#endif
