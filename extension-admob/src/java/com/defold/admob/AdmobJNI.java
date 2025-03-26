package com.defold.admob;

import androidx.annotation.NonNull;
import android.util.Log;
import android.util.DisplayMetrics;
import android.app.Activity;
import android.view.Display;
import android.view.Gravity;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.content.Context;
import android.content.SharedPreferences;

import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.ProcessLifecycleOwner;
import androidx.lifecycle.OnLifecycleEvent;
import androidx.lifecycle.Lifecycle.Event;

import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.appopen.AppOpenAd;
import com.google.android.gms.ads.appopen.AppOpenAd.AppOpenAdLoadCallback;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.AdInspectorError;
import com.google.android.gms.ads.OnAdInspectorClosedListener;
import com.google.android.gms.ads.RequestConfiguration;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;

import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;

import com.google.android.gms.ads.OnUserEarnedRewardListener;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.google.android.gms.ads.rewarded.ServerSideVerificationOptions;

import com.google.android.gms.ads.rewardedinterstitial.RewardedInterstitialAd;
import com.google.android.gms.ads.rewardedinterstitial.RewardedInterstitialAdLoadCallback;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;

public class AdmobJNI implements LifecycleObserver {

  private static final String TAG = "AdmobJNI";

  public static native void admobAddToQueue(int msg, String json);

  // duplicate of enums from admob_callback_private.h:
  // CONSTANTS:
  private static final int MSG_INTERSTITIAL =         1;
  private static final int MSG_REWARDED =             2;
  private static final int MSG_BANNER =               3;
  private static final int MSG_INITIALIZATION =       4;
  private static final int MSG_IDFA =                 5;
  private static final int MSG_REWARDED_INTERSTITIAL =6;
  private static final int MSG_APPOPEN =              7;

  private static final int EVENT_CLOSED =             1;
  private static final int EVENT_FAILED_TO_SHOW =     2;
  private static final int EVENT_OPENING =            3;
  private static final int EVENT_FAILED_TO_LOAD =     4;
  private static final int EVENT_LOADED =             5;
  private static final int EVENT_NOT_LOADED =         6;
  private static final int EVENT_EARNED_REWARD =      7;
  private static final int EVENT_COMPLETE =           8;
  private static final int EVENT_CLICKED =            9;
  private static final int EVENT_DESTROYED =          10;
  private static final int EVENT_JSON_ERROR =         11;
  private static final int EVENT_IMPRESSION_RECORDED =12;
  // 13-16 are for iOS only
  private static final int EVENT_NOT_SUPPORTED =      17;

  private static final int SIZE_ADAPTIVE_BANNER =     0;
  private static final int SIZE_BANNER =              1;
  private static final int SIZE_FLUID =               2;
  private static final int SIZE_FULL_BANNER =         3;
  private static final int SIZE_LARGE_BANNER =        4;
  private static final int SIZE_LEADEARBOARD =        5;
  private static final int SIZE_MEDIUM_RECTANGLE =    6;
  private static final int SIZE_SEARH =               7;
  private static final int SIZE_SKYSCRAPER =          8;
  private static final int SIZE_SMART_BANNER =        9;

  private static final int POS_NONE =                 0;
  private static final int POS_TOP_LEFT =             1;
  private static final int POS_TOP_CENTER =           2;
  private static final int POS_TOP_RIGHT =            3;
  private static final int POS_BOTTOM_LEFT =          4;
  private static final int POS_BOTTOM_CENTER =        5;
  private static final int POS_BOTTOM_RIGHT =         6;
  private static final int POS_CENTER =               7;

  private static final int MAX_AD_CONTENT_RATING_G =  0;
  private static final int MAX_AD_CONTENT_RATING_PG = 1;
  private static final int MAX_AD_CONTENT_RATING_T =  2;
  private static final int MAX_AD_CONTENT_RATING_MA = 3;

  // END CONSTANTS

  private String defoldUserAgent = "defold-x.y.z";

  private Activity activity;


