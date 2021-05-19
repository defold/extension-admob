# Notes on setup of this extension

The extension uses [iOS](https://developers.google.com/admob/ios/quick-start) and [Android](https://developers.google.com/admob/android/quick-start) SDKs.

## Android SDK update

Open `extension-admob/manifests/android/build.gradle` and change version in `compile 'com.google.android.gms:play-services-ads:X.X.X'` to the latest.

Check [Release Notes](https://developers.google.com/admob/android/rel-notes) to make sure there are no breaking changes and all new APIs implemented.

## iOS SDK update

Download the latest version of the SDK: https://developers.google.com/admob/ios/download
Copy frameworks from `SDK_folder/famework_name.xcframework/ios-arm64_armv7/famework_name.framework` to `extension-admob/lib/ios/` for every framework.

For example:
`GoogleMobileAdsSdkiOS-8.5.0/GoogleUtilities.xcframework/ios-arm64_armv7/GoogleUtilities.framework` to `extension-admob/lib/ios/GoogleUtilities.framework`.

![2021-05-16_12-11-08](https://user-images.githubusercontent.com/2209596/118393521-e79db780-b63f-11eb-9da7-eb3e7600f5dd.png)

Check [Release Notes](https://developers.google.com/admob/ios/rel-notes) to make sure there are no breaking changes and all new APIs implemented.
