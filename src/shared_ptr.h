#pragma once

#ifdef USE_TR1
#include <tr1/memory>
#define DEFINE_PTR(Type) typedef std::tr1::shared_ptr<Type> Type ## Ptr
#else
#include <memory>
#define DEFINE_PTR(Type) typedef std::shared_ptr<Type> Type ## Ptr
#endif
