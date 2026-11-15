#pragma once
#include <ntifs.h>
#include <ndis.h>

struct NDIS_IF_MINIPORT {
    ULONG CompartmentId;
    GUID NetworkGuid;
    IF_COUNTED_STRING ifAlias;
    NET_IF_OPER_STATUS ifOperStatus;
    ULONG ifOperStatusFlags;
    ULONG ifMtu;
    IF_PHYSICAL_ADDRESS ifPhysAddress;
    IF_PHYSICAL_ADDRESS PermanentPhysAddress;
    NET_IF_ADMIN_STATUS ifAdminStatus;
    ULONG64 XmitLinkSpeed;
    ULONG64 RcvLinkSpeed;
    BOOLEAN ifPromiscuousMode;
    BOOLEAN ifDeviceWakeUpEnable;
    BOOLEAN ifHideInterfaceInUi;
    NET_IF_MEDIA_CONNECT_STATE MediaConnectState;
    NET_IF_MEDIA_DUPLEX_STATE MediaDuplexState;
};

struct NDIS_IF_BLOCK;

struct NDIS_MINIPORT_BLOCK_STUB{
    UCHAR _reserved[0xFC8];
    NDIS_IF_BLOCK* IfBlock;
};

struct NDIS_IF_BLOCK {
    NDIS_OBJECT_HEADER Header;
    NET_IFINDEX ifIndex;
    IF_COUNTED_STRING ifDescr;
    USHORT ifType;
    NET_IF_ACCESS_TYPE AccessType;
    NET_IF_DIRECTION_TYPE DirectionType;
    NET_IF_CONNECTION_TYPE ConnectionType;
    GUID InterfaceGuid;
    BOOLEAN ifConnectorPresent;
    ULONG ifFlags;
    NDIS_MEDIUM MediaType;
    NDIS_PHYSICAL_MEDIUM PhysicalMediumType;
    NDIS_IF_MINIPORT MiniportInfo;
    LIST_ENTRY Link;
    LIST_ENTRY ProviderLink;
    LIST_ENTRY NetworkLink;
    PVOID ifRcvAddressTable;
    ULONG ifRcvAddressCount;
    ULONG ifMaxRcvAddressCount;
    ULONG LowerLayerIfCount;
    ULONG HigherLayerIfCount;
    LONG Ref;
    NET_LUID NetLuid;
    PVOID ProviderIfContext;
    PVOID ProviderHandle;
    ULONG Flags;
    ULONG WanTunnelType;
    ULONG PortNumber;
    ULONG64 ifLastChange;
    ULONG64 ifCounterDiscontinuityTime;
    PVOID Network;
    PVOID Compartment;
    PKEVENT AsyncEvent;
    BOOLEAN AlreadyDeregistered;
    BOOLEAN bNdisIsProvider;
    BOOLEAN IsNdisFilter;
    BOOLEAN MiniportAvailable;
    ULONG MiniportLinkReference;
    ULONG Source;
    union {
        NDIS_MINIPORT_BLOCK_STUB* Miniport;
        PVOID Filter;
    };
    ULONG SupportedStatistics;
    PIF_COUNTED_STRING ifL2NetworkInfo;
    PVOID RefCountTracker;
    PVOID MpRefCountTracker;
    ULONG InterlockedFlags;
    USHORT LastMiniportFatalErrorReason;
    ULONG64 MiniportFatalErrorTimestamp[3];
    UCHAR MiniportFatalErrorTimestampIndex;
    BOOLEAN NetworkIdOverridden;
    BOOLEAN IsWDFMiniportInterface;
    ULONG ifPurpose;
    PVOID PcwDataBlock;
};