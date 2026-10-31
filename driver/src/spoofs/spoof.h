#pragma once

#include <ntifs.h>
#include <cstdint>
#include "utils.h"
#include "structures/lsa.h"

#define LSASRV_DLL L"lsasrv.dll"

extern "C" NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);

static const WCHAR CHARSET[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

/**
 * @brief Finds Smbios table address using an aob pattern.
 * Creates a fake smbios table and assigns it to the original global variable
 * that holds the pointer to the original table.
 *
 * @code
 * bytes: \x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8B\x15
 * mask: xxx????xxxx?xx
 * @endcode
 */
void smbios();

/**
 * @breif Finds the lsap logon table using an aob pattern.
 * Iterate over the table sessions and checks what are the correct user session,
 * once found, it modifies the buffer of the cached username.
 *
 * @code
 * bytes: \x48\x8B\x0D\x00\x00\x00\x00\x8B\xC3\x48\x89\x85\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x89\xBD
 * mask: xxx????xxxxx????xxx????xxx
 * @endcode
 */
void username();

/**
 * @brief Finds the VPB structure into the C:\ volume opening it and it generates a new volume serial.
 */
void volumes();

/**
 * @biref Searches the lsap logon session table address using an aob scan into the lsasrv.dll module loaded into the
 * lsass.exe memory.
 *
 * @param process Pointer to lsass.exe process.
 *
 * @return Pointer to LSAP_LOGON_SESSION_TABLE
 */
LSAP_LOGON_SESSION_TABLE* findLsapLogonSessionTable(PEPROCESS process);

/**
 * @brief Iterates over every Session stored into the lsap logon session table using the table head as start point.
 *
 * @param table Pointer to LSAP_LOGON_SESSION_TABLE.
 */
void loopLsapSessions(LSAP_LOGON_SESSION_TABLE* table);

/**
 * @brief Checks if the entry has a valid address.
 *
 * @warning Checking only if the pointer points to something doesn't mean it's a valid session, in future it's needed
 * to be implemented some other control over this object.
 *
 * @param entry Entry pointer.
 * @return True if it's a valid session, false if it's not.
 */
bool isValidEntry(SEC_HANDLE_ENTRY* entry);

/**
 * @brief Checks if the session has a valid address.
 *
 * @warning Checking only if the pointer points to something doesn't mean it's a valid session, in future it's needed
 * to be implemented some other control over this object.
 *
 * @param session Session pointer.
 *
 * @return True if it's a valid session, false if it's not.
 */
bool isValidSession(LSAP_LOGON_SESSION* session);

/**
 * @brief Checks if the session sid it's a valid user or a system session using some checks on the sid data.
 * Checks even if the sid is a valid sid using 'RtlValidSid()' and if the passed sid it's not null.
 * @code
 * if (subAuthCount == 5 && subAuthTZero == 21 && subAuthTFour >= 1000)
 * @endcode
 *
 * @param sid Pointer to session sid.
 *
 * @return True if the session is an actual user, false if it's more likely some system session.
 */
bool isInteractiveUserSession(PSID sid);

/**
 * @brief Copies the PSID data from the lsass.exe process and maps it into the kernel memory.
 *
 * @param sourceSdi Pointer to original sid.
 * @param outBuff A UCHAR pointer that is used as output buffer, here is where the data is copied.
 *
 * @return True if successfully copied, false if not.
 */
bool copySidFromLsassProcess(PSID sourceSdi, UCHAR* outBuff);

/**
 * @brief Opens the root directory of the C: volume.
 *
 * The open goes through the file system, so the FILE_OBJECT associated with
 * the handle has its Vpb field set.
 *
 * @param handle Receives the kernel handle to the root directory.
 * The caller must close it with ZwClose.
 *
 */
void openVolumeHandle(PHANDLE handle);

/**
 * @brief Gets the FILE_OBJECT from a handle and reads its pointer to the VPB.
 *
 * @param handle Handle obtained from openVolumeHandle.
 * @param fileObject Receives the referenced FILE_OBJECT. If it is not NULL,
 * the caller must release it with ObDereferenceObject,
 * even when the function returns STATUS_NOT_FOUND.
 * @param vpb Receives a pointer to the original VPB, not a copy.
 * Use it only while fileObject remains referenced.
 *
 */
void getVpb(HANDLE handle, PFILE_OBJECT *fileObject, PVPB *vpb);

/**
 * @brief Generates a new random serial and it changes the one into the VPB till the reboot.
 *
 * @param vpb Pointer to the original VPB.
 */
void spoofVolumeSerial(PVPB vpb);