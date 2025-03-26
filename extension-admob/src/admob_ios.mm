#if defined(DM_PLATFORM_IOS)
#include "admob_private.h"
#include "admob_callback_private.h"

#include <GoogleMobileAds/GoogleMobileAds.h>
#include <UIKit/UIKit.h>
#import <AdSupport/AdSupport.h>

#if __has_include(<AppTrackingTransparency/ATTrackingManager.h>)
#import <AppTrackingTransparency/ATTrackingManager.h>
#endif

@interface AdmobExtAppOpenAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@interface AdmobExtInterstitialAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@interface AdmobExtRewardedAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@interface AdmobExtRewardedInterstitialAdDelegate : NSObject<GADFullScreenContentDelegate>
@end

@interface AdmobExtBannerAdDelegate : NSObject<GADBannerViewDelegate>
@end

@interface AdMobAppDelegate : NSObject <UIApplicationDelegate>
@end

namespace dmAdmob {

    static const char* m_DefoldUserAgent = nil;

    static UIViewController *uiViewController = nil;
    static AdMobAppDelegate *admobAppDelegate = nil;

    void SendSimpleMessage(MessageId msg, id obj) {
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
        SendSimpleMessage(msg, dict);
    }

    void SendSimpleMessage(MessageId msg, MessageEvent event, NSString *key_2, NSString *value_2) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        [dict setObject:value_2 forKey:key_2];
        SendSimpleMessage(msg, dict);
    }

    void SendSimpleMessage(MessageId msg, MessageEvent event, NSString *key_2, int value_2, NSString *key_3, NSString *value_3) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        [dict setObject:[NSNumber numberWithInt:value_2] forKey:key_2];
        [dict setObject:value_3 forKey:key_3];
        SendSimpleMessage(msg, dict);
    }

    void SendSimpleMessage(MessageId msg, MessageEvent event, NSString *key_2, int value_2, NSString *key_3, int value_3) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        [dict setObject:[NSNumber numberWithInt:value_2] forKey:key_2];
        [dict setObject:[NSNumber numberWithInt:value_3] forKey:key_3];
        SendSimpleMessage(msg, dict);
    }

    void Initialize() {
        [[GADMobileAds sharedInstance]
        startWithCompletionHandler:^(GADInitializationStatus *_Nonnull status) {
            SendSimpleMessage(MSG_INITIALIZATION, EVENT_COMPLETE);
        }];
    }

    GADRequest* createGADRequest() {
        GADRequest *request = [GADRequest request];
        request.requestAgent = [NSString stringWithUTF8String: m_DefoldUserAgent];
        return request;
    }


