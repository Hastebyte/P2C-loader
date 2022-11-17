#pragma once
#include <exception>

class Driver
{
	void* device_handle;

	bool load_driver();

public:
	class DriverException : std::exception
	{
		using std::exception::exception;
	};

	struct Allocation
	{
		void* mdl_address;
		void* kernel_va;
	};

	Driver();
	~Driver();

	Driver(Driver&&) = default;
	Driver(const Driver&) = delete;
	Driver& operator=(Driver&&) = default;
	Driver& operator=(const Driver&) = delete;

	Allocation allocate(size_t size) const noexcept;
	bool free(const Allocation& allocation) const noexcept;
	bool hide(const Allocation& allocation) const noexcept;

	bool memcpy(void* source, void* destination, size_t size) const noexcept;
	bool execute(void* entry, void* argument) const noexcept;
};
