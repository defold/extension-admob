#pragma once

namespace dmAdmob {

enum MESSAGE_ID
{
    MSG_INIT = 1,
};

void Initialize_Ext();

void Initialize();
void LoadInterstitial(const char* placementId);
void ShowInterstitial();
void LoadRewarded(const char* placementId);
void ShowRewarded();

} //namespace dmAdmob
