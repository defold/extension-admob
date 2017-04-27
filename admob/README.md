# How the AdMob example was setup

In case you'd like to do this from scratch, here are some notes about the setup of this library

## Firebase C++ sdk

The C++ libraries were taken from the [Firebase C++ sdk setup page](https://firebase.google.com/docs/cpp/setup)

	E.g. https://dl.google.com/firebase/sdk/cpp/firebase_cpp_sdk_3.1.0.zip

Also, checkout the [Firebase C++ Reference](https://firebase.google.com/docs/reference/cpp/)

### Android

A lot of info was derived from the [Firebase setup for Android](https://firebase.google.com/docs/cpp/setup#setup_for_android) page


### C++ libraries

The libraries were copied from:

	firebase_cpp_sdk/libs/android/armeabi-v7a/c++/*.a

#### The .jar files

The jar files were gotten from the build folder of the [Google AdMob test app](https://github.com/firebase/quickstart-cpp/tree/master/admob/testapp) and then renamed accordingly (and a few were removed, since they seemed irrelevant to this example):

    testapp/build/intermediates/exploded-aar/**/*.jar


#### The resource files

The resource files were taken from the build folder of the [Google AdMob test app](https://github.com/firebase/quickstart-cpp/tree/master/admob/testapp):

	testapp/build/generated/res/google-services/debug/**/*.*

A minor change was required to ```admob/res/values/values.xml```, and that was removing the adSize and adUnitId attributes (due to a build error)


### iOS

The main info about the setup come from the [Firebase setup for iOS](https://firebase.google.com/docs/cpp/setup#setup_for_ios) page

Another resource was [googleads-mobile-ios-examples](https://github.com/googleads/googleads-mobile-ios-examples)


### C++ libraries

The frameworks were copied from:

	firebase_cpp_sdk/frameworks/ios/universal

and the unwanted architectures were removed (```lipo -extract i386``` etc), and only armv7 and arm64 were kept.

