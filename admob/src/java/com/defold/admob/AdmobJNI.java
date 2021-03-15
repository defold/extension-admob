package com.defold.admob;

import android.app.Activity;

import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;

public class AdmobJNI {

    public static native void admobAddToQueue(int msg, String json);

    private Activity activity;

    public AdmobJNI(Activity activity) {
        this.activity = activity;
    }

    public void initialize() {
        MobileAds.initialize(activity, new OnInitializationCompleteListener() {
            @Override
            public void onInitializationComplete(InitializationStatus initializationStatus) {
                admobAddToQueue(1, "{\"error\":\"Error while converting simple message to JSON\"}");
            }
        });
    }
}