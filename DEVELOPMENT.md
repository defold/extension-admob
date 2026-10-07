# Notes on setup of this extension

The extension uses the [iOS Mobile Ads SDK](https://developers.google.com/admob/ios/quick-start) and the [GMA Next-Gen SDK for Android](https://developers.google.com/admob/android/next-gen/quick-start).

## Android SDK update

Run the Android updater from the repository root:

```sh
python3 updater/android.py
```

The updater reads the latest GMA Next-Gen SDK release and mediation adapter definitions, then regenerates `extension-admob/manifests/android/build.gradle`. Do not edit that generated file by hand.

Check the [GMA Next-Gen SDK release notes](https://developers.google.com/admob/android/next-gen/rel-notes) for breaking changes that require extension API updates.

## Android R8 verification

Enable `/builtins/manifests/android/dmengine.keep` in `android.r8_keep_rules`, then bundle the example with **Generate debug symbols** enabled (or `--with-symbols` in Bob). Check that the bundle's `mapping.txt` identifies R8 as the compiler.

Install the APK on an Android device or emulator. Using the example's Google test ad units, initialize AdMob, load and show a rewarded ad, finish it, and close it. Verify `EVENT_LOADED`, `EVENT_OPENING`, `EVENT_IMPRESSION_RECORDED`, `EVENT_EARNED_REWARD`, and `EVENT_CLOSED`, then load another ad.

The extension's `admob.keep` preserves both the JNI bridge and methods annotated with `android.webkit.JavascriptInterface`. The latter are called by JavaScript in the SDK's WebViews, so R8 cannot discover those calls from Java code. Keeping annotation attributes alone does not preserve the methods. This keep rule is also included in [Android Gradle's default rules](https://android.googlesource.com/platform/tools/base/+/refs/heads/studio-master-dev/build-system/gradle-core/src/main/resources/com/android/build/gradle/proguard-common.txt). When updating the SDK, also verify that these methods survive shrinking; a successful test ad can use a different communication path and hide a broken JavaScript bridge.

## iOS SDK update

Open `extension-admob/manifests/ios/Podfile` and change version in `pod 'Google-Mobile-Ads-SDK', 'X.X.X'` to the latest.

Check [Release Notes](https://developers.google.com/admob/ios/rel-notes) to make sure there are no breaking changes and all new APIs implemented.