  public AdmobJNI(Activity activity, String appOpenAdUnitId, String defoldUserAgent) {
      this.activity = activity;
      this.mAppOpenAdUnitId = appOpenAdUnitId;
      this.defoldUserAgent = defoldUserAgent;

      if (isAutomaticAppOpenEnabled()) {
        activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            initialize();
            ProcessLifecycleOwner.get().getLifecycle().addObserver(AdmobJNI.this);
            loadAppOpen(appOpenAdUnitId, true);
          }
        });
      }
  }

  public void initialize() {
      MobileAds.initialize(activity, new OnInitializationCompleteListener() {
          @Override
          public void onInitializationComplete(InitializationStatus initializationStatus) {
            sendSimpleMessage(MSG_INITIALIZATION, EVENT_COMPLETE);
          }
      });
  }

  public void setPrivacySettings(boolean enable_rdp) {
    SharedPreferences sharedPref = activity.getPreferences(Context.MODE_PRIVATE);
    SharedPreferences.Editor editor = sharedPref.edit();
    editor.putInt("gad_rdp", 1);
    editor.commit();
  }

  public void setMaxAdContentRating(final int max_ad_rating) {
    String rating = null;
    switch (max_ad_rating) {
      case MAX_AD_CONTENT_RATING_G:
        rating = RequestConfiguration.MAX_AD_CONTENT_RATING_G;
        break;
      case MAX_AD_CONTENT_RATING_PG:
        rating = RequestConfiguration.MAX_AD_CONTENT_RATING_PG;
        break;
      case MAX_AD_CONTENT_RATING_T:
        rating = RequestConfiguration.MAX_AD_CONTENT_RATING_T;
        break;
      case MAX_AD_CONTENT_RATING_MA:
        rating = RequestConfiguration.MAX_AD_CONTENT_RATING_MA;
        break;
      }
    if (rating != null) {
      RequestConfiguration requestConfiguration = MobileAds.getRequestConfiguration()
        .toBuilder()
        .setMaxAdContentRating(rating)
        .build();
      MobileAds.setRequestConfiguration(requestConfiguration);
      Log.d(TAG, "setMaxAdContentRating " + rating);
    }
  }

  public void requestIDFA() {
    sendSimpleMessage(MSG_IDFA, EVENT_NOT_SUPPORTED);
  }

  public void showAdInspector() {
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        MobileAds.openAdInspector(activity, new OnAdInspectorClosedListener() {
          public void onAdInspectorClosed(AdInspectorError error) {
            // Error will be non-null if ad inspector closed due to an error.
            if (error != null) {
              Log.d(TAG, error.toString());
            }
          }
        });
      }
    });
  }

  // https://www.baeldung.com/java-json-escaping
  private String getJsonConversionErrorMessage(String messageText) {
    String message = null;
      try {
          JSONObject obj = new JSONObject();
          obj.put("error", messageText);
          obj.put("event", EVENT_JSON_ERROR);
          message = obj.toString();
      } catch (JSONException e) {
          message = "{ \"error\": \"Error while converting simple message to JSON.\", \"event\": "+EVENT_JSON_ERROR+" }";
      }
    return message;
  }

  private void sendSimpleMessage(int msg, int eventId) {
      String message = null;
      try {
          JSONObject obj = new JSONObject();
          obj.put("event", eventId);
          message = obj.toString();
      } catch (JSONException e) {
          message = getJsonConversionErrorMessage(e.getLocalizedMessage());
      }
      admobAddToQueue(msg, message);
  }

  private void sendSimpleMessage(int msg, int eventId, String key_2, String value_2) {
      String message = null;
      try {
          JSONObject obj = new JSONObject();
          obj.put("event", eventId);
          obj.put(key_2, value_2);
          message = obj.toString();
      } catch (JSONException e) {
          message = getJsonConversionErrorMessage(e.getLocalizedMessage());
      }
      admobAddToQueue(msg, message);
    }

  private void sendSimpleMessage(int msg, int eventId, String key_2, int value_2, String key_3, String value_3) {
      String message = null;
      try {
          JSONObject obj = new JSONObject();
          obj.put("event", eventId);
          obj.put(key_2, value_2);
          obj.put(key_3, value_3);
          message = obj.toString();
      } catch (JSONException e) {
          message = getJsonConversionErrorMessage(e.getLocalizedMessage());
      }
      admobAddToQueue(msg, message);
    }

  private void sendSimpleMessage(int msg, int eventId, String key_2, int value_2, String key_3, int value_3) {
    String message = null;
    try {
        JSONObject obj = new JSONObject();
        obj.put("event", eventId);
        obj.put(key_2, value_2);
        obj.put(key_3, value_3);
        message = obj.toString();
    } catch (JSONException e) {
        message = getJsonConversionErrorMessage(e.getLocalizedMessage());
    }
    admobAddToQueue(msg, message);
  }

  private AdRequest createAdRequest() {
    return new AdRequest.Builder().setRequestAgent(defoldUserAgent).build();
  }

