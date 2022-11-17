#pragma once
#include "dll_mapper.hpp"
#include <Windows.h>
#include <winternl.h>

class ModuleMapper
{
	DllMapper& mapper;

	std::vector<uint8_t> image_buffer;
	PIMAGE_NT_HEADERS	 nt_headers = nullptr;
	uint8_t*			 local_base = nullptr;

	template <typename T>
	T from_rva(uint32_t offset) const noexcept
	{
		return T(offset ? (image_buffer.data() + offset) : nullptr);
	}

	template <typename T>
	std::pair<T, size_t> get_data_directory(uint32_t index) const noexcept
	{
		const auto& datadir_descriptor = nt_headers->OptionalHeader.DataDirectory[index];
		if (datadir_descriptor.Size == 0)
			return std::make_pair(T(nullptr), 0);

		return std::make_pair(from_rva<T>(datadir_descriptor.VirtualAddress), datadir_descriptor.Size);
	}

	bool map_as_image(const std::vector<uint8_t>& file_buffer);
	bool relocate(uintptr_t delta);
	bool resolve_imports();

public:
	ModuleMapper(DllMapper& mapper);

	ModuleMapper(ModuleMapper&&) = delete;
	ModuleMapper(const ModuleMapper&) = delete;
	ModuleMapper& operator=(ModuleMapper&&) = delete;
	ModuleMapper& operator=(const ModuleMapper&) = delete;

	std::optional<DllMapper::Module> map_module(const wchar_t* path);
};
