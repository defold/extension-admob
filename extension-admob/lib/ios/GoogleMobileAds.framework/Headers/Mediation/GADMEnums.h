//
//  GADMEnums.h
//  Google Mobile Ads SDK
//
//  Copyright 2011 Google. All rights reserved.
//

#import <Foundation/Foundation.h>

/// These are the types of animation we employ for transitions between two mediated ads.
typedef NS_ENUM(NSInteger, GADMBannerAnimationType) {
  GADMBannerAnimationTypeNone = 0,            ///< No animation.
  GADMBannerAnimationTypeFlipFromLeft = 1,    ///< Flip from left.
  GADMBannerAnimationTypeFlipFromRight = 2,   ///< Flip from right.
  GADMBannerAnimationTypeCurlUp = 3,          ///< Curl up.
  GADMBannerAnimationTypeCurlDown = 4,        ///< Curl down.
  GADMBannerAnimationTypeSlideFromLeft = 5,   ///< Slide from left.
  GADMBannerAnimationTypeSlideFromRight = 6,  ///< Slide from right.
  GADMBannerAnimationTypeFadeIn = 7,          ///< Fade in.
  GADMBannerAnimationTypeRandom = 8,          ///< Random animation.

  /// Deprecated enums.
  kGADMBannerAnimationTypeNone = 0,            ///< Deprecated. No animation.
  kGADMBannerAnimationTypeFlipFromLeft = 1,    ///< Deprecated. Flip from left.
  kGADMBannerAnimationTypeFlipFromRight = 2,   ///< Deprecated. Flip from right.
  kGADMBannerAnimationTypeCurlUp = 3,          ///< Deprecated. Curl up.
  kGADMBannerAnimationTypeCurlDown = 4,        ///< Deprecated. Curl down.
  kGADMBannerAnimationTypeSlideFromLeft = 5,   ///< Deprecated. Slide from left.
  kGADMBannerAnimationTypeSlideFromRight = 6,  ///< Deprecated. Slide from right.
  kGADMBannerAnimationTypeFadeIn = 7,          ///< Deprecated. Fade in.
  kGADMBannerAnimationTypeRandom = 8,          ///< Deprecated. Random animation.
};
