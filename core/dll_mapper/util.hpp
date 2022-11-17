#pragma once
#include <vector>
#include <fstream>
#include <Windows.h>
#include <random>

#define log_debug(...) do { printf("[DBG]: "); printf(__VA_ARGS__); printf("\n"); } while(0)
#define log_error(...) do { printf("[ERR]: "); printf(__VA_ARGS__); printf("\n"); } while(0)
#define log_info(...) do { printf("[ERR]: "); printf(__VA_ARGS__); printf("\n"); } while(0)

namespace util
{
	template <typename T>
	inline T random_value(T min, T max)
	{
		static std::random_device device;
		std::uniform_int_distribution<T> dist(min, max);

		return dist(device);
	}

	inline std::vector<uint8_t> read_file(const wchar_t* file_path)
	{
		std::ifstream stream(file_path, std::ios::in | std::ios::binary);

		stream.unsetf(std::ios::skipws);
		stream.seekg(0, std::ios::end);
		const auto length = stream.tellg();
		stream.seekg(0, std::ios::beg);

		if (length == -1)
			return {};

		std::vector<uint8_t> file_buffer(length);
		stream.read((char*)file_buffer.data(), length);

		return file_buffer;
	}

	inline PIMAGE_NT_HEADERS get_nt_headers(const uint8_t* buffer)
	{
		if (!buffer)
			return nullptr;

		const auto dos_header = PIMAGE_DOS_HEADER(buffer);
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return nullptr;

		const auto nt_headers = PIMAGE_NT_HEADERS(buffer + dos_header->e_lfanew);
		if (nt_headers->Signature != IMAGE_NT_SIGNATURE ||
			nt_headers->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
			return nullptr;

		constexpr auto current_machine = sizeof(void*) == 8 ?
			IMAGE_FILE_MACHINE_AMD64 : IMAGE_FILE_MACHINE_I386;
		if (nt_headers->FileHeader.Machine != current_machine)
			return nullptr;

		return nt_headers;
	}
}