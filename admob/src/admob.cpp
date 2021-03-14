#define EXTENSION_NAME AdMobExt
#define LIB_NAME "Admob"
#define MODULE_NAME "admob"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#include "admob_jni.h"

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)

namespace dmAdmob {

struct Admob
{
    jobject        m_AdmobJNI;
};

static Admob       g_admob;

static void InitJNIMethods(JNIEnv* env, jclass cls)
{

}

static void InitializeJNI()
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    ClassLoader class_loader = ClassLoader(env);
    jclass cls = class_loader.load("com.defold.admob.AdmobJNI");

    InitJNIMethods(env, cls);

    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");

    g_admob.m_AdmobJNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, dmGraphics::GetNativeAndroidActivity()));
}

static dmExtension::Result AppInitializeAdmob(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAdmob(dmExtension::Params* params)
{
    InitializeJNI();
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

#endif
