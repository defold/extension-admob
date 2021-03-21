#pragma once

namespace dmAdmob {

void Initialize_Ext();

void Initialize();
void LoadInterstitial(const char* placementId);
void ShowInterstitial();
void LoadRewarded(const char* placementId);
void ShowRewarded();

bool IsInterstitialLoaded();
bool IsRewardedLoaded();

} //namespace dmAdmob
