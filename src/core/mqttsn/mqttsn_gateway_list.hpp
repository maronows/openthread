/*
 *  Copyright (c) 2018, Vit Holasek
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MQTTSN_GATEWAY_LIST_HPP_
#define MQTTSN_GATEWAY_LIST_HPP_

#include <openthread/mqttsn.h>
#include "net/ip6_address.hpp"

namespace ot {

namespace Mqttsn {

typedef otMqttsnGatewayId GatewayId;

template <typename ItemType> class StaticListItem;

template <typename ItemType>
class StaticArrayList
{
public:
    StaticArrayList(StaticListItem<ItemType> *aItems, uint16_t aMaxSize)
        : mHead(NULL)
        , mItems(aItems)
        , mMaxSize(aMaxSize)
        , mSize(0)
    {
        Clear();
    }

    StaticListItem<ItemType> *Head() { return mHead; }

    const StaticListItem<ItemType> *Head(void) const { return mHead; }

    otError Add(const ItemType &aValue);

    otError Remove(StaticListItem<ItemType> *aItem);

    uint16_t Size(void) const { return mSize; };

    bool IsEmpty(void) const { return mSize == 0; };

    void Clear(void);

private:
    StaticListItem<ItemType> *mHead;
    StaticListItem<ItemType> *mItems;
    uint16_t mMaxSize;
    uint16_t mSize;
};

template <typename ItemType>
class StaticListItem
{
public:
    StaticListItem()
        : mValue()
        , mNext(NULL)
        , mIsRemoved(false)
    {
        ;
    }

    StaticListItem(const ItemType &aValue)
        : mValue(aValue)
        , mNext(NULL)
        , mIsRemoved(false)
    {
        ;
    }

    ItemType &Value()
    {
        return mValue;
    }

    const ItemType &Value() const
    {
        return mValue;
    }

    bool HasNext() const
    {
        return mNext != NULL;
    }

    StaticListItem<ItemType> *Next()
    {
        return mNext;
    }

    const StaticListItem<ItemType> *Next() const
    {
        return mNext;
    }
private:
    ItemType mValue;
    StaticListItem<ItemType> *mNext;
    bool mIsRemoved;

    template<typename> friend class StaticArrayList;
};

class ActiveGatewayList;

class GatewayInfo : public otMqttsnGatewayInfo
{
public:
    GatewayInfo()
        : mLastUpdatedTimestamp()
        , mDuration()
    {
        mGatewayId = 0;
        mGatewayAddress = ot::Ip6::Address();
    }

    GatewayInfo(GatewayId aGatewayId, const ot::Ip6::Address &aGatewayAddress,
            uint32_t aLastUpdatedTimestamp, uint32_t aDuration)
        : mLastUpdatedTimestamp(aLastUpdatedTimestamp)
        , mDuration(aDuration)
    {
        mGatewayId = aGatewayId;
        mGatewayAddress = aGatewayAddress;
    }

    GatewayId GetGatewayId() const
    {
        return mGatewayId;
    }

    const ot::Ip6::Address &GetGatewayAddress() const
    {
        return *static_cast<const ot::Ip6::Address *>(&mGatewayAddress);
    }

    bool operator==(const GatewayInfo &aOther) const
    {
        return GetGatewayAddress() == aOther.GetGatewayAddress() && mGatewayId == aOther.mGatewayId
                && mLastUpdatedTimestamp == aOther.mLastUpdatedTimestamp
                && mDuration == aOther.mDuration;
    }

    bool operator!=(const GatewayInfo &aOther) const { return !(*this == aOther); }

private:
    uint32_t mLastUpdatedTimestamp;
    uint32_t mDuration;

    friend class ActiveGatewayList;
};

class ActiveGatewayList
{
public:
    ActiveGatewayList()
        : mGatewayInfoListArray()
        , mGatewayInfoList(mGatewayInfoListArray, kMaxGatewayInfoCount)
    {

    }

    otError Add(GatewayId aGatewayId, ot::Ip6::Address aGatewayAddress, uint32_t aDuration);

    bool IsEmpty(void) const;

    void Clear();

    otError HandleTimer(void);

    const StaticArrayList<GatewayInfo> &GetList();

private:
    GatewayInfo *Find(GatewayId aGatewayId);

    StaticListItem<GatewayInfo> mGatewayInfoListArray[kMaxGatewayInfoCount];
    StaticArrayList<GatewayInfo> mGatewayInfoList;
};

}

}

#endif /* MQTTSN_GATEWAY_LIST_HPP_ */
