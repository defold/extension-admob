package com.defold.admob;

import androidx.annotation.NonNull;
import android.util.Log;
import android.app.Activity;

import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.FullScreenContentCallback;

import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;

import com.google.android.gms.ads.OnUserEarnedRewardListener;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;

public class AdmobJNI {

  private static final String TAG = "AdmobJNI";

  public static native void admobAddToQueue(int msg, String json);

  private Activity activity;

  public AdmobJNI(Activity activity) {
      this.activity = activity;
  }

  public void initialize() {
      MobileAds.initialize(activity, new OnInitializationCompleteListener() {
          @Override
          public void onInitializationComplete(InitializationStatus initializationStatus) {
              admobAddToQueue(1, "{\"test\":\"Test\"}");
          }
      });
  }

//--------------------------------------------------
// Interstitial ADS

  private InterstitialAd mInterstitialAd;

  public void loadInterstitial(final String placementId) {
      activity.runOnUiThread(new Runnable() {
      @Override
      public void run() {
          AdRequest adRequest = new AdRequest.Builder().build();

          InterstitialAd.load(activity, placementId, adRequest, new InterstitialAdLoadCallback() {
                  @Override
                  public void onAdLoaded(@NonNull InterstitialAd interstitialAd) {
                      // The mInterstitialAd reference will be null until
                      // an ad is loaded.
                     mInterstitialAd = interstitialAd;
                     Log.d(TAG, "onAdLoaded");
                     mInterstitialAd.setFullScreenContentCallback(new FullScreenContentCallback(){
                        @Override
                        public void onAdDismissedFullScreenContent() {
                          // Called when fullscreen content is dismissed.
                          Log.d(TAG, "The ad was dismissed.");
                          mInterstitialAd = null;
                        }

                        @Override
                        public void onAdFailedToShowFullScreenContent(AdError adError) {
                          // Called when fullscreen content failed to show.
                          Log.d(TAG, "The ad failed to show.");
                          mInterstitialAd = null;
                        }

                        @Override
                        public void onAdShowedFullScreenContent() {
                          // Called when fullscreen content is shown.
                          // Make sure to set your reference to null so you don't
                          // show it a second time.
                          mInterstitialAd = null;
                          Log.d(TAG, "The ad was shown.");
                        }
                      });
                  }

                  @Override
                  public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
                      // Handle the error
                     Log.d(TAG, loadAdError.getMessage());
                     mInterstitialAd = null;
                  }
              });
          }
      });
  }

  public void showInterstitial() {
      activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
              if (mInterstitialAd != null) {
                mInterstitialAd.show(activity);
              } else {
                Log.d(TAG, "The interstitial ad wasn't ready yet.");
              }
          }
      });
  }

//--------------------------------------------------
// Rewarded ADS

    private RewardedAd mRewardedAd;

    public void loadRewarded(final String placementId) {
      activity.runOnUiThread(new Runnable() {
        @Override
        public void run() {
          AdRequest adRequest = new AdRequest.Builder().build();

          RewardedAd.load(activity, placementId,
            adRequest, new RewardedAdLoadCallback(){
              @Override
              public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
                // Handle the error.
                Log.d(TAG, loadAdError.getMessage());
                mRewardedAd = null;
              }
              @Override
              public void onAdLoaded(@NonNull RewardedAd rewardedAd) {
                mRewardedAd = rewardedAd;
                Log.d(TAG, "onAdFailedToLoad");
                mRewardedAd.setFullScreenContentCallback(new FullScreenContentCallback() {
                  @Override
                  public void onAdShowedFullScreenContent() {
                    // Called when ad is shown.
                    Log.d(TAG, "Ad was shown.");
                    mRewardedAd = null;
                  }

                  @Override
                  public void onAdFailedToShowFullScreenContent(AdError adError) {
                    // Called when ad fails to show.
                    Log.d(TAG, "Ad failed to show.");
                    mRewardedAd = null;
                  }

                  @Override
                  public void onAdDismissedFullScreenContent() {
                    // Called when ad is dismissed.
                    // Don't forget to set the ad reference to null so you
                    // don't show the ad a second time.
                    Log.d(TAG, "Ad was dismissed.");
                    mRewardedAd = null;
                  }
                });
              }
          });
        }
      });
    }

      public void showRewarded() {
      activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            if (mRewardedAd != null) {
              mRewardedAd.show(activity, new OnUserEarnedRewardListener() {
                @Override
                public void onUserEarnedReward(@NonNull RewardItem rewardItem) {
                  // Handle the reward.
                   Log.d(TAG, "The user earned the reward.");
                   int rewardAmount = rewardItem.getAmount();
                   String rewardType = rewardItem.getType();
                }
              });
            } else {
              Log.d(TAG, "The rewarded ad wasn't ready yet.");
            }
          }
      });
  }

}