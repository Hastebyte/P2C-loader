#pragma once
#include <cstdint>
#include <optional>

class Process
{
	void* process_handle;

public:
	static std::optional<Process> find(const wchar_t* process_name);

	Process(void* process_handle);
	Process(Process&&) noexcept;
	Process(const Process&) = delete;
	Process& operator=(Process&&) noexcept;
	Process& operator=(const Process&) = delete;

	~Process();

	bool read_buffer(uintptr_t address, void* buffer, size_t size);
	bool write_buffer(uintptr_t address, const void* buffer, size_t size);

	uintptr_t get_peb_address();
	uintptr_t allocate(size_t size, bool execute);
	bool      free(uintptr_t address);
	bool	  execute(uintptr_t address, uintptr_t parameter = 0);

	template <typename T>
	T read(uintptr_t address)
	{
		T data{};
		read_buffer(address, &data, sizeof(T));

		return data;
	}

	template <typename T>
	void write(uintptr_t address, T data)
	{
		write_buffer(address, &data, sizeof(T));
	}
};