//--------------------------------------------------
// App Open ADS

    static AdmobExtAppOpenAdDelegate *admobExtAppOpenAdDelegate;
    static GADAppOpenAd *appOpenAd;
    static char *appOpenAdId;
    static bool isLoadingAppOpenAd;
    static bool isShowingAppOpenAd;

    void SetAppOpenAd(GADAppOpenAd *newAd) {
        if (appOpenAd == newAd) {
            return;
        }
        if (newAd != nil) {
            [newAd retain];
        }
        if (appOpenAd != nil) {
            [appOpenAd release];
        }
        appOpenAd = newAd;
    }

    void LoadAppOpen(const char* unitId, bool showImmediately) {
        // Do not load ad if one is already loading.
        if (isLoadingAppOpenAd) {
            NSLog(@"Already loading app open ad.");
            return;
        }

        // Do not load ad if there is an unused ad.
        if (IsAppOpenLoaded()) {
            SendSimpleMessage(MSG_APPOPEN, EVENT_LOADED);
            if (showImmediately) {
                ShowAppOpen();
            }
            return;
        }

        appOpenAdId = (char*)unitId;
        isLoadingAppOpenAd = true;
        [GADAppOpenAd 
            loadWithAdUnitID:[NSString stringWithUTF8String:unitId]
            request:createGADRequest()
            completionHandler:^(GADAppOpenAd *_Nullable ad, NSError *_Nullable error) {
                isLoadingAppOpenAd = false;
                if (error) {
                    SetAppOpenAd(nil);
                    NSLog([NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    SendSimpleMessage(MSG_APPOPEN, EVENT_FAILED_TO_LOAD, @"code", [error code],
                        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    return;
                }
                SetAppOpenAd(ad);
                SendSimpleMessage(MSG_APPOPEN, EVENT_LOADED);
                if (showImmediately) {
                    ShowAppOpen();
                }
         }];
    }

    bool IsAppOpenLoaded() {
        return appOpenAd != nil;
    }

    void ShowAppOpen() {
        // If the app open ad is already showing, do not show the ad again.
        if (isShowingAppOpenAd) {
            NSLog(@"The app open ad is already showing.");
            return;
        }

        if (!IsAppOpenLoaded()) {
            LoadAppOpen(appOpenAdId, false);
            return;
        }
        isShowingAppOpenAd = true;
        NSError* error;
        appOpenAd.fullScreenContentDelegate = admobExtAppOpenAdDelegate;
        if ([appOpenAd canPresentFromRootViewController:uiViewController error:&error]) {
            [appOpenAd presentFromRootViewController:uiViewController];
        } else {
            if (error) {
                SendSimpleMessage(MSG_APPOPEN, EVENT_NOT_LOADED, @"code", [error code],
                      @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
            } else {
                SendSimpleMessage(MSG_APPOPEN, EVENT_NOT_LOADED, @"error", @"Can't present App Open Ad");
            }
        }
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
        [GADInterstitialAd
            loadWithAdUnitID:[NSString stringWithUTF8String:unitId]
            request:createGADRequest()
            completionHandler:^(GADInterstitialAd *ad, NSError *error) {
                if (error) {
                    SetInterstitialAd(nil);
                    NSLog([NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    SendSimpleMessage(MSG_INTERSTITIAL, EVENT_FAILED_TO_LOAD, @"code", [error code],
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
                    SendSimpleMessage(MSG_INTERSTITIAL, EVENT_NOT_LOADED, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                } else {
                    SendSimpleMessage(MSG_INTERSTITIAL, EVENT_NOT_LOADED, @"error", @"Can't present interstitial AD");
                }
            }
        } else {
            SendSimpleMessage(MSG_INTERSTITIAL, EVENT_NOT_LOADED, @"error", @"Can't show interstitial AD that wasn't loaded.");
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

    void LoadRewarded(const char* unitId, const char* userId, const char* customData) {
        GADServerSideVerificationOptions *ssvOptions = [[GADServerSideVerificationOptions alloc] init];
        if (userId) {
            ssvOptions.userIdentifier = [NSString stringWithUTF8String:userId];
        }
        if (customData) {
            ssvOptions.customRewardString = [NSString stringWithUTF8String:customData];
        }
        [GADRewardedAd
            loadWithAdUnitID:[NSString stringWithUTF8String:unitId]
            request:createGADRequest()
            completionHandler:^(GADRewardedAd *ad, NSError *error) {
                if (error) {
                    SetRewardedAd(nil);
                    NSLog([NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    SendSimpleMessage(MSG_REWARDED, EVENT_FAILED_TO_LOAD, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    return;
                }
                ad.serverSideVerificationOptions = ssvOptions;
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
                            SendSimpleMessage(MSG_REWARDED, EVENT_EARNED_REWARD, @"amount", [reward.amount doubleValue], @"type", [reward type]);
                        }];
            } else {
                if (error) {
                    SendSimpleMessage(MSG_REWARDED, EVENT_NOT_LOADED, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                } else {
                    SendSimpleMessage(MSG_REWARDED, EVENT_NOT_LOADED, @"error", @"Can't present rewarded AD");
                }
            }
        } else {
            SendSimpleMessage(MSG_REWARDED, EVENT_NOT_LOADED, @"error", @"Can't show rewarded AD that wasn't loaded.");
        }
    }

//--------------------------------------------------
// Rewarded Interstitial ADS

    static GADRewardedInterstitialAd *rewardedInterstitialAd = nil;
    static AdmobExtRewardedInterstitialAdDelegate *admobExtRewardedInterstitialAdDelegate;

    void SetRewardedInterstitialAd(GADRewardedInterstitialAd *newAd) {
        if (rewardedInterstitialAd == newAd) {
            return;
        }
        if (newAd != nil) {
            [newAd retain];
        }
        if (rewardedInterstitialAd != nil) {
            [rewardedInterstitialAd release];
        }
        rewardedInterstitialAd = newAd;
    }

    void LoadRewardedInterstitial(const char* unitId) {
        [GADRewardedInterstitialAd
            loadWithAdUnitID:[NSString stringWithUTF8String:unitId]
            request:createGADRequest()
            completionHandler:^(GADRewardedInterstitialAd *ad, NSError *error) {
                if (error) {
                    SetRewardedInterstitialAd(nil);
                    NSLog([NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    SendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_FAILED_TO_LOAD, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                    return;
                }
                SetRewardedInterstitialAd(ad);
                SendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_LOADED);
            }];
    }

    bool IsRewardedInterstitialLoaded() {
        return rewardedInterstitialAd != nil;
    }

    void ShowRewardedInterstitial() {
        if (IsRewardedInterstitialLoaded()) {
            rewardedInterstitialAd.fullScreenContentDelegate = admobExtRewardedInterstitialAdDelegate;
            NSError* error;
            if ([rewardedInterstitialAd canPresentFromRootViewController:uiViewController error:&error]) {
                [rewardedInterstitialAd presentFromRootViewController:uiViewController
                    userDidEarnRewardHandler:^{
                            GADAdReward *reward = rewardedInterstitialAd.adReward;
                            SendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_EARNED_REWARD, @"amount", [reward.amount doubleValue], @"type", [reward type]);
                        }];
            } else {
                if (error) {
                    SendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_NOT_LOADED, @"code", [error code],
                          @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
                } else {
                    SendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_NOT_LOADED, @"error", @"Can't present rewarded interstitial AD");
                }
            }
        } else {
            SendSimpleMessage(MSG_REWARDED_INTERSTITIAL, EVENT_NOT_LOADED, @"error", @"Can't show rewarded interstitial AD that wasn't loaded.");
        }
    }



//--------------------------------------------------
// Banner ADS

    static GADBannerView *bannerAd = nil;
    static AdmobExtBannerAdDelegate *admobExtBannerAdDelegate;
    static BannerPosition lastBannerPos = POS_NONE;

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
            bannerSize = GADAdSizeBanner;
            break;
          case SIZE_FULL_BANNER:
            bannerSize = GADAdSizeFullBanner;
            break;
          case SIZE_LARGE_BANNER:
            bannerSize = GADAdSizeLargeBanner;
            break;
          case SIZE_LEADEARBOARD:
            bannerSize = GADAdSizeLeaderboard;
            break;
          case SIZE_MEDIUM_RECTANGLE:
            bannerSize = GADAdSizeMediumRectangle;
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

    void UpdatePosition() {
        UpdatePosition(lastBannerPos);
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
        [bannerAd loadRequest:createGADRequest()];
    }

    void ShowBanner(BannerPosition bannerPos) {
        if (!IsBannerLoaded()) {
          return;
        }
        bannerAd.hidden = NO;
        if (bannerPos != lastBannerPos || lastBannerPos == POS_NONE) {
            if (bannerPos != POS_NONE) {
                lastBannerPos = bannerPos;
            }
            UpdatePosition();
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

void Initialize_Ext(dmExtension::Params* params, const char* defoldUserAgent) {
    m_DefoldUserAgent = defoldUserAgent;

    UIWindow* window = dmGraphics::GetNativeiOSUIWindow();
    uiViewController = window.rootViewController;

    admobExtInterstitialAdDelegate = [[AdmobExtInterstitialAdDelegate alloc] init];
    admobExtRewardedAdDelegate = [[AdmobExtRewardedAdDelegate alloc] init];
    admobExtRewardedInterstitialAdDelegate = [[AdmobExtRewardedInterstitialAdDelegate alloc] init];
    admobExtBannerAdDelegate = [[AdmobExtBannerAdDelegate alloc] init];
    admobExtAppOpenAdDelegate = [[AdmobExtAppOpenAdDelegate alloc] init];
    admobAppDelegate = [[AdMobAppDelegate alloc] init];

    appOpenAdId = (char*)dmConfigFile::GetString(params->m_ConfigFile, "admob.app_open_ios", 0);
    if (appOpenAdId) {
        LoadAppOpen(appOpenAdId, true);
    }

    dmExtension::RegisteriOSUIApplicationDelegate(admobAppDelegate);
    dmExtension::RegisteriOSUIApplicationDelegate(admobExtAppOpenAdDelegate);
}

void Finalize_Ext() {
    dmExtension::UnregisteriOSUIApplicationDelegate(admobAppDelegate);
    [admobExtInterstitialAdDelegate dealloc];
    [admobExtRewardedAdDelegate dealloc];
    [admobExtRewardedInterstitialAdDelegate dealloc];
    [admobExtBannerAdDelegate dealloc];
    [admobAppDelegate dealloc];
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
                SendSimpleMessage(MSG_IDFA, EVENT_STATUS_AUTHORIZED);
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

void ShowAdInspector() {
    [GADMobileAds.sharedInstance presentAdInspectorFromViewController:uiViewController completionHandler:^(NSError *error) { 
        // error will be non-nil if there was an issue and the inspector was not displayed. 
        if (error) {
            dmLogInfo("AdInspector Error: %s", error);
        }
    }];
}

void ActivateApp() {
    ShowAppOpen();
}

void SetMaxAdContentRating(MaxAdRating max_ad_rating) {
    switch (max_ad_rating) {
        case MAX_AD_CONTENT_RATING_G:
            GADMobileAds.sharedInstance.requestConfiguration.maxAdContentRating = GADMaxAdContentRatingGeneral;
            break;
        case MAX_AD_CONTENT_RATING_PG:
            GADMobileAds.sharedInstance.requestConfiguration.maxAdContentRating = GADMaxAdContentRatingParentalGuidance;
            break;
        case MAX_AD_CONTENT_RATING_T:
            GADMobileAds.sharedInstance.requestConfiguration.maxAdContentRating = GADMaxAdContentRatingTeen;
            break;
        case MAX_AD_CONTENT_RATING_MA:
            GADMobileAds.sharedInstance.requestConfiguration.maxAdContentRating = GADMaxAdContentRatingMatureAudience;
            break;
    }
}

} //namespace

@implementation AdmobExtAppOpenAdDelegate

- (void)adWillPresentFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_APPOPEN, dmAdmob::EVENT_OPENING);
}

- (void)adDidDismissFullScreenContent:(nonnull id<GADFullScreenPresentingAd>)ad {
    dmAdmob::isShowingAppOpenAd = false;
    dmAdmob::SetAppOpenAd(nil);
    dmAdmob::LoadAppOpen(dmAdmob::appOpenAdId, false);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_APPOPEN, dmAdmob::EVENT_CLOSED);
}

- (void)ad:(nonnull id<GADFullScreenPresentingAd>)ad didFailToPresentFullScreenContentWithError:(nonnull NSError *)error {
    dmAdmob::isShowingAppOpenAd = false;
    dmAdmob::SetAppOpenAd(nil);
    dmAdmob::LoadAppOpen(dmAdmob::appOpenAdId, false);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_APPOPEN, dmAdmob::EVENT_FAILED_TO_SHOW, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)adDidRecordImpression:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_APPOPEN, dmAdmob::EVENT_IMPRESSION_RECORDED);
}

- (void)adDidRecordClick:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_APPOPEN, dmAdmob::EVENT_CLICKED);
}

@end

@implementation AdmobExtInterstitialAdDelegate

- (void)adDidDismissFullScreenContent:(id)ad {
    dmAdmob::SetInterstitialAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_CLOSED);
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    dmAdmob::SetInterstitialAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_FAILED_TO_SHOW, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)adWillPresentFullScreenContent:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_OPENING);
}

- (void)adDidRecordImpression:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_IMPRESSION_RECORDED);
}

- (void)adDidRecordClick:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_INTERSTITIAL, dmAdmob::EVENT_CLICKED);
}

