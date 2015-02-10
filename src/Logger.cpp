#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Logger.h"

void RZLogger::ERR(const char* fmt, ...)		//�������ó�����ӡ��Ϣ���˳�
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
#ifdef	WIN32
	system("pause>nul");		//��������˳�
#endif
	exit(EXIT_FAILURE);
}