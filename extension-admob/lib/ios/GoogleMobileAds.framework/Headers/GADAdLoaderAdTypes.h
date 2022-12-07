//
//  GADAdLoaderAdTypes.h
//  Google Mobile Ads SDK
//
//  Copyright 2015 Google LLC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GoogleMobileAds/GoogleMobileAdsDefines.h>

typedef NSString *GADAdLoaderAdType NS_STRING_ENUM;

/// Use with GADAdLoader to request native custom template ads. To receive ads, the ad loader's
/// delegate must conform to the GADCustomNativeAdLoaderDelegate protocol. See GADCustomNativeAd.h.
GAD_EXTERN GADAdLoaderAdType _Nonnull const GADAdLoaderAdTypeCustomNative;

/// Use with GADAdLoader to request Google Ad Manager banner ads. To receive ads, the ad loader's
/// delegate must conform to the GAMBannerAdLoaderDelegate protocol. See GAMBannerView.h.
GAD_EXTERN GADAdLoaderAdType _Nonnull const GADAdLoaderAdTypeGAMBanner;

/// Use with GADAdLoader to request native ads. To receive ads, the ad loader's delegate must
/// conform to the GADNativeAdLoaderDelegate protocol. See GADNativeAd.h.
GAD_EXTERN GADAdLoaderAdType _Nonnull const GADAdLoaderAdTypeNative;

#pragma mark Deprecated

/// Use with GADAdLoader to request native custom template ads. To receive ads, the ad loader's
/// delegate must conform to the GADCustomNativeAdLoaderDelegate protocol. See GADCustomNativeAd.h.
GAD_EXTERN GADAdLoaderAdType _Nonnull const kGADAdLoaderAdTypeCustomNative DEPRECATED_MSG_ATTRIBUTE(
    "Use GADAdLoaderAdTypeCustomNative instead") NS_SWIFT_NAME(kGADAdLoaderAdTypeCustomNative);

/// Use with GADAdLoader to request Google Ad Manager banner ads. To receive ads, the ad loader's
/// delegate must conform to the GAMBannerAdLoaderDelegate protocol. See GAMBannerView.h.
GAD_EXTERN GADAdLoaderAdType _Nonnull const kGADAdLoaderAdTypeGAMBanner DEPRECATED_MSG_ATTRIBUTE(
    "Use GADAdLoaderAdTypeGAMBanner instead") NS_SWIFT_NAME(kGADAdLoaderAdTypeGAMBanner);

/// Use with GADAdLoader to request native ads. To receive ads, the ad loader's delegate must
/// conform to the GADNativeAdLoaderDelegate protocol. See GADNativeAd.h.
GAD_EXTERN GADAdLoaderAdType _Nonnull const kGADAdLoaderAdTypeNative DEPRECATED_MSG_ATTRIBUTE(
    "Use GADAdLoaderAdTypeNative instead") NS_SWIFT_NAME(kGADAdLoaderAdTypeNative);
