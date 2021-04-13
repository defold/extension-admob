#if defined(DM_PLATFORM_IOS)
#include "admob_private.h"
#include "admob_callback_private.h"

#include <GoogleMobileAds/GoogleMobileAds.h>
#include <UIKit/UIKit.h>
#import <AdSupport/AdSupport.h>

#if __has_include(<AppTrackingTransparency/ATTrackingManager.h>)
#import <AppTrackingTransparency/ATTrackingManager.h>
#endif


@interface AdmobExtInterstitialAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@interface AdmobExtRewardedAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@interface AdmobExtBannerAdDelegate : NSObject<GADBannerViewDelegate>
@end

namespace dmAdmob {

    static UIViewController *uiViewController = nil;

    void SendSimpleMessageObjCObject(MessageId msg, id obj) {
        NSError* error;
        NSData* jsonData = [NSJSONSerialization dataWithJSONObject:obj options:(NSJSONWritingOptions)0 error:&error];
        if (jsonData)
        {
            NSString* nsstring = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
            AddToQueueCallback(msg, (const char*)[nsstring UTF8String]);
            [nsstring release];
        }
        else
        {
            NSMutableDictionary *dict = [NSMutableDictionary dictionary];
            [dict setObject:error.localizedDescription forKey:@"error"];
            [dict setObject:[NSNumber numberWithInt:EVENT_JSON_ERROR] forKey:@"event"];
            NSError* error2;
            NSData* errorJsonData = [NSJSONSerialization dataWithJSONObject:dict options:(NSJSONWritingOptions)0 error:&error2];
            if (errorJsonData)
            {
                NSString* nsstringError = [[NSString alloc] initWithData:errorJsonData encoding:NSUTF8StringEncoding];
                AddToQueueCallback(msg, (const char*)[nsstringError UTF8String]);
                [nsstringError release];
            }
            else
            {
                AddToQueueCallback(msg, [[NSString stringWithFormat:@"{ \"error\": \"Error while converting simple message to JSON.\", \"event\": %d }", EVENT_JSON_ERROR] UTF8String]);
            }
        }
    }

    void SendSimpleMessage(MessageId msg, MessageEvent event) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        SendSimpleMessageObjCObject(msg, dict);
    }

    void SendSimpleMessageString(MessageId msg, MessageEvent event, NSString *key_2, NSString *value_2) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        [dict setObject:value_2 forKey:key_2];
        SendSimpleMessageObjCObject(msg, dict);
    }

    void SendSimpleMessageIntString(MessageId msg, MessageEvent event, NSString *key_2, int value_2, NSString *key_3, NSString *value_3) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        [dict setObject:[NSNumber numberWithInt:value_2] forKey:key_2];
        [dict setObject:value_3 forKey:key_3];
        SendSimpleMessageObjCObject(msg, dict);
    }

    void SendSimpleMessageIntInt(MessageId msg, MessageEvent event, NSString *key_2, int value_2, NSString *key_3, int value_3) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        [dict setObject:[NSNumber numberWithInt:value_2] forKey:key_2];
        [dict setObject:[NSNumber numberWithInt:value_3] forKey:key_3];
        SendSimpleMessageObjCObject(msg, dict);
    }

    void Initialize() {
        [[GADMobileAds sharedInstance]
        startWithCompletionHandler:^(GADInitializationStatus *_Nonnull status) {
            SendSimpleMessage(MSG_INITIALIZATION, EVENT_COMPLETE);
        }];
    }

