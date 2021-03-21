#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>

#include "admob_jni.h"
#include "private_admob.h"
#include "com_defold_admob_AdmobJNI.h"
#include "private_admob_callback.h"

JNIEXPORT void JNICALL Java_com_defold_admob_AdmobJNI_admobAddToQueue(JNIEnv * env, jclass cls, jint jmsg, jstring jjson)
{
    const char* json = env->GetStringUTFChars(jjson, 0);
    dmAdmob::AddToQueueCallback((dmAdmob::MESSAGE_ID)jmsg, json);
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
};

static Admob       g_admob;

static void CallVoidMethod(jobject instance, jmethodID method)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

    env->CallVoidMethod(instance, method);
}

static void CallVoidMethodChar(jobject instance, jmethodID method, const char* cstr)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr);
    env->DeleteLocalRef(jstr);
}

static void InitJNIMethods(JNIEnv* env, jclass cls)
{
    g_admob.m_Initialize = env->GetMethodID(cls, "initialize", "()V");
    g_admob.m_LoadInterstitial = env->GetMethodID(cls, "loadInterstitial", "(Ljava/lang/String;)V");
    g_admob.m_ShowInterstitial = env->GetMethodID(cls, "showInterstitial", "()V");
    g_admob.m_LoadRewarded = env->GetMethodID(cls, "loadRewarded", "(Ljava/lang/String;)V");
    g_admob.m_ShowRewarded = env->GetMethodID(cls, "showRewarded", "()V");
    
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

void LoadInterstitial(const char* placementId)
{
    CallVoidMethodChar(g_admob.m_AdmobJNI, g_admob.m_LoadInterstitial, placementId);
}

void ShowInterstitial()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowInterstitial);
}

void LoadRewarded(const char* placementId)
{
    CallVoidMethodChar(g_admob.m_AdmobJNI, g_admob.m_LoadRewarded, placementId);
}

void ShowRewarded()
{
    CallVoidMethod(g_admob.m_AdmobJNI, g_admob.m_ShowRewarded);
}

}//namespace dmAdmob

#endif
