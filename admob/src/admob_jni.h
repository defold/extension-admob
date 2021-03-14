#if defined(DM_PLATFORM_ANDROID)
#include <dmsdk/sdk.h>

namespace dmAdmob {

struct ThreadAttacher {
	JNIEnv *env;
	bool has_attached;
	ThreadAttacher() : env(NULL), has_attached(false) {
		if (dmGraphics::GetNativeAndroidJavaVM()->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
			dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
			has_attached = true;
		}
	}
	~ThreadAttacher() {
		if (has_attached) {
			if (env->ExceptionCheck()) {
				env->ExceptionDescribe();
			}
			env->ExceptionClear();
			dmGraphics::GetNativeAndroidJavaVM()->DetachCurrentThread();
		}
	}
};

struct ClassLoader {
	private:
	JNIEnv *env;
	jobject class_loader_object;
	jmethodID find_class;
	public:
	ClassLoader(JNIEnv *env) : env(env) {
		jclass activity_class = env->FindClass("android/app/NativeActivity");
		jmethodID get_class_loader = env->GetMethodID(activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
		class_loader_object = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
		jclass class_loader = env->FindClass("java/lang/ClassLoader");
		find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
		env->DeleteLocalRef(activity_class);
		env->DeleteLocalRef(class_loader);
	}
	~ClassLoader() {
		env->DeleteLocalRef(class_loader_object);
	}
	jclass load(const char *class_name) {
		jstring str_class_name = env->NewStringUTF(class_name);
		jclass loaded_class = (jclass)env->CallObjectMethod(class_loader_object, find_class, str_class_name);
		env->DeleteLocalRef(str_class_name);
		return loaded_class;
	}
};

}//namespace dmAdmob

#endif