@end

@implementation AdmobExtRewardedAdDelegate

- (void)adDidDismissFullScreenContent:(id)ad {
    dmAdmob::SetRewardedAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_CLOSED);
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    dmAdmob::SetRewardedAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_FAILED_TO_SHOW, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)adWillPresentFullScreenContent:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_OPENING);
}

- (void)adDidRecordImpression:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_IMPRESSION_RECORDED);
}
- (void)adDidRecordClick:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED, dmAdmob::EVENT_CLICKED);
}

@end

@implementation AdmobExtRewardedInterstitialAdDelegate

- (void)adDidDismissFullScreenContent:(id)ad {
    dmAdmob::SetRewardedInterstitialAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED_INTERSTITIAL, dmAdmob::EVENT_CLOSED);
}

- (void)ad:(id)ad didFailToPresentFullScreenContentWithError:(NSError *)error {
    dmAdmob::SetRewardedInterstitialAd(nil);
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED_INTERSTITIAL, dmAdmob::EVENT_FAILED_TO_SHOW, @"code", [error code],
        @"error", [NSString stringWithFormat:@"Error domain: \"%@\". %@", [error domain], [error localizedDescription]]);
}

- (void)adWillPresentFullScreenContent:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED_INTERSTITIAL, dmAdmob::EVENT_OPENING);
}

