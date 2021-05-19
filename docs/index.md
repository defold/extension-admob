---
title: Defold AdMob extension API documentation
brief: This manual covers how to get use AdMob to show ads on iOS and Android in Defold.
---

# Defold AdMob extension API documentation

This extension provides a unified, simple to use interface to show AdMob ads on iOS and Android.


## Installation
To use this library in your Defold project, add the following URL to your `game.project` dependencies:

[https://github.com/defold/extension-admob/archive/master.zip](https://github.com/defold/extension-admob/archive/master.zip)

We recommend using a link to a zip file of a [specific release](https://github.com/defold/extension-admob/releases).


## Configuration
The extension can be configured by adding the following fields to game.project:

```
[admob]
app_id_ios = ca-app-pub-3940256099942544~1458002511
app_id_android = ca-app-pub-3940256099942544~3347511713
ios_tracking_usage_description = Your data will be used to provide you a better and personalized ad experience.
```

### app_id_ios
This is your iOS AdMob app ID. An app ID is a unique ID number assigned to your apps when they're added to AdMob. The app ID is used to identify your apps.

### app_id_android
This is your Android AdMob app ID. An app ID is a unique ID number assigned to your apps when they're added to AdMob. The app ID is used to identify your apps.

### ios_tracking_usage_description

Before requesting the unique IDFA string for a device on iOS 14 the application must request user authorization to access app-related data for tracking the user or the device. This is done automatically when `admob.request_idfa()` is called. The string set in `admob.ios_tracking_usage_description` will be shown to the user.

Apple documentation: https://developer.apple.com/documentation/apptrackingtransparency?language=objc


## Example

[Refer to the example project](https://github.com/defold/extension-admob/blob/master/main/ads.gui_script).


## Source code

The source code is available on [GitHub](https://github.com/defold/extension-admob)


## API reference
