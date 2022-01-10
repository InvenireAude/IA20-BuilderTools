/*
 * File: Listener.cpp
 *
 * Copyright (C) 2021, Albert Krzymowski
 *
 */

#include "Listener.h"


#include <ia20/iot/logger/LogLevel.h>

namespace IA20 {
namespace IOT {

/*************************************************************************/
Listener::Listener(std::unique_ptr<RingType::Interface>&& ptrInterface,
				Memory::SharableMemoryPool* pMemoryPool):
ptrInterface(std::move(ptrInterface)),
pMemoryPool(pMemoryPool){
	IA20_TRACER;
}
/*************************************************************************/
Listener::~Listener() throw(){
	IA20_TRACER;
}
/*************************************************************************/
}
}
