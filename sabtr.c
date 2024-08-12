// Author: Nurudin Imsirovic <realnurudinimsirovic@gmail.com>
// Abstract: Utility to reset trial state of StartAllBack
// License: Do whatever you want.
// Created: 2024-08-01 02:28 PM
// Updated: 2024-08-12 10:32 AM

// Steps the program takes:
// ------------------------
// 0. Make sure the current user has administrator privileges.
// 1. Open an absolute path to a registry key.
// 2. Test each subkey against a predefined set of criteria.
// 3. If a subkey was matched verify it does not contain any other
//    subkeys and values.
// 4. Try to delete it, if that fails inform the user and exit with
//    code 1.
// 5. If deletion was successful, inform the user, and exit with
//    errorlevel 0.

#include <stdio.h>
#include <windows.h>

// shlobj.h provides us a wrapper function 'IsUserAnAdmin()' that
// is simple to use (thank God).  But, MSDN says that we should
// instead use a different approach (but does not offer any
// concrete examples).  For now, this'll do.
//
// WARN: This function may lie to us, therefore a more thorough
//       approach should be taken to definitively come to a
//       conclusion on the user type running the program (admin,
//       not admin).
//
// NOTE: Must link the shell32 library.  It would be of benefit
//       to have an implementation that works without libraries
//       involved.
#include <shlobj.h>

//#define SABTR_DEBUG
#define SABTR_REG_KEY "Software\\Microsoft\\Windows\\"\
                      "CurrentVersion\\Explorer\\CLSID"

// Partial source from: MSDN - Enumerating Registry Subkeys
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

BOOL g_subkeyMatched = FALSE;
BOOL g_subkeyWasWarned = FALSE;
LPCSTR g_subkeyMatchedPath[MAX_KEY_LENGTH];
LPCSTR g_subkeyMatchedName[MAX_KEY_LENGTH];

int SABTR_ConcatSubkeysValues(HKEY hKey, LPCSTR lpSubKey) {
  HKEY hKeyHandle;

  LPCSTR concatKey[MAX_KEY_LENGTH];
  sprintf_s(concatKey, MAX_KEY_LENGTH, "%s\\%s", SABTR_REG_KEY,
    lpSubKey);

  LONG hKeyErr = RegOpenKeyEx(
    hKey,
    concatKey,
    0,
    KEY_READ,
    &hKeyHandle);

  if (hKeyErr != ERROR_SUCCESS) {
    printf("FAIL: Could not get info on the key: %s\n", concatKey);
    return -1;
  }

  DWORD i = 0, j = 0, retCode = 0;

  TCHAR    achKey[MAX_KEY_LENGTH];
  DWORD    cbName;
  TCHAR    achClass[MAX_PATH];
  DWORD    cchClassName;
  DWORD    cSubKeys;
  DWORD    cbMaxSubKey;
  DWORD    cchMaxClass;
  DWORD    cValues;
  DWORD    cchMaxValue;
  DWORD    cbMaxValueData;
  DWORD    cbSecurityDescriptor;
  FILETIME ftLastWriteTime;

  TCHAR achValue[MAX_VALUE_NAME] = {'\0'};
  DWORD cchValue = MAX_VALUE_NAME;

  retCode = RegQueryInfoKey(
    hKeyHandle,
    achClass,
    &cchClassName,
    NULL,
    &cSubKeys,
    &cbMaxSubKey,
    &cchMaxClass,
    &cValues,
    &cchMaxValue,
    &cbMaxValueData,
    &cbSecurityDescriptor,
    &ftLastWriteTime
  );

  if (retCode != ERROR_SUCCESS) {
    return -2;
  }

  return cSubKeys + cValues;
}

