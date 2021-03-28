#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>
/* Header for class com_defold_admob_AdmobJNI */

#ifndef COM_DEFOLD_ADMOB_ADMOBJNI_H
#define COM_DEFOLD_ADMOB_ADMOBJNI_H
#ifdef __cplusplus
extern "C" {
#endif
	/*
	* Class:     com_defold_admob_AdmobJNI
	* Method:    admobAddToQueue_first_arg
	* Signature: (ILjava/lang/String;I)V
	*/
	JNIEXPORT void JNICALL Java_com_defold_admob_AdmobJNI_admobAddToQueue
		(JNIEnv *, jclass, jint, jstring);

#ifdef __cplusplus
}
#endif
#endif

#endif
