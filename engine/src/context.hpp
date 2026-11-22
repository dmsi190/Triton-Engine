// context.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "engine.hpp"
#include "capabilities.hpp"
#include "types.hpp"

namespace triton
{
	class IBackend;
	class XSubsystem;

	class cContext
	{
		std::unordered_map<ClassType, IBackend*> _backends;
		std::unordered_map<ClassType, XSubsystem*> _subsystems;

	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T>
		void RegisterBackend(T* backend);

		void RegisterSubsystem(XSubsystem* subsystem)
		{
			// TODO: static_assert that T must inherit from iObject
			ClassType type = subsystem->GetType();
			const auto it = _subsystems.find(type);
			if (it == _subsystems.end())
				_subsystems.insert({ type, subsystem });
		}

		template <typename T>
		void ReleaseSubsystem();

		template <typename T>
		inline T* GetBackend() const;

		template <typename T>
		inline T* GetSubsystem() const;
	};
}

template <typename T>
void triton::cContext::RegisterBackend(T* backend)
{
	// TODO: static_assert that T must inherit from iBackend
	const ClassType type = T::GetTypeStatic();
	const auto it = _backends.find(type);
	if (it == _backends.end())
		_backends.insert({ type, backend });
}

template <typename T>
void triton::cContext::ReleaseSubsystem()
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _subsystems.find(type);
	if (it == _subsystems.end())
		delete it->second;
}

template <typename T>
T* triton::cContext::GetBackend() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _backends.find(type);
	if (it != _backends.end())
		return (T*)it->second;
	else
		return nullptr;
}

template <typename T>
T* triton::cContext::GetSubsystem() const
{
	const ClassType type = T::GetTypeStatic();
	const auto it = _subsystems.find(type);
	if (it != _subsystems.end())
		return (T*)it->second;
	else
		return nullptr;
}