#define EXTENSION_NAME AdMobExt
#define LIB_NAME "Admob"
#define MODULE_NAME "admob"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)

namespace dmAdmob {

static dmExtension::Result AppInitializeAdmob(dmExtension::AppParams* params)
{
  return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAdmob(dmExtension::Params* params)
{
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
