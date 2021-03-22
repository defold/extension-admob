package com.defold.admob;

import androidx.annotation.NonNull;
import android.util.Log;
import android.util.DisplayMetrics;
import android.app.Activity;
import android.view.Display;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;

import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;

import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;

import com.google.android.gms.ads.OnUserEarnedRewardListener;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;

public class AdmobJNI {

  private static final String TAG = "AdmobJNI";

  public static native void admobAddToQueue(int msg, String json);

  // duplicate of enums from private_admob_callback.h:
  private static final int MSG_INTERSTITIAL =       1;
  private static final int MSG_REWARDED =           2;
  private static final int MSG_BANNER =             3;
  private static final int MSG_INITIALIZATION =     4;

  private static final int EVENT_CLOSED =             1;
  private static final int EVENT_FAILED_TO_SHOW =     2;
  private static final int EVENT_OPENING =            3;
  private static final int EVENT_FAILED_TO_LOAD =     4;
  private static final int EVENT_LOADED =             5;
  private static final int EVENT_NOT_LOADED =         6;
  private static final int EVENT_EARNED_REWARD =      7;
  private static final int EVENT_COMPLETE =           8;
  private static final int EVENT_CLICKED =            9;


  private Activity activity;

  public AdmobJNI(Activity activity) {
      this.activity = activity;
  }

  public void initialize() {
      MobileAds.initialize(activity, new OnInitializationCompleteListener() {
          @Override
          public void onInitializationComplete(InitializationStatus initializationStatus) {
            sendSimpleMessage(MSG_INITIALIZATION, EVENT_COMPLETE);
          }
      });
  }

  private String getJsonConversionErrorMessage(String messageText) {
    String message = null;
      try {
          JSONObject obj = new JSONObject();
          obj.put("error", messageText);
          message = obj.toString();
      } catch (JSONException e) {
          message = "{ \"error\": \"Error while converting simple message to JSON.\" }";
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
          message = getJsonConversionErrorMessage(e.getMessage());
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
          message = getJsonConversionErrorMessage(e.getMessage());
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
          message = getJsonConversionErrorMessage(e.getMessage());
      }
      admobAddToQueue(msg, message);
    }

//--------------------------------------------------
// Interstitial ADS

  private InterstitialAd mInterstitialAd;

  public void loadInterstitial(final String unitId) {
      activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
          AdRequest adRequest = new AdRequest.Builder().build();

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

  public void loadRewarded(final String unitId) {
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AdRequest adRequest = new AdRequest.Builder().build();

        RewardedAd.load(activity, unitId,
          adRequest, new RewardedAdLoadCallback(){
            @Override
            public void onAdLoaded(@NonNull RewardedAd rewardedAd) {
              // Log.d(TAG, "onAdLoaded");
              mRewardedAd = rewardedAd;
              sendSimpleMessage(MSG_REWARDED, EVENT_LOADED);
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
// Banner ADS

  private LinearLayout layout;
  private WindowManager.LayoutParams windowParams;
  private AdView mBannerAdView;
  private boolean isShown = false;

  public void loadBanner(final String unitId) {
    mBannerAdView = new AdView(activity);
    mBannerAdView.setAdUnitId(unitId);
    AdSize adSize = getAdaptiveSize();
    mBannerAdView.setAdSize(adSize); //TODO: Use different sizes
    activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
          AdRequest adRequest = new AdRequest.Builder().build();
          mBannerAdView.setAdListener(new AdListener() {
            @Override
            public void onAdLoaded() {
              // Code to be executed when an ad finishes loading.
              createLayout();
              sendSimpleMessage(MSG_BANNER, EVENT_LOADED);
            }

            @Override
            public void onAdFailedToLoad(LoadAdError loadAdError) {
              // Code to be executed when an ad request fails.
              sendSimpleMessage(MSG_BANNER, EVENT_FAILED_TO_LOAD, "code", loadAdError.getCode(),
                          "error", String.format("Error domain: \"%s\". %s", loadAdError.getDomain(), loadAdError.getMessage()));
            }

            @Override
            public void onAdOpened() {
              // Code to be executed when an ad opens an overlay that
              // covers the screen.
              sendSimpleMessage(MSG_BANNER, EVENT_OPENING);
            }

            @Override
            public void onAdClicked() {
              // Code to be executed when the user clicks on an ad.
              sendSimpleMessage(MSG_BANNER, EVENT_CLICKED);
            }

            @Override
            public void onAdClosed() {
              // Code to be executed when the user is about to return
              // to the app after tapping on an ad.
              sendSimpleMessage(MSG_BANNER, EVENT_CLOSED);
            }
          });
          mBannerAdView.loadAd(adRequest);
        }
    });
  }

  public void showBanner() {
    if (isShown || mBannerAdView == null) {
        return;
    }
    isShown = true;
    activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
            WindowManager wm = activity.getWindowManager();
            // windowParams.gravity = m_bannerPosition.getGravity();
            wm.addView(layout, windowParams);
        }
    });
  }

  public void hideBanner() {
  }

  private AdSize getAdaptiveSize() {
    Display display = activity.getWindowManager().getDefaultDisplay();
    DisplayMetrics outMetrics = new DisplayMetrics();
    display.getMetrics(outMetrics);

    float widthPixels = outMetrics.widthPixels;
    float density = outMetrics.density;

    int adWidth = (int) (widthPixels / density);
    
    return AdSize.getCurrentOrientationAnchoredAdaptiveBannerAdSize(activity, adWidth);
  }

  private void createLayout() {
    layout = new LinearLayout(activity);
    layout.setOrientation(LinearLayout.VERTICAL);

    MarginLayoutParams params = new MarginLayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
    params.setMargins(0, 0, 0, 0);

    layout.addView(mBannerAdView, params);

    windowParams = new WindowManager.LayoutParams();
    windowParams.x = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.y = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
    windowParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
  }

}