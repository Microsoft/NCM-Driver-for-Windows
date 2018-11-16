// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

class NcmRxQueue
{

#pragma region NETADAPTER event call backs

public:

    PAGED
    static 
    NTSTATUS 
    EvtCreateRxQueue(
        _In_ NETADAPTER netAdapter,
        _Inout_ NETRXQUEUE_INIT * netRxQueueInit);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    inline
    static
    void
    EvtSetNotificationEnabled(
        _In_ NETPACKETQUEUE netQueue,
        _In_ BOOLEAN notificationEnabled)
    {
        Get(netQueue)->SetNotificationEnabled(notificationEnabled);
    }

    PAGED
    inline
    static
    void
    EvtCancel(_In_ NETPACKETQUEUE netQueue)
    {
        Get(netQueue)->Cancel();
    }

    _IRQL_requires_max_(DISPATCH_LEVEL)
    inline
    static
    void
    EvtAdvance(_In_ NETPACKETQUEUE netQueue)
    {
        Get(netQueue)->Advance();
    }

    PAGED
    inline
    static
    void
    EvtStart(_In_ NETPACKETQUEUE netQueue)
    {
        Get(netQueue)->Start();
    }

#pragma endregion

private:

    _IRQL_requires_max_(DISPATCH_LEVEL)
    static
    NcmRxQueue* Get(_In_ NETPACKETQUEUE queue);

    PAGED
    static
    void
    EvtDestroyRxQueue(_In_ WDFOBJECT object);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    inline
    void
    SetNotificationEnabled(_In_ BOOLEAN notificationEnabled)
    {
        InterlockedExchange(&m_NotificationEnabled, notificationEnabled);
    }

    _IRQL_requires_max_(DISPATCH_LEVEL)
    inline
    void
    NotifyReceive()
    {
        if (InterlockedExchange(&m_NotificationEnabled, FALSE) == TRUE)
        {
            NetRxQueueNotifyMoreReceivedPacketsAvailable(m_Queue);
        }
    }

    PAGED
    NcmRxQueue(_In_ NcmAdapter* ncmAdapter,
               _In_ NETPACKETQUEUE queue) :
        m_NcmAdapter(ncmAdapter),
        m_Queue(queue)
    {
        m_Rings = NetRxQueueGetRingCollection(m_Queue);
    }

    PAGED
    NTSTATUS
    InitializeQueue();

    _IRQL_requires_max_(DISPATCH_LEVEL)
    void Advance();

    PAGED
    void Cancel();

    PAGED
    void Start();

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS GetNextFrame(
        _Outptr_result_buffer_(frameSize)  PUCHAR* frame,
        _Out_ size_t* frameSize);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    bool MatchPacketFilter(
        _In_reads_bytes_(frameSize) UCHAR const* frame,
        _In_ size_t frameSize);

private:

    NET_RING_COLLECTION const * m_Rings = nullptr;
    LONG                        m_NotificationEnabled = 0;
    NcmAdapter*                 m_NcmAdapter = nullptr;
    NETPACKETQUEUE              m_Queue = nullptr;
    NTB_HANDLE                  m_NtbHandle = nullptr;
    RX_BUFFER*                  m_RxBufferInProcessing = nullptr;
    RX_BUFFER_QUEUE             m_RxBufferQueue = nullptr;

    friend NcmAdapter;
};

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(NcmRxQueue, NcmGetRxQueueFromHandle);

