
# DISCLAIMER: This is an example

Although we want to make this example usable for the users, we cannot guarantee that
we have the time to polish this example or even add bugfixes.

You should see this example as just that, an example.

# Known issues

* Currently, the deletion of banners and native express ads is fairly unusable on iOS
	- They crash when unloaded
	- To avoid the crash, they currently "leak". Use at own discretion!

# Setup

## How do I use this extension?

Add the package link (https://github.com/defold/extension-admob/archive/v1.0.zip or the version you want) to the project setting project.dependencies, and you should be good to go.

See the [manual](http://www.defold.com/manuals/libraries/) for further info.

## Firebase

Log in to the [Firebase Console](https://console.firebase.google.com/) and setup your app accordingly
Download your GoogleService-Info.plist and google-services.json and put them in your project.

	project/
		res/
			android/
				google-services.json
			ios/
				GoogleService-Info.plist


## Android

### AndroidManifest.xml

Change your manifest file accordingly. See the example file ```admob/AndroidManifest.xml```, and look for "Firebase"



## game.project API

First, you need to set these options in your game.project

### The AdMob app id's

You add your app id's for each platform like so:

	[admob]
	app_id_ios = ca-app-pub-1231231231231231~1111111111
	app_id_android = ca-app-pub-1231231231231231~2222222222


### Android manifest

	[android]
	manifest = /admob/AndroidManifest.xml

### Bundle Resources

To make sure your GoogleService-Info.plist and google-services.json are added to the bundle:

	[project]
	bundle_resources = /res

### App manifest

We've also introduced a new file called an "app manifest". It allows you to exclude jar files, lib files, symbols etc.
It can be named something appropriate, and later linked to using the tag.

	[native_extension]
	app_manifest = game.appmanifest


# Lua API

## Functions

	admob.load_banner(adunit, {info}, callback)
	admob.show_banner()
	admob.hide_banner()
	admob.move_banner()
	admob.unload_banner()

	admob.load_nativeexpress(adunit, {info}, callback)
	admob.show_nativeexpress()
	admob.hide_nativeexpress()
	admob.move_nativeexpress()
	admob.unload_nativeexpress()

	admob.load_interstitial(adunit, {info}, callback)
	admob.show_interstitial()
	admob.unload_interstitial()

	admob.load_rewardedvideo(adunit, {info}, callback)
	admob.show_rewardedvideo()
	admob.unload_rewardedvideo()

The info table can have these options:

	
    width: 			Width of the ad (for banner types)
    height:			Height of the ad (for banner types)
    birthday_year
    birthday_month
    birthday_day:	The birthday of the app user
    gender:			The gender of the app user

    tagged_for_child_directed_treatment: The ad policy

    keywords:		A list of keywords
    extras:			A list of key/value pairs. Each entry is in itself a table: extras = { {"key" = "value"}, {"key2" = "value2"} }
    testdevices:		A list of device sha1's to allow to test the ads


Example:

    admob.load_banner(self.banner_ad_unit, { width = 320, height = 50, birthday_day = 13, testdevices = self.testdevices, keywords = self.keywords }, callback )

## Constants

	admob.TYPE_BANNER
	admob.TYPE_INTERSTITIAL
	admob.TYPE_NATIVEEXPRESS
	admob.TYPE_REWARDEDVIDEO

	admob.POSITION_BOTTOM
	admob.POSITION_BOTTOMLEFT
	admob.POSITION_BOTTOMRIGHT
	admob.POSITION_TOP
	admob.POSITION_TOPLEFT
	admob.POSITION_TOPRIGHT

	admob.ERROR_NONE
	admob.ERROR_ALREADYINITIALIZED
	admob.ERROR_INTERNALERROR
	admob.ERROR_INVALIDREQUEST
	admob.ERROR_LOADINPROGRESS
	admob.ERROR_NETWORKERROR
	admob.ERROR_NOFILL
	admob.ERROR_NOWINDOWTOKEN
	admob.ERROR_UNINITIALIZED

	admob.MESSAGE_APP_LEAVE
	admob.MESSAGE_FAILED_TO_LOAD
	admob.MESSAGE_HIDE
	admob.MESSAGE_LOADED
	admob.MESSAGE_REWARD
	admob.MESSAGE_SHOW
	admob.MESSAGE_UNLOADED

	admob.CHILDDIRECTED_TREATMENT_STATE_NOT_TAGGED
	admob.CHILDDIRECTED_TREATMENT_STATE_TAGGED
	admob.CHILDDIRECTED_TREATMENT_STATE_UNKNOWN

	admob.GENDER_FEMALE
	admob.GENDER_MALE
	admob.GENDER_UNKNOWN


# How the AdMob example was setup

In case you'd like to do this from scratch yourself, here are some notes about the setup of this library

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