//--------------------------------------------------
// App Open Ads

  private String mAppOpenAdUnitId = null;
  private AppOpenAd mAppOpenAd = null;
  private boolean mIsLoadingAppOpenAd = false;
  private boolean mIsShowingAppOpenAd = false;

  @OnLifecycleEvent(Event.ON_START)
  protected void onMoveToForeground() {
    Log.d(TAG, "onMoveToForeground");
    showAppOpen();
  }

  private boolean isAutomaticAppOpenEnabled() {
    return mAppOpenAdUnitId != null;
  }

  public boolean isAppOpenLoaded() {
    return mAppOpenAd != null;
  }

  public void showAppOpen() {
    // If the app open ad is already showing, do not show the ad again
    if (mIsShowingAppOpenAd) {
      Log.d(TAG, "The app open ad is already showing.");
      return;
    }

    // If the app open ad is not available yet then load it
    if (!isAppOpenLoaded() && isAutomaticAppOpenEnabled()) {
      Log.d(TAG, "The app open ad is not ready yet.");
      loadAppOpen(mAppOpenAdUnitId, true);
      return;
    }

    Log.d(TAG, "Showing app open ad.");
    mIsShowingAppOpenAd = true;
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        mAppOpenAd.setFullScreenContentCallback(
            new FullScreenContentCallback() {

              @Override
              public void onAdDismissedFullScreenContent() {
                // Called when fullscreen content is dismissed.
                // Clean up state and load a new ad
                Log.d(TAG, "Ad dismissed fullscreen content.");
                sendSimpleMessage(MSG_APPOPEN, EVENT_CLOSED);
                mAppOpenAd = null;
                mIsShowingAppOpenAd = false;
                if (isAutomaticAppOpenEnabled()) {
                  loadAppOpen(mAppOpenAdUnitId, false);
                }
              }

              @Override
              public void onAdFailedToShowFullScreenContent(AdError adError) {
                // Called when fullscreen content failed to show.
                // Clean up state and load a new ad
                Log.d(TAG, adError.getMessage());
                sendSimpleMessage(MSG_APPOPEN, EVENT_FAILED_TO_SHOW, "code", adError.getCode(),
                                "error", String.format("Error domain: \"%s\". %s", adError.getDomain(), adError.getMessage()));
                mAppOpenAd = null;
                mIsShowingAppOpenAd = false;
                if (isAutomaticAppOpenEnabled()) {
                  loadAppOpen(mAppOpenAdUnitId, false);
                }
              }

              @Override
              public void onAdShowedFullScreenContent() {
                // Called when fullscreen content is shown.
                Log.d(TAG, "Ad showed fullscreen content.");
                sendSimpleMessage(MSG_APPOPEN, EVENT_OPENING);
              }

              @Override
              public void onAdImpression() {
                sendSimpleMessage(MSG_APPOPEN, EVENT_IMPRESSION_RECORDED);
              }

              @Override
              public void onAdClicked() {
                sendSimpleMessage(MSG_APPOPEN, EVENT_CLICKED);
              }
            });
        mAppOpenAd.show(activity);
      }
    });
  }

  // Load an app open ad with the provided ad unit id, optionally also showing it
  // immediately when it loaded
  public void loadAppOpen(String adUnitId, boolean showImmediately) {
    // Do not load ad if one is already loading.
    if (mIsLoadingAppOpenAd) {
      Log.d(TAG, "Already loading app open ad.");
      return;
    }

    // Do not load ad if there is an unused ad.
    if (isAppOpenLoaded()) {
      Log.d(TAG, "App open ad was already loaded.");
      sendSimpleMessage(MSG_APPOPEN, EVENT_LOADED);
      if (showImmediately) {
        showAppOpen();
      }
      return;
    }

    Log.d(TAG, "Loading app open ad.");
    mIsLoadingAppOpenAd = true;
    AdRequest request = new AdRequest.Builder().build();
    AppOpenAd.load(
      activity, adUnitId, request,
      AppOpenAd.APP_OPEN_AD_ORIENTATION_PORTRAIT,
      new AppOpenAdLoadCallback() {
        @Override
        public void onAdLoaded(AppOpenAd ad) {
          // Called when an app open ad has loaded.
          Log.d(TAG, "Ad was loaded.");
          sendSimpleMessage(MSG_APPOPEN, EVENT_LOADED);
          mAppOpenAd = ad;
          mIsLoadingAppOpenAd = false;
          if (showImmediately) {
            showAppOpen();
          }
        }

        @Override
        public void onAdFailedToLoad(LoadAdError loadAdError) {
          // Called when an app open ad has failed to load.
          Log.d(TAG, loadAdError.getMessage());
          sendSimpleMessage(MSG_APPOPEN, EVENT_FAILED_TO_SHOW, "code", loadAdError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", loadAdError.getDomain(), loadAdError.getMessage()));
          mIsLoadingAppOpenAd = false;
        }
      });
  }

  // Load an app open ad with the provided ad unit id but don't show it
  public void loadAppOpen(String adUnitId) {
    loadAppOpen(adUnitId, false);
  }

//--------------------------------------------------
// Interstitial ADS

  private InterstitialAd mInterstitialAd;

  public void loadInterstitial(final String unitId) {
      activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AdRequest adRequest = createAdRequest();

        InterstitialAd.load(activity, unitId, adRequest, new InterstitialAdLoadCallback() {
                @Override
                public void onAdLoaded(@NonNull InterstitialAd interstitialAd) {
                  // The mInterstitialAd reference will be null until
                  // an ad is loaded.
                  // Log.d(TAG, "onAdLoaded");
                   mInterstitialAd = interstitialAd;
                   sendSimpleMessage(MSG_INTERSTITIAL, EVENT_LOADED);
                   mInterstitialAd.setFullScreenContentCallback(new FullScreenContentCallback(){
                      @Override
                      public void onAdDismissedFullScreenContent() {
                        // Called when fullscreen content is dismissed.
                        // Log.d(TAG, "The ad was closed.");
                        mInterstitialAd = null;
                        sendSimpleMessage(MSG_INTERSTITIAL, EVENT_CLOSED);
                      }

                      @Override
                      public void onAdFailedToShowFullScreenContent(AdError adError) {
                        // Called when fullscreen content failed to show.
                        mInterstitialAd = null;
                        sendSimpleMessage(MSG_INTERSTITIAL, EVENT_FAILED_TO_SHOW, "code", adError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", adError.getDomain(), adError.getMessage()));
                      }

                      @Override
                      public void onAdShowedFullScreenContent() {
                        // Called when fullscreen content is shown.
                        // Make sure to set your reference to null so you don't
                        // show it a second time.
                        // Log.d(TAG, "The ad was shown.");
                        mInterstitialAd = null;
                        sendSimpleMessage(MSG_INTERSTITIAL, EVENT_OPENING);
                      }

                      @Override
                      public void onAdImpression() {
                        sendSimpleMessage(MSG_INTERSTITIAL, EVENT_IMPRESSION_RECORDED);
                      }

                      @Override
                      public void onAdClicked() {
                        sendSimpleMessage(MSG_INTERSTITIAL, EVENT_CLICKED);
                      }
                    });
                }

                @Override
                public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
                  // Handle the error
                  // Log.d(TAG, loadAdError.getMessage());
                   mInterstitialAd = null;
                   sendSimpleMessage(MSG_INTERSTITIAL, EVENT_FAILED_TO_LOAD, "code", loadAdError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", loadAdError.getDomain(), loadAdError.getMessage()));
                }
            });
          }
      });
  }

  public void showInterstitial() {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
            if (isInterstitialLoaded()) {
              mInterstitialAd.show(activity);
            } else {
              // Log.d(TAG, "The interstitial ad wasn't ready yet.");
              sendSimpleMessage(MSG_INTERSTITIAL, EVENT_NOT_LOADED, "error", "Can't show Interstitial AD that wasn't loaded.");
            }
        }
    });
  }

  public boolean isInterstitialLoaded() {
    return mInterstitialAd != null;
  }

