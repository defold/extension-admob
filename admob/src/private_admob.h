#pragma once

namespace dmAdmob {

void Initialize_Ext();

void Initialize();
void LoadInterstitial(const char* unitId);
void ShowInterstitial();
void LoadRewarded(const char* unitId);
void ShowRewarded();
void LoadBanner(const char* unitId);
void ShowBanner();
void HideBanner();

bool IsInterstitialLoaded();
bool IsRewardedLoaded();

} //namespace dmAdmob
