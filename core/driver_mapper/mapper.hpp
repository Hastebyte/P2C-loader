#pragma once
#define  WIN32_LEAN_AND_MEAN
#define  WIN32_NO_STATUS
#define  NOMINMAX
#include <Windows.h>
#include <winternl.h>
#undef	 WIN32_LEAN_AND_MEAN
#undef	 WIN32_NO_STATUS
#undef	 NOMINMAX

#include "module_manager.hpp"
#include <cstdint>
#include <memory>

class Mapper
{
public:
	class InvalidHeaderException : public std::exception
	{
		using std::exception::exception;
	};

private:
	std::unique_ptr<uint8_t[]> image;
	PIMAGE_NT_HEADERS nt_headers;

	template <typename T>
	T from_rva(const uint32_t offset) const noexcept
	{
		return T(offset ? (image.get() + offset) : nullptr);
	}

	template <typename T>
	std::pair<T, size_t> get_data_directory(const uint32_t index) const noexcept
	{
		const auto& datadir_descriptor = nt_headers->OptionalHeader.DataDirectory[index];
		if (datadir_descriptor.Size == 0)
			return std::make_pair(T(nullptr), 0);

		return std::make_pair(from_rva<T>(datadir_descriptor.VirtualAddress), datadir_descriptor.Size);
	}

public:
	explicit Mapper(const uint8_t* file_buffer);
	~Mapper() = default;

	Mapper(Mapper&&) = default;
	Mapper(const Mapper&) = delete;
	Mapper& operator=(Mapper&&) = default;
	Mapper& operator=(const Mapper&) = delete;

	bool relocate_image(uintptr_t new_base) const noexcept;
	bool resolve_imports(ModuleManager& module_manager) const noexcept;
    //void erase_headers() const noexcept; // dont use replaced with strip_image
    void strip_image() const noexcept;

	std::pair<void*, size_t> get_image() const noexcept;
	void* get_entrypoint(uintptr_t base_address) const noexcept;
};
