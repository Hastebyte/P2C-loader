#pragma once
#include <string>
#include <optional>

namespace redirection
{
	std::optional<std::wstring> resolve(const std::wstring& name, const std::wstring& parent_name = L"");
}