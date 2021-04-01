#if defined(DM_PLATFORM_IOS)
#include "admob_private.h"
#include "admob_callback_private.h"

#include <GoogleMobileAds/GoogleMobileAds.h>
#include <Foundation/Foundation.h>

namespace dmAdmob {

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

    void SendSimpleMessageInt(MessageId msg, MessageEvent event) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setObject:[NSNumber numberWithInt:event] forKey:@"event"];
        SendSimpleMessageObjCObject(msg, dict);
    }

    void Initialize_Ext() {

    }

    void Initialize() {
        [[GADMobileAds sharedInstance]
            startWithCompletionHandler:^(GADInitializationStatus *_Nonnull status) {
              SendSimpleMessageInt(MSG_INITIALIZATION, EVENT_COMPLETE);
            }];
    }

    void LoadInterstitial(const char* unitId){
        
    }

    void ShowInterstitial() {
        
    }

    void LoadRewarded(const char* unitId) {
        
    }

    void ShowRewarded() {
        
    }

    void LoadBanner(const char* unitId, BannerSize bannerSize) {

    }

    void DestroyBanner() {
        
    }

    void ShowBanner(BannerPosition bannerPos) {
        
    }

    void HideBanner() {
        
    }

    bool IsInterstitialLoaded() {

    }

    bool IsRewardedLoaded() {
        
    }

    bool IsBannerLoaded() {
        
    }

} //namespace

#endif
