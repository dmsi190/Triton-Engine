// object.cpp
/*
#include "object.hpp"
#include "event_manager.hpp"
#include "context.hpp"

using namespace types;

void triton::iObject::Subscribe(eEventType type, EventFunction&& function)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Subscribe(this, type, std::move(function));
}

void triton::iObject::Unsubscribe(eEventType type)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Unsubscribe(this, type);
}

void triton::iObject::Send(eEventType type)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Send(type);
}

void triton::iObject::Send(eEventType type, XDataBuffer* data)
{
    cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
    dispatcher->Send(type, data);
}*/