//--------------------------------------------------
// Interstitial ADS

    static GADInterstitialAd *interstitialAd = nil;
    static AdmobExtInterstitialAdDelegate *admobExtInterstitialAdDelegate;

    void SetInterstitialAd(GADInterstitialAd *newAd) {
        if (interstitialAd == newAd) {
            return;
        }
        if (newAd != nil) {
            [newAd retain];
        }
        if (interstitialAd != nil) {
            [interstitialAd release];
        }
        interstitialAd = newAd;
    }

    void LoadInterstitial(const char* unitId) {
        GADRequest *request = [GADRequest request];
        [GADInterstitialAd
            loadWithAdUnitID:[NSString stringWithUTF8String:unitId]
            request:request
            completionHandler:^(GADInterstitialAd *ad, NSError *error) {
                if (error) {
                    SetInterstitialAd(nil);
                    NSLog([NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    SendSimpleMessageIntString(MSG_INTERSTITIAL, EVENT_FAILED_TO_LOAD, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    return;
                }
                SetInterstitialAd(ad);
                SendSimpleMessage(MSG_INTERSTITIAL, EVENT_LOADED);
            }];
    }

    bool IsInterstitialLoaded() {
        return interstitialAd != nil;
    }

    void ShowInterstitial() {
        if (IsInterstitialLoaded()) {
            interstitialAd.fullScreenContentDelegate = admobExtInterstitialAdDelegate;
            NSError* error;
            if ([interstitialAd canPresentFromRootViewController:uiViewController error:&error]) {
                [interstitialAd presentFromRootViewController:uiViewController];
            } else {
                if (error) {
                    SendSimpleMessageIntString(MSG_INTERSTITIAL, EVENT_NOT_LOADED, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                } else {
                    SendSimpleMessageString(MSG_INTERSTITIAL, EVENT_NOT_LOADED, @"error", @"Can't present interstitial AD");
                }
            }
        } else {
            SendSimpleMessageString(MSG_INTERSTITIAL, EVENT_NOT_LOADED, @"error", @"Can't show interstitial AD that wasn't loaded.");
        }
    }

//--------------------------------------------------
// Rewarded ADS

    static GADRewardedAd *rewardedAd = nil;
    static AdmobExtRewardedAdDelegate *admobExtRewardedAdDelegate;

    void SetRewardedAd(GADRewardedAd *newAd) {
        if (rewardedAd == newAd) {
            return;
        }
        if (newAd != nil) {
            [newAd retain];
        }
        if (rewardedAd != nil) {
            [rewardedAd release];
        }
        rewardedAd = newAd;
    }

    void LoadRewarded(const char* unitId) {
        GADRequest *request = [GADRequest request];
        [GADRewardedAd
            loadWithAdUnitID:[NSString stringWithUTF8String:unitId]
            request:request
            completionHandler:^(GADRewardedAd *ad, NSError *error) {
                if (error) {
                    SetRewardedAd(nil);
                    NSLog([NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    SendSimpleMessageIntString(MSG_REWARDED, EVENT_FAILED_TO_LOAD, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    return;
                }
                SetRewardedAd(ad);
                SendSimpleMessage(MSG_REWARDED, EVENT_LOADED);
            }];
    }

    bool IsRewardedLoaded() {
        return rewardedAd != nil;
    }

    void ShowRewarded() {
        if (IsRewardedLoaded()) {
            rewardedAd.fullScreenContentDelegate = admobExtRewardedAdDelegate;
            NSError* error;
            if ([rewardedAd canPresentFromRootViewController:uiViewController error:&error]) {
                [rewardedAd presentFromRootViewController:uiViewController
                    userDidEarnRewardHandler:^{
                            GADAdReward *reward = rewardedAd.adReward;
                            SendSimpleMessageIntString(MSG_REWARDED, EVENT_EARNED_REWARD, @"amount", [reward.amount doubleValue], @"type", [reward type]);
                        }];
            } else {
                if (error) {
                    SendSimpleMessageIntString(MSG_REWARDED, EVENT_NOT_LOADED, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                } else {
                    SendSimpleMessageString(MSG_REWARDED, EVENT_NOT_LOADED, @"error", @"Can't present rewarded AD");
                }
            }
        } else {
            SendSimpleMessageString(MSG_REWARDED, EVENT_NOT_LOADED, @"error", @"Can't show rewarded AD that wasn't loaded.");
        }
    }


//--------------------------------------------------
// Banner ADS

    static GADBannerView *bannerAd = nil;
    static AdmobExtBannerAdDelegate *admobExtBannerAdDelegate;
    static BannerPosition lastBannerPos;

    GADAdSize GetAdaptiveSize() {
        UIView *defoldView = uiViewController.view;
        CGRect frame = defoldView.frame;

        if (@available(iOS 11.0, *)) {
            frame = UIEdgeInsetsInsetRect(defoldView.frame, defoldView.safeAreaInsets);
        }

        CGFloat viewWidth = frame.size.width;
        return GADCurrentOrientationAnchoredAdaptiveBannerAdSizeWithWidth(viewWidth);
    }

    GADAdSize GetSizeConstant(BannerSize bannerSizeConst) {
        GADAdSize bannerSize = GetAdaptiveSize(); // SIZE_ADAPTIVE_BANNER
        //SIZE_FLUID, SIZE_SEARH, SIZE_SKYSCRAPER, SIZE_SMART_BANNER are not available on iOS
        switch (bannerSizeConst) {
          case SIZE_BANNER:
            bannerSize = kGADAdSizeBanner;
            break;
          case SIZE_FULL_BANNER:
            bannerSize = kGADAdSizeFullBanner;
            break;
          case SIZE_LARGE_BANNER:
            bannerSize = kGADAdSizeLargeBanner;
            break;
          case SIZE_LEADEARBOARD:
            bannerSize = kGADAdSizeLeaderboard;
            break;
          case SIZE_MEDIUM_RECTANGLE:
            bannerSize = kGADAdSizeMediumRectangle;
            break;
          }
        return bannerSize;
    }

    void UpdatePosition(BannerPosition bannerSizeConst) {
        UIView *defoldView = uiViewController.view;
        CGRect bounds = defoldView.bounds;
        if (@available(iOS 11.0, *)) {
            CGRect safeAreaFrame = defoldView.safeAreaLayoutGuide.layoutFrame;
            if (!CGSizeEqualToSize(CGSizeZero, safeAreaFrame.size)) {
                bounds = safeAreaFrame;
            }
        }

        CGFloat top = CGRectGetMinY(bounds) + CGRectGetMidY(bannerAd.bounds);
        CGFloat bottom = CGRectGetMaxY(bounds) - CGRectGetMidY(bannerAd.bounds);
        CGFloat centerY = CGRectGetMidY(bounds);

        if (CGRectGetHeight(bannerAd.bounds) >= CGRectGetHeight(defoldView.bounds)) {
            top = CGRectGetMidY(defoldView.bounds);
        }

        CGFloat left = CGRectGetMinX(bounds) + CGRectGetMidX(bannerAd.bounds);
        CGFloat right = CGRectGetMaxX(bounds) - CGRectGetMidX(bannerAd.bounds);
        CGFloat centerX = CGRectGetMidX(bounds);

        if (CGRectGetWidth(bannerAd.bounds) >= CGRectGetWidth(defoldView.bounds)) {
            left = CGRectGetMidX(defoldView.bounds);
        }

        CGPoint bannerPos = CGPointMake(centerX, centerY);
        switch (bannerSizeConst) {
            case POS_TOP_LEFT:
                bannerPos = CGPointMake(left, top);
            break;
            case POS_TOP_CENTER:
                bannerPos = CGPointMake(centerX, top);
            break;
            case POS_TOP_RIGHT:
                bannerPos = CGPointMake(right, top);
            break;
            case POS_BOTTOM_LEFT:
                bannerPos = CGPointMake(left, bottom);
            break;
            case POS_BOTTOM_CENTER:
                bannerPos = CGPointMake(centerX, bottom);
            break;
            case POS_BOTTOM_RIGHT:
                bannerPos = CGPointMake(right, bottom);
            break;
            case POS_CENTER:
                bannerPos = CGPointMake(centerX, centerY);
            break;
        }
        bannerAd.center = bannerPos;
    }

    bool IsBannerLoaded() {
        return bannerAd != nil;
    }

    void LoadBanner(const char* unitId, BannerSize bannerSize) {
        if (IsBannerLoaded()) {
          return;
        }
        bannerAd = [[GADBannerView alloc] initWithAdSize:GetSizeConstant(bannerSize)];
        bannerAd.adUnitID = [NSString stringWithUTF8String:unitId];
        bannerAd.delegate = admobExtBannerAdDelegate;
        bannerAd.rootViewController = uiViewController;
        bannerAd.hidden = YES;
        [uiViewController.view addSubview:bannerAd];
        [bannerAd loadRequest:[GADRequest request]];
    }

    void ShowBanner(BannerPosition bannerPos) {
        if (!IsBannerLoaded()) {
          return;
        }
        bannerAd.hidden = NO;
        if (bannerPos != lastBannerPos) {
            UpdatePosition(bannerPos);
            lastBannerPos = bannerPos;
        }
    }

    void HideBanner() {
        if (!IsBannerLoaded()) {
          return;
        }
        bannerAd.hidden = YES;
    }

    void DestroyBanner() {
        if (!IsBannerLoaded()) {
          return;
        }
        HideBanner();
        bannerAd.delegate = nil;
        [bannerAd release];
        bannerAd = nil;
        SendSimpleMessage(MSG_BANNER, EVENT_DESTROYED);
    }

//--------------------------------------------------

void Initialize_Ext() {
    UIWindow* window = dmGraphics::GetNativeiOSUIWindow();
    uiViewController = window.rootViewController;

    admobExtInterstitialAdDelegate = [[AdmobExtInterstitialAdDelegate alloc] init];
    admobExtRewardedAdDelegate = [[AdmobExtRewardedAdDelegate alloc] init];
    admobExtBannerAdDelegate = [[AdmobExtBannerAdDelegate alloc] init];
}

void SetPrivacySettings(bool enable_rdp) {
    [NSUserDefaults.standardUserDefaults setBool:enable_rdp ? YES : NO forKey:@"gad_rdp"];
}

void RequestIDFA() {
    if (@available(iOS 14, *))
    {
        [ATTrackingManager requestTrackingAuthorizationWithCompletionHandler:^(ATTrackingManagerAuthorizationStatus status) {
            switch (status) {
              case ATTrackingManagerAuthorizationStatusAuthorized:
                SendSimpleMessage(MSG_IDFA, EVENT_STATUS_AUTORIZED);
                break;
              case ATTrackingManagerAuthorizationStatusDenied:
                SendSimpleMessage(MSG_IDFA, EVENT_STATUS_DENIED);
                break;
              case ATTrackingManagerAuthorizationStatusNotDetermined:
                SendSimpleMessage(MSG_IDFA, EVENT_STATUS_NOT_DETERMINED);
                break;
              case ATTrackingManagerAuthorizationStatusRestricted:
                SendSimpleMessage(MSG_IDFA, EVENT_STATUS_RESTRICTED);
                break;
              }
        }];
    }
    else
    {
        SendSimpleMessage(MSG_IDFA, EVENT_NOT_SUPPORTED);
    }
}

} //namespace

@implementation AdmobExtInterstitialAdDelegate

- (void)adDidDismissFullScreenContent:(id)ad {
    dmAdmob::SetInterstitialAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_CLOSED);
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    dmAdmob::SetInterstitialAd(nil);
    dmAdmob::SendSimpleMessageIntString(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_FAILED_TO_SHOW, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)adDidPresentFullScreenContent:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_OPENING);
}

@end

@implementation AdmobExtRewardedAdDelegate

- (void)adDidDismissFullScreenContent:(id)ad {
    dmAdmob::SetRewardedAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_CLOSED);
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    dmAdmob::SetRewardedAd(nil);
    dmAdmob::SendSimpleMessageIntString(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_FAILED_TO_SHOW, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)adDidPresentFullScreenContent:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_OPENING);
}

@end

@implementation AdmobExtBannerAdDelegate

- (void)bannerViewDidReceiveAd:(GADBannerView *)bannerView {
    // Tells the delegate that an ad request successfully received an ad.
    // The delegate may want to add the banner view to the view hierarchy if it hasn’t been added yet.
    [dmAdmob::uiViewController.view addSubview:bannerView];
    CGFloat bannerHeight = CGRectGetHeight(CGRectStandardize(bannerView.frame)) * [UIScreen mainScreen].scale;
    CGFloat bannerWidth = CGRectGetWidth(CGRectStandardize(bannerView.frame)) * [UIScreen mainScreen].scale;
    dmAdmob::SendSimpleMessageIntInt(dmAdmob::MSG_BANNER, dmAdmob::EVENT_LOADED, @"height", (int)ceilf(bannerHeight), @"width", (int)ceilf(bannerWidth));
}

- (void)bannerView:(GADBannerView *)bannerView didFailToReceiveAdWithError:(NSError *)error {
    // Tells the delegate that an ad request failed.
    // The failure is normally due to network connectivity or ad availablility (i.e., no fill).
    dmAdmob::SendSimpleMessageIntString(dmAdmob::MSG_BANNER, dmAdmob::EVENT_FAILED_TO_LOAD, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)bannerViewDidRecordImpression:(GADBannerView *)bannerView {
    // Tells the delegate that an impression has been recorded for an ad.
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_BANNER, dmAdmob::EVENT_IMPRESSION_RECORDED);
}

- (void)bannerViewWillDismissScreen:(GADBannerView *)bannerView {
  // Tells the delegate that the full screen view will be dismissed.
}

- (void)bannerViewDidDismissScreen:(GADBannerView *)bannerView {
    // Tells the delegate that the full screen view has been dismissed.
    // The delegate should restart anything paused while handling bannerViewWillPresentScreen:.
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_BANNER, dmAdmob::EVENT_CLOSED);
}

- (void)bannerViewWillPresentScreen:(GADBannerView *)bannerView {
    // Tells the delegate that a full screen view will be presented in response to the user clicking on an ad.
    // The delegate may want to pause animations and time sensitive interactions.
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_BANNER, dmAdmob::EVENT_CLICKED);
}

@end

#endif
