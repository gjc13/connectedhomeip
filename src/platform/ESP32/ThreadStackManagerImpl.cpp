/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          ESP-IDF platforms.
 *
 */
#include <esp_err.h>
#include <freertos/portmacro.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp>

#include <platform/ESP32/ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>

#include "openthread-esp32.h"

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    esp_err_t err = esp_openthread_init(0, nullptr);

    if (err == ESP_OK)
    {
        CHIP_ERROR error = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(esp_openthread_get_instance());
        return error;
    }
    else
    {
        return CHIP_ERROR_INTERNAL;
    }
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    // Intentionally empty.
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_LockThreadStack()
{
    esp_openthread_lock_acquire(portMAX_DELAY);
}

bool ThreadStackManagerImpl::_TryLockThreadStack()
{
    // There's no openthread_api_mutex_try_lock() in Zephyr, so until it's contributed we must use the low-level API
    return esp_openthread_lock_acquire(0);
}

void ThreadStackManagerImpl::_UnlockThreadStack()
{
    esp_openthread_lock_release();
}

void ThreadStackManagerImpl::_ProcessThreadActivity()
{
    // Intentionally empty.
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStart()
{
    // Intentionally empty.
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStop()
{
    // Intentionally empty.
}
} // namespace DeviceLayer
} // namespace chip
