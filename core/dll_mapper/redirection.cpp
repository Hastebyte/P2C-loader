#include "redirection.hpp"
#include "apiset_structs.hpp"
#include "nt_structs.hpp"
#include <Windows.h>
#include <winternl.h>

extern "C" LONG NTAPI RtlCompareUnicodeStrings(
	PWCH String1,
	SIZE_T String1Length,
	PWCH String2,
	SIZE_T String2Length,
	BOOLEAN CaseInSensitive
);

static PAPI_SET_NAMESPACE_ENTRY SearchForApiSet(PAPI_SET_NAMESPACE ApiSet, PCWSTR Name, USHORT NameLength)
{
	if (NameLength == 0 || ApiSet->Count < 2)
		return nullptr;

	PCWSTR Iterator = Name;
	ULONG  CalculatedHash = 0;
	for (ULONG Counter = NameLength; Counter != 0; ++Iterator, --Counter)
	{
		wchar_t Character = (USHORT)(*Iterator - 'A') > 25 ? (*Iterator) : (*Iterator + ' ');
		CalculatedHash = Character + ApiSet->HashFactor * CalculatedHash;
	}

	ULONG				Counter = 0;
	ULONG				ApiSetCount = ApiSet->Count - 1;
	PAPI_SET_HASH_ENTRY HashEntry = nullptr;
	while (true)
	{
		ULONG HashIndex = (ApiSetCount + Counter) / ULONG(sizeof(wchar_t));
		ULONG HashEntryOffset = ApiSet->HashOffset + (HashIndex * sizeof(API_SET_HASH_ENTRY));

		HashEntry = (PAPI_SET_HASH_ENTRY)((ULONG_PTR)ApiSet + HashEntryOffset);
		if (CalculatedHash < HashEntry->Hash)
		{
			ApiSetCount = HashIndex - 1;
			if (Counter > ApiSetCount)
				return nullptr;

			continue;
		}

		if (CalculatedHash == HashEntry->Hash)
			break;

		Counter = HashIndex + 1;
		if (Counter > ApiSetCount)
			return nullptr;
	}

	if (HashEntry == nullptr)
		return nullptr;

	PAPI_SET_NAMESPACE_ENTRY FoundEntry = (PAPI_SET_NAMESPACE_ENTRY)(
		(ULONG_PTR)ApiSet + ApiSet->EntryOffset + HashEntry->Index * sizeof(API_SET_NAMESPACE_ENTRY)
	);

	if (FoundEntry != nullptr)
	{
		PWSTR  EntryName = (PWSTR)((ULONG_PTR)ApiSet + FoundEntry->NameOffset);
		SIZE_T EntryLength = FoundEntry->HashedLength / sizeof(wchar_t);

		if (!RtlCompareUnicodeStrings((PWCH)Name, NameLength, EntryName, EntryLength, true))
			return FoundEntry;
	}

	return nullptr;
}

static PAPI_SET_VALUE_ENTRY SearchForApiSetHost(PAPI_SET_NAMESPACE ApiSet, PAPI_SET_NAMESPACE_ENTRY Entry, PWCHAR ParentName, SHORT ParentNameLength)
{
	ULONG EntryValueOffset = Entry->ValueOffset;
	ULONG EntryAliasCount = Entry->ValueCount - 1;
	PAPI_SET_VALUE_ENTRY FoundEntry = (PAPI_SET_VALUE_ENTRY)((ULONG_PTR)ApiSet + EntryValueOffset);

	if (EntryAliasCount == 0)
		return FoundEntry;

	ULONG Counter = 1;
	do
	{
		ULONG EntryAliasIndex = (EntryAliasCount + Counter) / ULONG(sizeof(wchar_t));
		PAPI_SET_VALUE_ENTRY AliasEntry = (PAPI_SET_VALUE_ENTRY)(
			(ULONG_PTR)ApiSet + EntryValueOffset + EntryAliasIndex * sizeof(API_SET_VALUE_ENTRY)
		);

		PWSTR  EntryName = (PWSTR)((ULONG_PTR)ApiSet + AliasEntry->NameOffset);
		SIZE_T EntryLength = AliasEntry->NameLength / sizeof(wchar_t);

		LONG CompareResult = RtlCompareUnicodeStrings(
			ParentName,
			ParentNameLength,
			EntryName,
			EntryLength,
			true);

		if (CompareResult < 0)
		{
			EntryAliasCount = EntryAliasIndex - 1;
		}
		else
		{
			if (CompareResult == 0)
			{
				return (PAPI_SET_VALUE_ENTRY)(
					(ULONG_PTR)ApiSet + EntryValueOffset +
					(ULONG_PTR(EntryAliasCount + Counter) / sizeof(wchar_t)) * sizeof(API_SET_VALUE_ENTRY)
				);
			}

			Counter = EntryAliasIndex + 1;
		}
	} while (Counter <= EntryAliasCount);

	return FoundEntry;
}