//--------------------------------------------------
// Rewarded ADS

  private RewardedAd mRewardedAd;

  private void setRewardedCustomData(final String userId, final String customData) {
    if( userId != null)  
    {
      ServerSideVerificationOptions options = new ServerSideVerificationOptions.Builder()
        .setUserId(userId)
        .setCustomData(customData != null ? customData : "")
        .build();
      mRewardedAd.setServerSideVerificationOptions(options);
    }
  }

  public void loadRewarded(final String unitId, final String userId,  final String customData) {
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AdRequest adRequest = createAdRequest();

        RewardedAd.load(activity, unitId,
          adRequest, new RewardedAdLoadCallback(){
            @Override
            public void onAdLoaded(@NonNull RewardedAd rewardedAd) {
              // Log.d(TAG, "onAdLoaded");
              mRewardedAd = rewardedAd;
              sendSimpleMessage(MSG_REWARDED, EVENT_LOADED);
              setRewardedCustomData(userId, customData);
              mRewardedAd.setFullScreenContentCallback(new FullScreenContentCallback() {
                @Override
                public void onAdDismissedFullScreenContent() {
                  // Called when ad is dismissed.
                  // Don't forget to set the ad reference to null so you
                  // don't show the ad a second time.
                  // Log.d(TAG, "Ad was dismissed.");
                  mRewardedAd = null;
                  sendSimpleMessage(MSG_REWARDED, EVENT_CLOSED);
                }

                @Override
                public void onAdFailedToShowFullScreenContent(AdError adError) {
                  // Called when ad fails to show.
                  // Log.d(TAG, "Ad failed to show.");
                  mRewardedAd = null;
                  sendSimpleMessage(MSG_REWARDED, EVENT_FAILED_TO_SHOW, "code", adError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", adError.getDomain(), adError.getMessage()));
                }

                @Override
                public void onAdShowedFullScreenContent() {
                  // Called when ad is shown.
                  // Log.d(TAG, "Ad was shown.");
                  mRewardedAd = null;
                  sendSimpleMessage(MSG_REWARDED, EVENT_OPENING);
                }

                @Override
                public void onAdImpression() {
                  sendSimpleMessage(MSG_REWARDED, EVENT_IMPRESSION_RECORDED);
                }

                @Override
                public void onAdClicked() {
                  sendSimpleMessage(MSG_REWARDED, EVENT_CLICKED);
                }

              });
            }

            @Override
            public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
              // Handle the error.
              // Log.d(TAG, "onAdFailedToLoad");
              mRewardedAd = null;
              sendSimpleMessage(MSG_REWARDED, EVENT_FAILED_TO_LOAD, "code", loadAdError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", loadAdError.getDomain(), loadAdError.getMessage()));
            }
        });
      }
    });
  }

  public void showRewarded() {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (isRewardedLoaded()) {
            mRewardedAd.show(activity, new OnUserEarnedRewardListener() {
              @Override
              public void onUserEarnedReward(@NonNull RewardItem rewardItem) {
                // Handle the reward.
                // Log.d(TAG, "The user earned the reward.");
                int rewardAmount = rewardItem.getAmount();
                String rewardType = rewardItem.getType();
                sendSimpleMessage(MSG_REWARDED, EVENT_EARNED_REWARD, "amount", rewardAmount, "type", rewardType);
              }
            });
          } else {
            // Log.d(TAG, "The rewarded ad wasn't ready yet.");
            sendSimpleMessage(MSG_REWARDED, EVENT_NOT_LOADED, "error", "Can't show Rewarded AD that wasn't loaded.");
          }
        }
    });
  }

  public boolean isRewardedLoaded() {
    return mRewardedAd != null;
  }

