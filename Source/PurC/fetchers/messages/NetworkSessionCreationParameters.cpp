/*
 * Copyright (C) 2018-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "NetworkSessionCreationParameters.h"

#include "ArgumentCoders.h"

namespace PurCFetcher {

void NetworkSessionCreationParameters::encode(IPC::Encoder& encoder) const
{
    encoder << sessionID;
    encoder << boundInterfaceIdentifier;
    encoder << allowsCellularAccess;

    encoder << cookiePersistentStoragePath;
    encoder << cookiePersistentStorageType;

    encoder << networkCacheDirectory << networkCacheDirectoryExtensionHandle;

    encoder << deviceManagementRestrictionsEnabled;
    encoder << allLoadsBlockedByDeviceManagementRestrictionsForTesting;
    encoder << dataConnectionServiceType;
    encoder << fastServerTrustEvaluationEnabled;
    encoder << networkCacheSpeculativeValidationEnabled;
    encoder << shouldUseTestingNetworkSession;
    encoder << staleWhileRevalidateEnabled;
    encoder << testSpeedMultiplier;
    encoder << suppressesConnectionTerminationOnSystemChange;
    encoder << allowsServerPreconnect;
    encoder << requiresSecureHTTPSProxyConnection;
    encoder << preventsSystemHTTPProxyAuthentication;
    encoder << resourceLoadStatisticsParameters;
}

std::optional<NetworkSessionCreationParameters> NetworkSessionCreationParameters::decode(IPC::Decoder& decoder)
{
    std::optional<PAL::SessionID> sessionID;
    decoder >> sessionID;
    if (!sessionID)
        return std::nullopt;

    std::optional<String> boundInterfaceIdentifier;
    decoder >> boundInterfaceIdentifier;
    if (!boundInterfaceIdentifier)
        return std::nullopt;

    std::optional<AllowsCellularAccess> allowsCellularAccess;
    decoder >> allowsCellularAccess;
    if (!allowsCellularAccess)
        return std::nullopt;

    std::optional<String> cookiePersistentStoragePath;
    decoder >> cookiePersistentStoragePath;
    if (!cookiePersistentStoragePath)
        return std::nullopt;

    std::optional<CookiePersistentStorageType> cookiePersistentStorageType;
    decoder >> cookiePersistentStorageType;
    if (!cookiePersistentStorageType)
        return std::nullopt;

    std::optional<String> networkCacheDirectory;
    decoder >> networkCacheDirectory;
    if (!networkCacheDirectory)
        return std::nullopt;

    std::optional<SandboxExtension::Handle> networkCacheDirectoryExtensionHandle;
    decoder >> networkCacheDirectoryExtensionHandle;
    if (!networkCacheDirectoryExtensionHandle)
        return std::nullopt;

    std::optional<bool> deviceManagementRestrictionsEnabled;
    decoder >> deviceManagementRestrictionsEnabled;
    if (!deviceManagementRestrictionsEnabled)
        return std::nullopt;

    std::optional<bool> allLoadsBlockedByDeviceManagementRestrictionsForTesting;
    decoder >> allLoadsBlockedByDeviceManagementRestrictionsForTesting;
    if (!allLoadsBlockedByDeviceManagementRestrictionsForTesting)
        return std::nullopt;

    std::optional<String> dataConnectionServiceType;
    decoder >> dataConnectionServiceType;
    if (!dataConnectionServiceType)
        return std::nullopt;

    std::optional<bool> fastServerTrustEvaluationEnabled;
    decoder >> fastServerTrustEvaluationEnabled;
    if (!fastServerTrustEvaluationEnabled)
        return std::nullopt;

    std::optional<bool> networkCacheSpeculativeValidationEnabled;
    decoder >> networkCacheSpeculativeValidationEnabled;
    if (!networkCacheSpeculativeValidationEnabled)
        return std::nullopt;

    std::optional<bool> shouldUseTestingNetworkSession;
    decoder >> shouldUseTestingNetworkSession;
    if (!shouldUseTestingNetworkSession)
        return std::nullopt;

    std::optional<bool> staleWhileRevalidateEnabled;
    decoder >> staleWhileRevalidateEnabled;
    if (!staleWhileRevalidateEnabled)
        return std::nullopt;

    std::optional<unsigned> testSpeedMultiplier;
    decoder >> testSpeedMultiplier;
    if (!testSpeedMultiplier)
        return std::nullopt;

    std::optional<bool> suppressesConnectionTerminationOnSystemChange;
    decoder >> suppressesConnectionTerminationOnSystemChange;
    if (!suppressesConnectionTerminationOnSystemChange)
        return std::nullopt;

    std::optional<bool> allowsServerPreconnect;
    decoder >> allowsServerPreconnect;
    if (!allowsServerPreconnect)
        return std::nullopt;

    std::optional<bool> requiresSecureHTTPSProxyConnection;
    decoder >> requiresSecureHTTPSProxyConnection;
    if (!requiresSecureHTTPSProxyConnection)
        return std::nullopt;

    std::optional<bool> preventsSystemHTTPProxyAuthentication;
    decoder >> preventsSystemHTTPProxyAuthentication;
    if (!preventsSystemHTTPProxyAuthentication)
        return std::nullopt;

    std::optional<ResourceLoadStatisticsParameters> resourceLoadStatisticsParameters;
    decoder >> resourceLoadStatisticsParameters;
    if (!resourceLoadStatisticsParameters)
        return std::nullopt;

    return {{
        *sessionID
        , WTFMove(*boundInterfaceIdentifier)
        , WTFMove(*allowsCellularAccess)
        , WTFMove(*cookiePersistentStoragePath)
        , WTFMove(*cookiePersistentStorageType)
        , WTFMove(*deviceManagementRestrictionsEnabled)
        , WTFMove(*allLoadsBlockedByDeviceManagementRestrictionsForTesting)
        , WTFMove(*networkCacheDirectory)
        , WTFMove(*networkCacheDirectoryExtensionHandle)
        , WTFMove(*dataConnectionServiceType)
        , WTFMove(*fastServerTrustEvaluationEnabled)
        , WTFMove(*networkCacheSpeculativeValidationEnabled)
        , WTFMove(*shouldUseTestingNetworkSession)
        , WTFMove(*staleWhileRevalidateEnabled)
        , WTFMove(*testSpeedMultiplier)
        , WTFMove(*suppressesConnectionTerminationOnSystemChange)
        , WTFMove(*allowsServerPreconnect)
        , WTFMove(*requiresSecureHTTPSProxyConnection)
        , WTFMove(*preventsSystemHTTPProxyAuthentication)
        , WTFMove(*resourceLoadStatisticsParameters)
    }};
}

} // namespace PurCFetcher
