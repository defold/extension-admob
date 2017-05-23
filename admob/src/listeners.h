#pragma once

#include "firebase/admob.h"
#include "firebase/admob/banner_view.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/native_express_ad_view.h"
#include "firebase/admob/rewarded_video.h"

namespace AdMobExtension {

class BannerViewListener : public firebase::admob::BannerView::Listener
{
public:
    BannerViewListener(int* coveringad, int id) : m_CoveringAdID(coveringad), m_Id(id) {}
    void OnBoundingBoxChanged(firebase::admob::BannerView* banner_view, firebase::admob::BoundingBox box) {
        (void)banner_view;
        (void)box;
    }
    void OnPresentationStateChanged(firebase::admob::BannerView* banner_view, firebase::admob::BannerView::PresentationState state);

    int*    m_CoveringAdID;
    int     m_Id;       // The internal ad number
};

class InterstitialAdListener : public firebase::admob::InterstitialAd::Listener
{
public:
    InterstitialAdListener(int* coveringad, int id) : m_CoveringAdID(coveringad), m_Id(id) {}
    void OnPresentationStateChanged(firebase::admob::InterstitialAd* interstitial_ad, firebase::admob::InterstitialAd::PresentationState state);

    int*    m_CoveringAdID;
    int m_Id; // The internal ad number
};


class NativeExpressAdViewListener : public firebase::admob::NativeExpressAdView::Listener
{
public:
    NativeExpressAdViewListener(int* coveringad, int id) : m_CoveringAdID(coveringad), m_Id(id) {}
    void OnBoundingBoxChanged(firebase::admob::NativeExpressAdView* ad_view, firebase::admob::BoundingBox box) {
        (void)ad_view;
        (void)box;
    }
    void OnPresentationStateChanged(firebase::admob::NativeExpressAdView* ad_view, firebase::admob::NativeExpressAdView::PresentationState state);
    int*    m_CoveringAdID;
    int     m_Id;       // The internal ad number
};

class RewardedVideoListener : public firebase::admob::rewarded_video::Listener
{
public:
    RewardedVideoListener(int* coveringad, int id) : m_CoveringAdID(coveringad), m_Id(id) {}
    void OnRewarded(firebase::admob::rewarded_video::RewardItem reward);
    void OnPresentationStateChanged(firebase::admob::rewarded_video::PresentationState state);

    int*    m_CoveringAdID;
    int     m_Id;       // The internal ad number
};

}