- (void)adDidRecordImpression:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED_INTERSTITIAL, dmAdmob::EVENT_IMPRESSION_RECORDED);
}
- (void)adDidRecordClick:(id)ad {
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_REWARDED_INTERSTITIAL, dmAdmob::EVENT_CLICKED);
}

@end

@implementation AdmobExtBannerAdDelegate

- (void)bannerViewDidReceiveAd:(GADBannerView *)bannerView {
    // Tells the delegate that an ad request successfully received an ad.
    // The delegate may want to add the banner view to the view hierarchy if it hasn’t been added yet.
    [dmAdmob::uiViewController.view addSubview:bannerView];
    CGFloat bannerHeight = CGRectGetHeight(CGRectStandardize(bannerView.frame)) * [UIScreen mainScreen].scale;
    CGFloat bannerWidth = CGRectGetWidth(CGRectStandardize(bannerView.frame)) * [UIScreen mainScreen].scale;
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_BANNER, dmAdmob::EVENT_LOADED, @"height", (int)ceilf(bannerHeight), @"width", (int)ceilf(bannerWidth));
}

- (void)bannerView:(GADBannerView *)bannerView didFailToReceiveAdWithError:(NSError *)error {
    // Tells the delegate that an ad request failed.
    // The failure is normally due to network connectivity or ad availablility (i.e., no fill).
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_BANNER, dmAdmob::EVENT_FAILED_TO_LOAD, @"code", [error code],
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

- (void)bannerViewDidRecordClick:(GADBannerView *)bannerView {
    // Tells the delegate that a full screen view will be presented in response to the user clicking on an ad.
    // The delegate may want to pause animations and time sensitive interactions.
    dmAdmob::SendSimpleMessage(dmAdmob::MSG_BANNER, dmAdmob::EVENT_CLICKED);
}

@end

@implementation AdMobAppDelegate

- (instancetype)init {
    self = [super init];
    if (self) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
    }
    return self;
}

- (void) UpdatePosition {
    dmAdmob::UpdatePosition();
}

- (void)orientationDidChange:(NSNotification *)notification {
    [self performSelector:@selector(UpdatePosition) withObject:nil afterDelay:0.1];
}

- (void) applicationDidBecomeActive:(UIApplication *)application {
    NSLog(@"applicationDidBecomeActive.");
    //dmAdmob::ShowAppOpen();
    dmAdmob::LoadAppOpen(dmAdmob::appOpenAdId, true);
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
    [super dealloc];
}

@end

#endif
