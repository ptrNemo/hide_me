#pragma once
#include <ntifs.h>

struct LSAP_LIST_ENTRY64 {
    LSAP_LIST_ENTRY64* Flink;
    LSAP_LIST_ENTRY64* Blink;
};

struct RTL_CRITICAL_SECTION {
    PVOID DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    PVOID OwningThread;
    PVOID LockSemaphore;
    ULONG_PTR SpinCount;
};

struct LSAP_LOGON_SESSION_TABLE {
    UCHAR _unknown_0x00[4];
    UCHAR Flags;
    UCHAR _pad_0x05[3];
    ULONG Count;
    PVOID CachedEntry;
    LSAP_LIST_ENTRY64 Head;
    UCHAR _pad_0x28[8];
    RTL_CRITICAL_SECTION Lock;
};

struct LSAP_DS_NAME_MAP {
    LONGLONG ValidUntil;
    LONGLONG LastAccess;
    LONG RefCount;
    UCHAR Reserved_14[0x04];
    UNICODE_STRING Name;
};

struct LSAP_LOGON_SESSION {
    UCHAR Reserved_0000[0x10];
    UCHAR Lock[0x40];
    UNICODE_STRING DnsDomainName;
    UCHAR Reserved_0060[0x10];
    LUID LogonId;
    UCHAR Reserved_0078[0x18];
    UNICODE_STRING AccountName;
    UNICODE_STRING DomainName;
    UCHAR Reserved_00B0[0x20];
    PSID Sid;
    ULONG LogonType;
    ULONG Reserved_00DC;
    UCHAR Reserved_00E0[0x08];
    ULONG Session;
    ULONG Reserved_00EC;
    UCHAR Reserved_00F0[0x88];
    LSAP_DS_NAME_MAP* NameByFormat[15];
    ULONG Flags;
    UCHAR Reserved_01F4[0x40];
};

struct SRWLOCK {
    PVOID Ptr;
};

struct SEC_HANDLE_ENTRY {
    LSAP_LIST_ENTRY64 Links;
    SecHandle Handle;
    PVOID Context;
    ULONG RefCount;
    ULONG OpenCount;
    ULONG DuplicateCount;
    UCHAR Flags;
    UCHAR _pad_0x35[3];
    LUID OwnerLuid;
    SRWLOCK Lock;
};









