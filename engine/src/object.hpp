// object.hpp

#pragma once

#include <cstring>
#include <array>
#include "log.hpp"
#include "event_types.hpp"
#include "memory_allocator.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class XDataBuffer;
	class cGameObject;

	using ClassType = ::std::string;

	#define TRITON_OBJECT(typeName) \
		public: \
			static ClassType GetTypeStatic() { return #typeName; } \
			virtual ClassType GetType() const override { return GetTypeStatic(); } \

	#define TRITON_CLASS_NAME(className) \
		public: \
			static std::string GetTypeStatic() { return #className; } \
			virtual ClassType GetType() const { return GetTypeStatic(); } \
		private: \

	class cIdentifier
	{
	public:
		static cTag Generate(const ::std::string& seed);
	};

	class iObject;
	class cObjectPtr
	{
	public:
		iObject* object = nullptr;
	};

	class iObject
	{
		friend class CCreator;

	protected:
		SMemoryAllocation _mem_alloc;
		cContext* _context;

	public:

		iObject() = default; // TODO: remove this <<<<<-----

		explicit iObject(const SMemoryAllocation& mem_alloc, cContext* context)
			: _mem_alloc(mem_alloc), _context(context) {}
		
		virtual ~iObject() = default;

		iObject() = delete;
		iObject(const iObject&) = delete;
		iObject& operator=(const iObject&) = delete;
		iObject(iObject&&) = delete;
		iObject& operator=(iObject&&) = delete;

		virtual ClassType GetType() const = 0;

		void Subscribe(eEventType type, EventFunction&& function);
		void Unsubscribe(eEventType type);
		void Send(eEventType type);
		void Send(eEventType type, XDataBuffer* data);

		inline cContext* GetContext() const { return _context; }
	};
}