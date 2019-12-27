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

#include <mqttsn/mqttsn_gateway_list.hpp>
#include "common/timer.hpp"

namespace ot {

namespace Mqttsn {

template <typename ItemType>
otError StaticArrayList<ItemType>::Add(const ItemType &aValue)
{
    otError error = OT_ERROR_NO_BUFS;
    if (mItems == NULL || mMaxSize == 0 || mSize >= mMaxSize)
    {
        return error;
    }
    for (uint16_t i = 0; i < mMaxSize; i++)
    {
        // Find free space in the buffer for new item
        if (mItems[i].mIsRemoved)
        {
            mItems[i] = StaticListItem<ItemType>(aValue);
            mItems[i].mNext = mHead;
            mHead = &mItems[i];
            mSize++;
            error = OT_ERROR_NONE;
            break;
        }
    }
    return error;
}

template <typename ItemType>
otError StaticArrayList<ItemType>::Remove(StaticListItem<ItemType> *aItem)
{
    if (mItems == NULL || IsEmpty())
    {
        return OT_ERROR_NOT_FOUND;
    }
    if (aItem == mHead)
    {
        mHead = aItem->Next();
        aItem->mIsRemoved = true;
        mSize--;
        return OT_ERROR_NONE;
    }
    StaticListItem<ItemType> *previousItem = mHead;
    while (previousItem->HasNext())
    {
        if (previousItem->Next() == aItem)
        {
            previousItem->mNext = aItem->Next();
            aItem->mIsRemoved = true;
            mSize--;
            return OT_ERROR_NONE;
        }
        previousItem = previousItem->Next();
    }
    return OT_ERROR_NOT_FOUND;
}

template <typename ItemType>
void StaticArrayList<ItemType>::Clear()
{
    if (mItems == NULL)
    {
        return;
    }
    for (uint16_t i = 0; i < mMaxSize; i++)
    {
        mItems[i].mIsRemoved = true;
        mItems[i].mNext = NULL;
    }
    mHead = NULL;
    mSize = 0;
}

otError ActiveGatewayList::Add(GatewayId aGatewayId, const ot::Ip6::Address &aGatewayAddress, uint32_t aDuration)
{
    otError error = OT_ERROR_NONE;
    uint32_t millisNow = TimerMilli::GetNow().GetValue();

    GatewayInfo *gatewayInfo = Find(aGatewayId);
    if (gatewayInfo != NULL)
    {
        // If gateway exists in the list just update information
        gatewayInfo->mGatewayAddress = aGatewayAddress;
        gatewayInfo->mLastUpdatedTimestamp = millisNow;
        gatewayInfo->mDuration = aDuration;
    }
    else
    {
        GatewayInfo newGatewayInfo = GatewayInfo(aGatewayId, aGatewayAddress, millisNow, aDuration);
        SuccessOrExit(error = mGatewayInfoList.Add(newGatewayInfo));
    }

exit:
    return error;
}

bool ActiveGatewayList::IsEmpty() const
{
    return mGatewayInfoList.IsEmpty();
}

void ActiveGatewayList::Clear()
{
    mGatewayInfoList.Clear();
}

otError ActiveGatewayList::HandleTimer()
{
	otError error = OT_ERROR_NONE;
	StaticListItem<GatewayInfo> *item = NULL;
	uint32_t millisNow;
    if (mGatewayInfoList.IsEmpty())
    {
        ExitNow(error = OT_ERROR_NONE);
    }
	millisNow = TimerMilli::GetNow().GetValue();
    item = mGatewayInfoList.Head();
    // Find all expired gateways in the list and remove them
    do
    {
        StaticListItem<GatewayInfo> *currentItem = item;
        item = currentItem->Next();
        GatewayInfo &info = currentItem->Value();
        if (millisNow > info.mLastUpdatedTimestamp + info.mDuration)
        {
            SuccessOrExit(error = mGatewayInfoList.Remove(currentItem));
        }
    }
    while (item != NULL);
exit:
	return error;
}

GatewayInfo *ActiveGatewayList::Find(GatewayId aGatewayId)
{
    StaticListItem<GatewayInfo> *item = mGatewayInfoList.Head();
    if (item != NULL)
    {
        do
        {
            if (item->Value().GetGatewayId() == aGatewayId)
            {
                return &item->Value();
            }
            item = item->Next();
        } while (item != NULL);
    }
    return NULL;
}

const StaticArrayList<GatewayInfo> &ActiveGatewayList::GetList()
{
    return mGatewayInfoList;
}

}

}
