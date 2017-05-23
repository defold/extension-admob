#include "listeners.h"
#include "enums.h"

namespace AdMobExtension {
    
typedef void (*PostCommandFn)(int id);
extern void QueueCommand(int id, int message, int firebase_result, const char* firebase_message, PostCommandFn fn);
extern void QueueRewardCommand(int id, int message, float reward, const char* reward_type);

void BannerViewListener::OnPresentationStateChanged(firebase::admob::BannerView* banner_view, firebase::admob::BannerView::PresentationState state)
{
    if( state == firebase::admob::BannerView::kPresentationStateCoveringUI ) // When clicked
    {
        if( *m_CoveringAdID == -1 ) // Because the state change gets triggered twice
        {
            *m_CoveringAdID = m_Id;
            QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_APP_LEAVE, 0, 0, 0);
        }
    }
    else if( state == firebase::admob::BannerView::kPresentationStateHidden )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_HIDE, 0, 0, 0);
    }
    else if( state == firebase::admob::BannerView::kPresentationStateVisibleWithAd )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_SHOW, 0, 0, 0);
    }
}

void InterstitialAdListener::OnPresentationStateChanged(firebase::admob::InterstitialAd* interstitial_ad, firebase::admob::InterstitialAd::PresentationState state)
{
    // When showing ad, it also leaves the app
    if( state == firebase::admob::InterstitialAd::kPresentationStateCoveringUI )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_SHOW, 0, 0, 0);
        if(*m_CoveringAdID == -1)
        {
            *m_CoveringAdID = m_Id;
            QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_APP_LEAVE, 0, 0, 0);
        }
    }
    else if( state == firebase::admob::InterstitialAd::kPresentationStateHidden )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_HIDE, 0, 0, 0);
    }
}

void RewardedVideoListener::OnRewarded(firebase::admob::rewarded_video::RewardItem reward)
{
    QueueRewardCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_REWARD, reward.amount, reward.reward_type.c_str());
}

void RewardedVideoListener::OnPresentationStateChanged(firebase::admob::rewarded_video::PresentationState state)
{
    // When showing ad, it also leaves the app
    if( state == firebase::admob::rewarded_video::kPresentationStateCoveringUI ||
        state == firebase::admob::rewarded_video::kPresentationStateVideoHasStarted )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_SHOW, 0, 0, 0);
        if(*m_CoveringAdID == -1)
        {
            *m_CoveringAdID = m_Id;
            QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_APP_LEAVE, 0, 0, 0);
        }
    }
    else if( state == firebase::admob::rewarded_video::kPresentationStateHidden )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_HIDE, 0, 0, 0);
    }
}

void NativeExpressAdViewListener::OnPresentationStateChanged(firebase::admob::NativeExpressAdView* ad_view, firebase::admob::NativeExpressAdView::PresentationState state)
{
    if( state == firebase::admob::NativeExpressAdView::kPresentationStateCoveringUI ) // When clicked
    {
        if( *m_CoveringAdID == -1 ) // Because the state change gets triggered twice
        {
            *m_CoveringAdID = m_Id;
            QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_APP_LEAVE, 0, 0, 0);
        }
    }
    else if( state == firebase::admob::NativeExpressAdView::kPresentationStateHidden )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_HIDE, 0, 0, 0);
    }
    else if( state == firebase::admob::NativeExpressAdView::kPresentationStateVisibleWithAd )
    {
        QueueCommand(m_Id, AdMobExtension::ADMOB_MESSAGE_SHOW, 0, 0, 0);
    }
}


}