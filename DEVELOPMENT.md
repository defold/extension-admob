# Notes on setup of this extension

The extension uses [iOS](https://developers.google.com/admob/ios/quick-start) and [Android](https://developers.google.com/admob/android/quick-start) SDKs.

## Android SDK update

Open `extension-admob/manifests/android/build.gradle` and change version in `compile 'com.google.android.gms:play-services-ads:X.X.X'` to the latest.

Check [Release Notes](https://developers.google.com/admob/android/rel-notes) to make sure there are no breaking changes and all new APIs implemented.

## iOS SDK update

Open `extension-admob/manifests/ios/Podfile` and change version in `pod 'Google-Mobile-Ads-SDK', 'X.X.X'` to the latest.

Check [Release Notes](https://developers.google.com/admob/ios/rel-notes) to make sure there are no breaking changes and all new APIs implemented.
