/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef CHIP_PLATFORM_MDNS_H_
#define CHIP_PLATFORM_MDNS_H_

#include <stdint.h>

#include "core/CHIPError.h"
#include "inet/IPAddress.h"
#include "inet/InetInterface.h"

namespace chip {
namespace DeviceLayer {

static constexpr uint8_t kMdnsNameMaxSize  = 32;
static constexpr uint8_t kMdnsTypeMaxSize  = 32;
static constexpr uint16_t kMdnsTextMaxSize = 64;

enum class MdnsServiceProtocol : uint8_t
{
    kMdnsProtocolUdp = 0,
    kMdnsProtocolTcp,
    kMdnsProtocolUnknown = 255,
};

struct TextEntry
{
    const uint8_t * mData;
    size_t mSize;
};

struct MdnsService
{
    char mName[kMdnsNameMaxSize + 1];
    char mType[kMdnsTypeMaxSize + 1];
    MdnsServiceProtocol mProtocol;
    uint16_t mPort;
    chip::Inet::InterfaceId interface;
    TextEntry * mTextEntryies;
    size_t mTextEntrySize;
};

struct MdnsResolveResult
{
    MdnsService mService;
    chip::Inet::IPAddress mAddress;
};

/**
 * The callback function for mDNS resolve.
 *
 * The callback function SHALL NOT take the ownership of the result->mService.mTextEntries
 * memory.
 *
 * @param[in] context     The context passed to ChipMdnsBrowse or ChipMdnsResolve.
 * @param[in] result      The mdns resolve result, can be nullptr if error happens.
 * @param[in] error       The error code.
 *
 */
using MdnsResolveCallback = void (*)(void * context, MdnsResolveResult * result, CHIP_ERROR error);

/**
 * The callback function for mDNS browse.
 *
 * The callback function SHALL NOT take the ownership of the service->mTextEntries
 * memory.
 *
 * @param[in] context       The context passed to ChipMdnsBrowse or ChipMdnsResolve.
 * @param[in] services      The service list, can be nullptr.
 * @param[in] serciesSize   The size of the service list.
 * @param[in] error         The error code.
 *
 */
using MdnsBrowseCallback = void (*)(void * context, MdnsService * services, size_t servicesSize, CHIP_ERROR error);

using MdnsAsnycReturnCallback = void (*)(CHIP_ERROR error);

/**
 * This function intializes the mdns module
 *
 * @param[in] initCallback    The callback for notifying the initialization result
 * @param[in] errorCallback   The callback for notifying internal errors
 *
 * @retval CHIP_NO_ERROR  The initialization succeeds.
 * @retval Error code     The initialization fails
 *
 */
CHIP_ERROR ChipMdnsInit(MdnsAsnycReturnCallback initCallback, MdnsAsnycReturnCallback errorCallback);

/**
 * This function publishes an service via mDNS.
 *
 * Calling the function again with the same service name, type, protocol,
 * interface and port but different text will update the text published.
 * This function will NOT take the ownership of service->mTextEntries memory.
 *
 * @param[in] service   The service entry.
 *
 * @retval CHIP_NO_ERROR                The publish succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The service is nullptr.
 * @retval Error code                   The publish fails.
 *
 */
CHIP_ERROR ChipMdnsPublishService(const MdnsService * service);

/**
 * This function stops publishing service via mDNS.
 *
 * @retval CHIP_NO_ERROR                The publish stops successfully.
 * @retval Error code                   Stopping the publish fails.
 *
 */
CHIP_ERROR ChipMdnsStopPublish();

/**
 * This function browses the services published by mdns
 *
 * @param[in] type       The service type.
 * @param[in] protocol   The service protocol.
 * @param[in] interface  The interface to send queries.
 * @param[in] callback   The callback for found services.
 * @param[in] context    The user context.
 *
 * @retval CHIP_NO_ERROR                The browse succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The type or callback is nullptr.
 * @retval Error code                   The browse fails.
 *
 */
CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                          MdnsBrowseCallback callback, void * context);

/**
 * This function resolves the services published by mdns
 *
 * @param[in] name       The service instance name.
 * @param[in] type       The service type.
 * @param[in] protocol   The service protocol.
 * @param[in] interface  The interface to send queries.
 * @param[in] callback   The callback for found services.
 * @param[in] context    The user context.
 *
 * @retval CHIP_NO_ERROR                The resolve succeeds.
 * @retval CHIP_ERROR_INVALID_ARGUMENT  The name, type or callback is nullptr.
 * @retval Error code                   The resolve fails.
 *
 */
CHIP_ERROR ChipMdnsResolve(const char * name, const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                           MdnsResolveCallback callback, void * context);

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_PLATFORM_MDNS_H_