//--------------------------------------------------
// Rewarded Interstitial ADS

  private RewardedInterstitialAd mRewardedInterstitialAd;

  public void loadRewardedInterstitial(final String unitId) {
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AdRequest adRequest = createAdRequest();

        RewardedInterstitialAd.load(activity, unitId,
          adRequest, new RewardedInterstitialAdLoadCallback(){
            @Override
            public void onAdLoaded(@NonNull RewardedInterstitialAd rewardedAd) {
              // Log.d(TAG, "onAdLoaded");
              mRewardedInterstitialAd = rewardedAd;
              sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_LOADED);
              mRewardedInterstitialAd.setFullScreenContentCallback(new FullScreenContentCallback() {
                @Override
                public void onAdDismissedFullScreenContent() {
                  // Called when ad is dismissed.
                  // Don't forget to set the ad reference to null so you
                  // don't show the ad a second time.
                  // Log.d(TAG, "Ad was dismissed.");
                  mRewardedInterstitialAd = null;
                  sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_CLOSED);
                }

                @Override
                public void onAdFailedToShowFullScreenContent(AdError adError) {
                  // Called when ad fails to show.
                  // Log.d(TAG, "Ad failed to show.");
                  mRewardedInterstitialAd = null;
                  sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_FAILED_TO_SHOW, "code", adError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", adError.getDomain(), adError.getMessage()));
                }

                @Override
                public void onAdShowedFullScreenContent() {
                  // Called when ad is shown.
                  // Log.d(TAG, "Ad was shown.");
                  mRewardedInterstitialAd = null;
                  sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_OPENING);
                }

                @Override
                public void onAdImpression() {
                  sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_IMPRESSION_RECORDED);
                }

                @Override
                public void onAdClicked() {
                  sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_CLICKED);
                }

              });
            }

            @Override
            public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
              // Handle the error.
              // Log.d(TAG, "onAdFailedToLoad");
              mRewardedInterstitialAd = null;
              sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_FAILED_TO_LOAD, "code", loadAdError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", loadAdError.getDomain(), loadAdError.getMessage()));
            }
        });
      }
    });
  }

  public void showRewardedInterstitial() {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (isRewardedInterstitialLoaded()) {
            mRewardedInterstitialAd.show(activity, new OnUserEarnedRewardListener() {
              @Override
              public void onUserEarnedReward(@NonNull RewardItem rewardItem) {
                // Handle the reward.
                // Log.d(TAG, "The user earned the reward.");
                int rewardAmount = rewardItem.getAmount();
                String rewardType = rewardItem.getType();
                sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_EARNED_REWARD, "amount", rewardAmount, "type", rewardType);
              }
            });
          } else {
            // Log.d(TAG, "The rewarded ad wasn't ready yet.");
            sendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_NOT_LOADED, "error", "Can't show Rewarded Interstitial AD that wasn't loaded.");
          }
        }
    });
  }

  public boolean isRewardedInterstitialLoaded() {
    return mRewardedInterstitialAd != null;
  }

