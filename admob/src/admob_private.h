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

void Initialize_Ext();

void Initialize();
void LoadInterstitial(const char* unitId);
void ShowInterstitial();
void LoadRewarded(const char* unitId);
void ShowRewarded();
void LoadBanner(const char* unitId, BannerSize bannerSize);
void UnloadBanner();
void ShowBanner();
void HideBanner();

bool IsInterstitialLoaded();
bool IsRewardedLoaded();
bool IsBannerLoaded();

} //namespace dmAdmob
