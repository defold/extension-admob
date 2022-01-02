#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#pragma once

namespace dmAdmob {

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

void Initialize_Ext();

void Initialize();
void LoadInterstitial(const char* unitId);
void ShowInterstitial();
void LoadRewarded(const char* unitId);
void ShowRewarded();
void LoadBanner(const char* unitId, BannerSize bannerSize);
void DestroyBanner();
void ShowBanner(BannerPosition bannerPos);
void HideBanner();
void SetPrivacySettings(bool enable_rdp);
void RequestIDFA();
void ShowAdInspector();
void ActivateApp();

bool IsInterstitialLoaded();
bool IsRewardedLoaded();
bool IsBannerLoaded();

} //namespace dmAdmob

#endif
