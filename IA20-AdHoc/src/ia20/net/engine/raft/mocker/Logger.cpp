/*
 * File: Logger.cpp
 *
 * Copyright (C) 2020, Invenire Aude, Albert Krzymowski
 *
 */

#include "Logger.h"
#include <ia20/net/conn/Address.h>

#include "../LogEntry.h"

namespace IA20 {
namespace Net {
namespace Engine {
namespace Raft {
namespace Mocker {

/*************************************************************************/
Logger::Logger(size_t iMemorySize):
    iMemorySize(iMemorySize),
    pLastCommit(NULL),
    pLastEntry(NULL),
    pMemory(malloc(iMemorySize)){
  IA20_TRACER;

  if(!pMemory)
    IA20_THROW(SystemException("Cannot allocate mocker memory: ")<<iMemorySize<<" bytes.");

  pNextEntry = pMemory;
  iSpaceLeft = iMemorySize;
}
/*************************************************************************/
Logger::~Logger() throw(){
	IA20_TRACER;

  if(pMemory)
    free(pMemory);
}
/*************************************************************************/
const LogEntry* Logger::appendEntry(const LogEntryId& entryId,
                                    LogEntrySizeType  iEntryDataSize ,
                                    const void* pSrcData){


  IA20_LOG(LogLevel::INSTANCE.isInfo(), "Raft :: appendEntry: "<<LogEntry::ComputeSpace(iEntryDataSize)<<" ? "<<iSpaceLeft);

  if(LogEntry::ComputeSpace(iEntryDataSize) > iSpaceLeft)
    IA20_THROW(InternalException("LogEntry::ComputeSpace(iEntryDataSize) > iSpaceLeft"));

  LogEntry* pEntry = new (pNextEntry) LogEntry(entryId, pLastEntry, iEntryDataSize, pSrcData);
  pNextEntry = pEntry->next();

  iSpaceLeft -= (reinterpret_cast<uint8_t*>(pNextEntry) - reinterpret_cast<uint8_t*>(pEntry));

  IA20_LOG(LogLevel::INSTANCE.isInfo(), "Raft :: Mocker :: appendEntry: iSpaceLeft: "<<iSpaceLeft);

  pLastEntry = pEntry;
  return pEntry;

}
/*************************************************************************/
void Logger::commit(const LogEntry* pLogEntry){
  IA20_TRACER;

  if(pLastCommit  && pLogEntry < pLastCommit)
    IA20_THROW(InternalException("Mocker::Logger::commit :: pLogEntry < pLastCommit"));

  IA20_LOG(LogLevel::INSTANCE.isInfo(), "Raft :: Mocker :: commit: "<<pLogEntry->getEntryId());

  pLastCommit = pLogEntry;
}
/*************************************************************************/
void Logger::simpleDump(std::ostream& os){
  IA20_TRACER;

  const LogEntry* pCursor = reinterpret_cast<const LogEntry*>(pMemory);

  while(pCursor < pNextEntry){
    os<<pCursor->getEntryId();
    if(pCursor->getEntryDataSize()){
      os<<"{"<<String((const char*)pCursor->getData(),pCursor->getEntryDataSize())<<"}";
    }
    if(pCursor == pLastCommit)
    os<<"*";

    pCursor = pCursor->next();
  }
}
/*************************************************************************/
}
}
}
}
}
