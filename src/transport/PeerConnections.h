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
#pragma once

#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <system/TimeSource.h>
#include <transport/PeerConnectionState.h>

namespace chip {
namespace Transport {

/**
 * Handles a set of peer connection states.
 *
 * Intended for:
 *   - handle connection active time and expiration
 *   - allocate and free space for connection states.
 */
template <size_t kMaxConnectionCount, Time::Source kTimeSource = Time::Source::kSystem>
class PeerConnections
{
public:
    /**
     * Allocates a new peer connection state state object out of the internal resource pool.
     *
     * @param address represents the connection state address
     * @param state [out] will contain the connection state if one was available. May be null if no return value is desired.
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum connection count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR CreateNewPeerConnectionState(const PeerAddress & address, PeerConnectionState ** state)
    {
        CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

        if (state)
        {
            *state = nullptr;
        }

        for (size_t i = 0; i < kMaxConnectionCount; i++)
        {
            if (!mStates[i].IsInitialized())
            {
                mStates[i] = PeerConnectionState(address);
                mStates[i].SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs());

                if (state)
                {
                    *state = &mStates[i];
                }

                err = CHIP_NO_ERROR;
                break;
            }
        }

        return err;
    }

    /**
     * Allocates a new peer connection state state object out of the internal resource pool.
     *
     * @param peerNode represents optional peer Node's ID
     * @param peerKeyId represents the encryption key ID assigned by peer node
     * @param localKeyId represents the encryption key ID assigned by local node
     * @param state [out] will contain the connection state if one was available. May be null if no return value is desired.
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum connection count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR CreateNewPeerConnectionState(const Optional<NodeId> & peerNode, uint16_t peerKeyId, uint16_t localKeyId,
                                            PeerConnectionState ** state)
    {
        CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

        if (state)
        {
            *state = nullptr;
        }

        for (size_t i = 0; i < kMaxConnectionCount; i++)
        {
            if (!mStates[i].IsInitialized())
            {
                mStates[i] = PeerConnectionState();
                mStates[i].SetPeerKeyID(peerKeyId);
                mStates[i].SetLocalKeyID(localKeyId);
                mStates[i].SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs());

                if (peerNode.HasValue())
                {
                    mStates[i].SetPeerNodeId(peerNode.Value());
                }

                if (state)
                {
                    *state = &mStates[i];
                }

                err = CHIP_NO_ERROR;
                break;
            }
        }

        return err;
    }

    /**
     * Get a peer connection state given a Peer address.
     *
     * @param address is the connection to find (based on address)
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerConnectionState * FindPeerConnectionState(const PeerAddress & address, PeerConnectionState * begin)
    {
        PeerConnectionState * state = nullptr;
        PeerConnectionState * now   = &mStates[0];

        if (begin >= now && begin <= &mStates[kMaxConnectionCount])
        {
            now = begin + 1;
        }

        for (; now <= &mStates[kMaxConnectionCount]; now++)
        {
            if (now->GetPeerAddress() == address)
            {
                state = now;
                break;
            }
        }
        return state;
    }

    /**
     * Get a peer connection state given a Node Id.
     *
     * @param nodeId is the connection to find (based on nodeId). Note that initial connections
     *        do not have a node id set. Use this if you know the node id should be set.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerConnectionState * FindPeerConnectionState(NodeId nodeId, PeerConnectionState * begin)
    {
        PeerConnectionState * state = nullptr;
        PeerConnectionState * now   = &mStates[0];

        if (begin >= now && begin <= &mStates[kMaxConnectionCount])
        {
            now = begin + 1;
        }

        for (; now <= &mStates[kMaxConnectionCount]; now++)
        {
            if (!now->IsInitialized())
            {
                continue;
            }
            if (now->GetPeerNodeId() == nodeId)
            {
                state = now;
                break;
            }
        }
        return state;
    }

    /**
     * Get a peer connection state given a Node Id and Peer's Encryption Key Id.
     *
     * @param nodeId is the connection to find (based on nodeId). Note that initial connections
     *        do not have a node id set. Use this if you know the node id should be set.
     * @param peerKeyId Encryption key ID used by the peer node.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerConnectionState * FindPeerConnectionState(Optional<NodeId> nodeId, uint16_t peerKeyId, PeerConnectionState * begin)
    {
        PeerConnectionState * state = nullptr;
        PeerConnectionState * now   = &mStates[0];

        if (begin >= now && begin <= &mStates[kMaxConnectionCount])
        {
            now = begin + 1;
        }

        for (; now <= &mStates[kMaxConnectionCount]; now++)
        {
            if (!now->IsInitialized())
            {
                continue;
            }
            if (now->GetPeerKeyID() == peerKeyId)
            {
                if (!nodeId.HasValue() || now->GetPeerNodeId() == kUndefinedNodeId || now->GetPeerNodeId() == nodeId.Value())
                {
                    state = now;
                    break;
                }
            }
        }
        return state;
    }

    /**
     * Get a peer connection state given a Node Id and Peer's Encryption Key Id.
     *
     * @param nodeId is the connection to find (based on peer nodeId). Note that initial connections
     *        do not have a node id set. Use this if you know the node id should be set.
     * @param localKeyId Encryption key ID used by the local node.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerConnectionState * FindPeerConnectionStateByLocalKey(Optional<NodeId> nodeId, uint16_t localKeyId,
                                                            PeerConnectionState * begin)
    {
        PeerConnectionState * state = nullptr;
        PeerConnectionState * now   = &mStates[0];

        if (begin >= now && begin <= &mStates[kMaxConnectionCount])
        {
            now = begin + 1;
        }

        for (; now <= &mStates[kMaxConnectionCount]; now++)
        {
            if (!now->IsInitialized())
            {
                continue;
            }
            if (now->GetLocalKeyID() == localKeyId)
            {
                if (!nodeId.HasValue() || now->GetPeerNodeId() == kUndefinedNodeId || now->GetPeerNodeId() == nodeId.Value())
                {
                    state = now;
                    break;
                }
            }
        }
        return state;
    }

    /// Convenience method to mark a peer connection state as active
    void MarkConnectionActive(PeerConnectionState * state)
    {
        state->SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs());
    }

    /// Convenience method to expired a peer connection state and fired the related callback
    void MarkConnectionExpired(PeerConnectionState * state)
    {
        if (OnConnectionExpired)
        {
            OnConnectionExpired(*state, mConnectionExpiredArgument);
        }

        *state = PeerConnectionState(PeerAddress::Uninitialized());
    }

    /**
     * Iterates through all active connections and expires any connection with an idle time
     * larger than the given amount.
     *
     * Expiring a connection involves callback execution and then clearing the internal state.
     */
    void ExpireInactiveConnections(uint64_t maxIdleTimeMs)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        for (size_t i = 0; i < kMaxConnectionCount; i++)
        {
            if (!mStates[i].GetPeerAddress().IsInitialized())
            {
                continue; // not an active connection
            }

            uint64_t connectionActiveTime = mStates[i].GetLastActivityTimeMs();
            if (connectionActiveTime + maxIdleTimeMs >= currentTime)
            {
                continue; // not expired
            }

            MarkConnectionExpired(&mStates[i]);
        }
    }

    /// Allows access to the underlying time source used for keeping track of connection active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

    /**
     * Sets the handler for expired connections
     *
     * @param[in] handler The callback to call when a connection is marked as expired
     * @param[in] param   The argument to pass in to the handler function
     *
     */
    template <class T>
    void SetConnectionExpiredHandler(void (*handler)(const PeerConnectionState &, T *), T * param)
    {
        mConnectionExpiredArgument = param;
        OnConnectionExpired        = reinterpret_cast<ConnectionExpiredHandler>(handler);
    }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    PeerConnectionState mStates[kMaxConnectionCount];

    typedef void (*ConnectionExpiredHandler)(const PeerConnectionState & state, void * param);

    ConnectionExpiredHandler OnConnectionExpired = nullptr; ///< Callback for connection expiry
    void * mConnectionExpiredArgument            = nullptr; ///< Argument for callback
};

} // namespace Transport
} // namespace chip