void SABTR_TestKeys(HKEY hKey) {
  DWORD i = 0, j = 0, retCode = 0;

  TCHAR    achKey[MAX_KEY_LENGTH];
  DWORD    cbName;
  TCHAR    achClass[MAX_PATH];
  DWORD    cchClassName;
  DWORD    cSubKeys;
  DWORD    cbMaxSubKey;
  DWORD    cchMaxClass;
  DWORD    cValues;
  DWORD    cchMaxValue;
  DWORD    cbMaxValueData;
  DWORD    cbSecurityDescriptor;
  FILETIME ftLastWriteTime;

  TCHAR achValue[MAX_VALUE_NAME] = {'\0'};
  DWORD cchValue = MAX_VALUE_NAME;

  retCode = RegQueryInfoKey(
    hKey,
    achClass,
    &cchClassName,
    NULL,
    &cSubKeys,
    &cbMaxSubKey,
    &cchMaxClass,
    &cValues,
    &cchMaxValue,
    &cbMaxValueData,
    &cbSecurityDescriptor,
    &ftLastWriteTime
  );

  if (cSubKeys == 0) {
    printf(" | FAIL: No subkeys exist at: HKCU\\%s\n", SABTR_REG_KEY);
    return;
  }

  // Enumerate the subkeys, until RegEnumKeyEx fails.
  printf(" | INFO: Found %d subkeys\n", cSubKeys);

  for (i = 0; i < cSubKeys; i++) {
    cbName = MAX_KEY_LENGTH;
    retCode = RegEnumKeyEx(hKey, i,
      achKey,
      &cbName,
      NULL,
      NULL,
      NULL,
      &ftLastWriteTime
    );

    if (retCode != ERROR_SUCCESS) {
      printf(" | WARN: Could not enumerate key: HKCU\\%s\\%s\n",
        SABTR_REG_KEY, achKey);
      continue;
    }

    #ifdef SABTR_DEBUG
    printf(" | DBGR: Subkey (%d) %s\n", i + 1, achKey);
    #endif

    int concatCount = SABTR_ConcatSubkeysValues(
      HKEY_CURRENT_USER,
      achKey
    );

    // matching key?
    if (concatCount == 0) {
      // WARN: matched count occured twice?
      if (g_subkeyMatched == TRUE && g_subkeyWasWarned == FALSE) {
        printf("\n +---------------------------------------------+\n");
        printf(" | WARN: Multiple subkeys match the criteria   |\n");
        printf(" | (first encountered subkey will be affected) |\n");
        printf(" +---------------------------------------------+\n\n");
        g_subkeyWasWarned = TRUE;
        break;
      }

      g_subkeyMatched = TRUE;

      sprintf_s(
        g_subkeyMatchedName,
        MAX_KEY_LENGTH,
        "%s",
        achKey
      );

      // Concatenate SABTR_REG_KEY with achKey
      sprintf_s(
        g_subkeyMatchedPath,
        MAX_KEY_LENGTH,
        "%s\\%s",
        SABTR_REG_KEY,
        achKey
      );
    }
  }

  return;
}

int main() {
  BOOL admin = IsUserAnAdmin();

  printf("[+] INFO: Acquiring user type information\n");

  if (admin) {
    printf(" | PASS: User is an Administrator\n\n");

    // 1. Open handle to the registry key
    HKEY hKey1 = 0;

    LONG hKey1Err = RegOpenKeyEx(
      HKEY_CURRENT_USER,
      TEXT(SABTR_REG_KEY),
      0,
      KEY_READ,
      &hKey1);

    printf("[+] INFO: Trying to open handle to key: HKCU\\%s\n",
      SABTR_REG_KEY);

    if (hKey1Err != ERROR_SUCCESS) {
      printf(" | FAIL: Failed to open handle (Code: %d)\n", hKey1Err);
      return 1;
    } else {
      printf(" | PASS: Successfully opened handle\n\n", SABTR_REG_KEY);
    }

    // 2. Query each subkey and test against predefined criteria.
    printf("[+] INFO: Querying subkeys ...\n");
    SABTR_TestKeys(hKey1);

    // We can close early
    RegCloseKey(hKey1);

    // 3. Delete the subkey. Inform the user on failure and abort.
    if (g_subkeyMatched == FALSE) {
      printf(" | FAIL: No subkey was matched.\n");
      RegCloseKey(hKey1);
      return 1;
    }

    if (g_subkeyWasWarned == TRUE) {
      printf(" | WARN: Found multiple matching subkeys!\n");
      printf(" | WARN: -- Only this subkey will be affected: %s\n", g_subkeyMatchedName);
    } else {
      printf(" | PASS: Found a matching subkey: %s\n", g_subkeyMatchedName);
    }

    printf(" | INFO: Absolute Path: %s\n", g_subkeyMatchedPath);

    #ifdef SABTR_DEBUG
    printf(" | DBGR: RegDeleteKeyA(HKEY_CURRENT_USER, \"%s\")\n", g_subkeyMatchedPath);
    #endif

    LONG deleteStatus = RegDeleteKeyA(HKEY_CURRENT_USER, g_subkeyMatchedPath);

    if (deleteStatus != ERROR_SUCCESS) {
      printf(" | FAIL: Could not delete the subkey\n");
      return 1;
    }

    printf(" | PASS: Successfully deleted the subkey\n");
    return 0;
  }

  printf(" | FAIL: Utility demands Administrator privileges.\n\n");
  return 1;
}
