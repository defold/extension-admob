#pragma once

namespace AdMobExtension {

enum AdMobAdType
{
    ADMOB_TYPE_BANNER,
    ADMOB_TYPE_INTERSTITIAL,
    ADMOB_TYPE_REWARDEDVIDEO,
    ADMOB_TYPE_NATIVEEXPRESS,
    ADMOB_TYPE_MAX,
};

enum AdMobError
{
    ADMOB_ERROR_NONE                = firebase::admob::kAdMobErrorNone,
    ADMOB_ERROR_UNINITIALIZED       = firebase::admob::kAdMobErrorUninitialized,
    ADMOB_ERROR_ALREADYINITIALIZED  = firebase::admob::kAdMobErrorAlreadyInitialized,
    ADMOB_ERROR_LOADINPROGRESS      = firebase::admob::kAdMobErrorLoadInProgress,
    ADMOB_ERROR_INTERNALERROR       = firebase::admob::kAdMobErrorInternalError,
    ADMOB_ERROR_INVALIDREQUEST      = firebase::admob::kAdMobErrorInvalidRequest,
    ADMOB_ERROR_NETWORKERROR        = firebase::admob::kAdMobErrorNetworkError,
    ADMOB_ERROR_NOFILL              = firebase::admob::kAdMobErrorNoFill,
    ADMOB_ERROR_NOWINDOWTOKEN       = firebase::admob::kAdMobErrorNoWindowToken,
};

// https://firebase.google.com/docs/reference/cpp/namespace/firebase/admob#namespacefirebase_1_1admob_1a1908085a11fb74e08dd24b1ec5b019ec
enum AdMobChildResquestTreatmentState
{
    ADMOB_CHILDDIRECTED_TREATMENT_STATE_NOT_TAGGED  = firebase::admob::kChildDirectedTreatmentStateNotTagged,
    ADMOB_CHILDDIRECTED_TREATMENT_STATE_TAGGED      = firebase::admob::kChildDirectedTreatmentStateTagged,
    ADMOB_CHILDDIRECTED_TREATMENT_STATE_UNKNOWN     = firebase::admob::kChildDirectedTreatmentStateUnknown
};

enum AdMobGender
{
    ADMOB_GENDER_UNKNOWN        = firebase::admob::kGenderUnknown,
    ADMOB_GENDER_FEMALE         = firebase::admob::kGenderFemale,
    ADMOB_GENDER_MALE           = firebase::admob::kGenderMale,
};

enum AdMobPosition
{
    ADMOB_POSITION_TOP          = firebase::admob::BannerView::kPositionTop,
    ADMOB_POSITION_BOTTOM       = firebase::admob::BannerView::kPositionBottom,
    ADMOB_POSITION_TOPLEFT      = firebase::admob::BannerView::kPositionTopLeft,
    ADMOB_POSITION_TOPRIGHT     = firebase::admob::BannerView::kPositionTopRight,
    ADMOB_POSITION_BOTTOMLEFT   = firebase::admob::BannerView::kPositionBottomLeft,
    ADMOB_POSITION_BOTTOMRIGHT  = firebase::admob::BannerView::kPositionBottomRight
};

enum AdMobEvent
{
    ADMOB_MESSAGE_LOADED,
    ADMOB_MESSAGE_FAILED_TO_LOAD,
    ADMOB_MESSAGE_SHOW,
    ADMOB_MESSAGE_HIDE,
    ADMOB_MESSAGE_REWARD,
    ADMOB_MESSAGE_APP_LEAVE,
    ADMOB_MESSAGE_UNLOADED,
};

}