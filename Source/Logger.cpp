#include "Globals.h"
#include "Logger.h"

void Logger::AddLog(const char* msg, ...)  IM_FMTARGS(2)
{
	_console.AddLog(msg);
}