static BOOLEAN ResolveToHost(PAPI_SET_NAMESPACE ApiSet, PUNICODE_STRING Name, PUNICODE_STRING ParentName, PUNICODE_STRING ResolvedName)
{
	if (Name->Length < 8)
	{
		ResolvedName->Length = Name->Length;
		ResolvedName->MaximumLength = Name->MaximumLength;
		ResolvedName->Buffer = Name->Buffer;

		return true;
	}

	ULONG64 Masked = *(ULONG64*)Name->Buffer & 0xffffffdfffdfffdf;
	if (Masked != 0x2d004900500041 && Masked != 0x2d005400580045)
	{
		ResolvedName->Length = Name->Length;
		ResolvedName->MaximumLength = Name->MaximumLength;
		ResolvedName->Buffer = Name->Buffer;

		return true;
	}

	ULONG Counter = Name->Length;
	PWSTR Iterator = Name->Buffer + (Name->Length / sizeof(wchar_t));
	do
	{
		if (Counter <= 1)
			break;

		Counter -= sizeof(wchar_t);
		Iterator -= 1;
	} while (*Iterator != '-');

	USHORT NoExtensionLength = (USHORT)Counter / sizeof(wchar_t);
	if (NoExtensionLength > 0)
	{
		PAPI_SET_NAMESPACE_ENTRY NamespaceEntry = SearchForApiSet(ApiSet, Name->Buffer, NoExtensionLength);
		if (NamespaceEntry == nullptr)
			return false;

		PAPI_SET_VALUE_ENTRY HostLibrary = nullptr;
		if (ParentName != nullptr && NamespaceEntry->ValueCount > 1)
		{
			HostLibrary = SearchForApiSetHost(
				ApiSet,
				NamespaceEntry,
				ParentName->Buffer,
				ParentName->Length / sizeof(wchar_t)
			);
		}
		else
		{
			HostLibrary = (PAPI_SET_VALUE_ENTRY)((ULONG_PTR)ApiSet + NamespaceEntry->ValueOffset);
		}

		if (HostLibrary == nullptr)
			return false;

		ResolvedName->Buffer = (PWSTR)((ULONG_PTR)ApiSet + HostLibrary->ValueOffset);
		ResolvedName->MaximumLength = (USHORT)HostLibrary->ValueLength;
		ResolvedName->Length = (USHORT)HostLibrary->ValueLength;

		return true;
	}

	return false;
}

std::optional<std::wstring> redirection::resolve(const std::wstring& name, const std::wstring& parent_name)
{
	UNICODE_STRING name_ustr;
	UNICODE_STRING parent_name_ustr;
	UNICODE_STRING resolved_name_ustr;

	RtlInitUnicodeString(&name_ustr, name.c_str());
	RtlInitUnicodeString(&parent_name_ustr, parent_name.c_str());

	const auto api_set = *(PAPI_SET_NAMESPACE*)((ULONG_PTR)__readgsqword(0x60) + 0x68);
	if (ResolveToHost(api_set, &name_ustr, parent_name.empty() ? nullptr : &parent_name_ustr, &resolved_name_ustr))
		return std::make_optional(std::wstring(resolved_name_ustr.Buffer, resolved_name_ustr.Buffer + resolved_name_ustr.Length / sizeof(wchar_t)));

	return std::nullopt;
}
