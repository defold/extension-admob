#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#pragma once

#include <dmsdk/sdk.h>

namespace dmAdmob {


// The same constants/enums are in AdmobJNI.java
// If you change enums here, pls make sure you update the constants there as well
    
enum BannerSize
{
    SIZE_ADAPTIVE_BANNER =  0,
    SIZE_BANNER =           1,
    SIZE_FLUID =            2,
    SIZE_FULL_BANNER =      3,
    SIZE_LARGE_BANNER =     4,
    SIZE_LEADEARBOARD =     5,
    SIZE_MEDIUM_RECTANGLE = 6,
    SIZE_SEARH =            7,
    SIZE_SKYSCRAPER =       8,
    SIZE_SMART_BANNER =     9
};

enum BannerPosition
{
    POS_NONE =              0,
    POS_TOP_LEFT =          1,
    POS_TOP_CENTER =        2,
    POS_TOP_RIGHT =         3,
    POS_BOTTOM_LEFT =       4,
    POS_BOTTOM_CENTER =     5,
    POS_BOTTOM_RIGHT =      6,
    POS_CENTER =            7
};

enum MaxAdRating
{
    MAX_AD_CONTENT_RATING_G      = 0,
    MAX_AD_CONTENT_RATING_PG     = 1,
    MAX_AD_CONTENT_RATING_T      = 2,
    MAX_AD_CONTENT_RATING_MA     = 3
};

void Initialize_Ext(dmExtension::Params* params, const char* defoldUserAgent);
void Finalize_Ext();

void Initialize();
void LoadAppOpen(const char* unitId, bool showImmediately);
void ShowAppOpen();
void LoadInterstitial(const char* unitId);
void ShowInterstitial();
void LoadRewarded(const char* unitId, const char* userId,  const char* customData);
void ShowRewarded();
void LoadRewardedInterstitial(const char* unitId);
void ShowRewardedInterstitial();
void LoadBanner(const char* unitId, BannerSize bannerSize);
void DestroyBanner();
void ShowBanner(BannerPosition bannerPos);
void HideBanner();
void SetPrivacySettings(bool enable_rdp);
void RequestIDFA();
void ShowAdInspector();
void ActivateApp();
void SetMaxAdContentRating(MaxAdRating max_ad_rating);

bool IsAppOpenLoaded();
bool IsInterstitialLoaded();
bool IsRewardedLoaded();
bool IsRewardedInterstitialLoaded();
bool IsBannerLoaded();

} //namespace dmAdmob

#endif
