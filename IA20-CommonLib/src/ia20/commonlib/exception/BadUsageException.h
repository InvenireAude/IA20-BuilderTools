/*
 * File: IA20-CommonLib/src/commonlib/exception/BadUsageException.h
 *
 * Copyright (C) 2021, Albert Krzymowski
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* ChangeLog:
 *
 */

#ifndef _IA20_BadUsageException_H_
#define _IA20_BadUsageException_H_

#include "Exception.h"

namespace IA20{

/*************************************************************************/
/** The BadUsageException exception class.
 */

class BadUsageException : public IA20::Exception
{
public:
	BadUsageException();
	BadUsageException(const String& strInfo);

	virtual ~BadUsageException() throw();

	virtual const char*  getName();
};

}

#endif /*BadUsageEXCEPTION_H_*/