//--------------------------------------------------
// Banner ADS

  private LinearLayout layout;
  private AdView mBannerAdView;
  private WindowManager windowManager;
  private boolean isBannerShown = false;
  private int m_bannerPosition = Gravity.NO_GRAVITY;

  public void loadBanner(final String unitId, int bannerSize) {
    if (isBannerLoaded())
    {
      return;
    }
    final AdView view = new AdView(activity);
    view.setAdUnitId(unitId);
    AdSize adSize = getSizeConstant(bannerSize);
    view.setAdSize(adSize);
    final int bannerWidth  = adSize.getWidthInPixels(activity);
    final int bannerHeight = adSize.getHeightInPixels(activity);
    view.pause();
    // Log.d(TAG, "loadBanner");
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
          AdRequest adRequest = createAdRequest();
          view.setAdListener(new AdListener() {
            @Override
            public void onAdLoaded() {
              // Code to be executed when an ad finishes loading and when banner refreshed.
              // Log.d(TAG, "onAdLoaded");
              if (!isBannerLoaded()) {
                mBannerAdView = view;
                createLayout();
              }
              sendSimpleMessage(MSG_BANNER, EVENT_LOADED, "height", bannerHeight, "width", bannerWidth);
            }

            @Override
            public void onAdFailedToLoad(LoadAdError loadAdError) {
              // Code to be executed when an ad request fails.
              // Log.d(TAG, "onAdFailedToLoad");
              sendSimpleMessage(MSG_BANNER, EVENT_FAILED_TO_LOAD, "code", loadAdError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", loadAdError.getDomain(), loadAdError.getMessage()));
            }

            @Override
            public void onAdOpened() {
              // Code to be executed when an ad opens an overlay that
              // covers the screen.
              // Log.d(TAG, "onAdOpened");
              sendSimpleMessage(MSG_BANNER, EVENT_OPENING);
            }

            @Override
            public void onAdClicked() {
              // Code to be executed when the user clicks on an ad.
              // Log.d(TAG, "onAdClicked");
              sendSimpleMessage(MSG_BANNER, EVENT_CLICKED);
            }

            @Override
            public void onAdClosed() {
              // Code to be executed when the user is about to return
              // to the app after tapping on an ad.
              // Log.d(TAG, "onAdClosed");
              sendSimpleMessage(MSG_BANNER, EVENT_CLOSED);
            }

            @Override
            public void onAdImpression() {
              sendSimpleMessage(MSG_BANNER, EVENT_IMPRESSION_RECORDED);
            }
          });
          view.loadAd(adRequest);
        }
    });
  }

  public void destroyBanner() {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (!isBannerLoaded()) {
            return;
          }
          if (isBannerShown) {
            windowManager.removeView(layout);
          }
          mBannerAdView.destroy();
          layout = null;
          mBannerAdView = null;
          isBannerShown = false;
          sendSimpleMessage(MSG_BANNER, EVENT_DESTROYED);
        }
      });
  }

  public void showBanner(final int pos) {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (!isBannerLoaded()) {
            return;
          }
          layout.setSystemUiVisibility(activity.getWindow().getDecorView().getSystemUiVisibility());
          int gravity = getGravity(pos);
          if ((m_bannerPosition == Gravity.NO_GRAVITY || m_bannerPosition != gravity) && isBannerShown) {
            if (gravity != Gravity.NO_GRAVITY) {
              m_bannerPosition = gravity;
            }
            windowManager.updateViewLayout(layout, getParameters());
            return;
          }
          if (!layout.isShown())
          {

            if (gravity != Gravity.NO_GRAVITY) {
              m_bannerPosition = gravity;
            }
            windowManager.addView(layout, getParameters());
            mBannerAdView.resume();
            isBannerShown = true;
          }
        }
    });
  }

  public void hideBanner() {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (!isBannerLoaded() || !isBannerShown) {
            return;
          }
          isBannerShown = false;
          windowManager.removeView(layout);
          mBannerAdView.pause();
        }
    });
  }

  public boolean isBannerLoaded() {
    return mBannerAdView != null;
  }

  public void updateBannerLayout() {
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (!isBannerLoaded()) {
            return;
          }
          layout.setSystemUiVisibility(activity.getWindow().getDecorView().getSystemUiVisibility());
          if (!isBannerShown) {
            return;
          }
          windowManager.removeView(layout);
          if (isBannerShown)
          {
            windowManager.updateViewLayout(layout, getParameters());
            if (!layout.isShown())
            {
              windowManager.addView(layout, getParameters());
            }
          }
        }
    });
  }

  private int getGravity(int bannerPosConst) {
    int bannerPos = Gravity.NO_GRAVITY; //POS_NONE
    switch (bannerPosConst) {
      case POS_TOP_LEFT:
        bannerPos = Gravity.TOP | Gravity.LEFT;
        break;
      case POS_TOP_CENTER:
        bannerPos = Gravity.TOP | Gravity.CENTER_HORIZONTAL;
        break;
      case POS_TOP_RIGHT:
        bannerPos = Gravity.TOP | Gravity.RIGHT;
        break;
      case POS_BOTTOM_LEFT:
        bannerPos = Gravity.BOTTOM | Gravity.LEFT;
        break;
      case POS_BOTTOM_CENTER:
        bannerPos = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        break;
      case POS_BOTTOM_RIGHT:
        bannerPos = Gravity.BOTTOM | Gravity.RIGHT;
        break;
      case POS_CENTER:
        bannerPos = Gravity.CENTER;
        break;
      }
    return bannerPos;
  }

  private AdSize getSizeConstant(int bannerSizeConst) {
    AdSize bannerSize = getAdaptiveSize(); // SIZE_ADAPTIVE_BANNER
    switch (bannerSizeConst) {
      case SIZE_BANNER:
        bannerSize = AdSize.BANNER;
        break;
      case SIZE_FLUID:
        bannerSize = AdSize.FLUID;
        break;
      case SIZE_FULL_BANNER:
        bannerSize = AdSize.FULL_BANNER;
        break;
      case SIZE_LARGE_BANNER:
        bannerSize = AdSize.LARGE_BANNER;
        break;
      case SIZE_LEADEARBOARD:
        bannerSize = AdSize.LEADERBOARD;
        break;
      case SIZE_MEDIUM_RECTANGLE:
        bannerSize = AdSize.MEDIUM_RECTANGLE;
        break;
      case SIZE_SEARH:
        bannerSize = AdSize.SEARCH;
        break;
      case SIZE_SKYSCRAPER:
        bannerSize = AdSize.WIDE_SKYSCRAPER;
        break;
      case SIZE_SMART_BANNER:
        bannerSize = AdSize.SMART_BANNER;
        break;
      }
    return bannerSize;
  }

  private AdSize getAdaptiveSize() {
    // Determine the screen width (less decorations) to use for the ad width.
    Display display = activity.getWindowManager().getDefaultDisplay();
    DisplayMetrics outMetrics = new DisplayMetrics();
    display.getMetrics(outMetrics);

    float density = outMetrics.density;

    float adWidthPixels = layout != null ? layout.getWidth() : 0;

    // If the ad width isn't known, default to the full screen width.
    if (adWidthPixels == 0) {
      adWidthPixels = outMetrics.widthPixels;
    }

    int adWidth = (int) (adWidthPixels / density);
    return AdSize.getCurrentOrientationAnchoredAdaptiveBannerAdSize(activity, adWidth);
  }

  private void createLayout() {
    windowManager = activity.getWindowManager();
    layout = new LinearLayout(activity);
    layout.setOrientation(LinearLayout.VERTICAL);

    MarginLayoutParams params = new MarginLayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
    params.setMargins(0, 0, 0, 0);
    layout.setSystemUiVisibility(activity.getWindow().getDecorView().getSystemUiVisibility());

    layout.addView(mBannerAdView, params);
  }

  private WindowManager.LayoutParams getParameters() {
    WindowManager.LayoutParams windowParams = new WindowManager.LayoutParams();
    windowParams.x = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.y = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
    windowParams.gravity = m_bannerPosition;
    return windowParams;
  }

}
