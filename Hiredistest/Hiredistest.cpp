#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <winsock.h>
#include <hiredis\hiredis.h>

class CConnectionInformation
{

public:
	CConnectionInformation()
		: Port(0)
	{
		memset(this->Host, 0, sizeof(this->Host));
		memset(this->AuthenticationPassword, 0, sizeof(this->Host));
	}
	virtual ~CConnectionInformation()
	{}

	char Host[129];
	unsigned long Port;
	char AuthenticationPassword[129];
};

CConnectionInformation g_RedisConnectionRTLogServer;

// No synchronization needed - this will called in the Thread reading the Log Queue which is already synchronized
class CRTLogtoREDIS
{

public:

	CRTLogtoREDIS()
		:m_redisContext(nullptr)
	{

	}

	virtual ~CRTLogtoREDIS()
	{
		this->Disconnect();
	}

	bool Log(LPCSTR LogText)
	{
		bool l_Result(false);

		try
		{
			redisReply* l_redisReply = nullptr;

			l_redisReply = this->PublishLogstream(LogText);

			if (l_redisReply)
			{

				printf_s("%s: %s    type: %ld    integer: %lld     str: %s\r\n\r\n", __FUNCTION__, LogText, l_redisReply->type, l_redisReply->integer, l_redisReply->str);

				l_Result = ((l_redisReply->type == REDIS_REPLY_INTEGER) /*&& (l_redisReply->integer == 2)*/);

				freeReplyObject(l_redisReply);
			}

		}
		catch (...)
		{
			printf_s("try...catch %s", __FUNCTION__);
		}

		if (!l_Result)
		{
			this->Disconnect();
			printf_s("Error while %s: [%s]", __FUNCTION__, LogText);
		}

		return (l_Result);
	}

private:

	redisContext* m_redisContext;

	bool Connect()
	{
		bool l_Result(false);

		try
		{
			if (!this->m_redisContext)
			{
				timeval timeout = { 0, 500000 }; // 0.5 seconds
				this->m_redisContext = redisConnectWithTimeout(g_RedisConnectionRTLogServer.Host, g_RedisConnectionRTLogServer.Port, timeout);

				if ((this->m_redisContext != nullptr) && (this->m_redisContext->err == REDIS_OK))
				{
					redisEnableKeepAlive(this->m_redisContext);

					if (!this->Authenticate())
						this->Disconnect();

				}
			}

			if ((this->m_redisContext != nullptr) && (this->m_redisContext->err == REDIS_OK))
				l_Result = true;
			else
				this->Disconnect();

		}
		catch (...)
		{
			printf_s("try...catch %s", __FUNCTION__);
		}

		return (l_Result);
	}

	void Disconnect()
	{
		try
		{
			if (this->m_redisContext != nullptr)
				redisFree(this->m_redisContext);

			this->m_redisContext = nullptr;
		}
		catch (...)
		{
			printf_s("try...catch %s", __FUNCTION__);
		}

	}

	bool Authenticate()
	{
		bool l_result = false;

		try
		{
			//redisReply* l_redisReply = nullptr;

			//const char l_AuthenticationCommand[] = { "auth" };

			//static char l_commandtext[_countof(l_AuthenticationCommand) + _countof(g_RedisConnectionRTLogServer.AuthenticationPassword) + (2 * sizeof(char))];

			//sprintf_s(l_commandtext, _countof(l_commandtext), "%s %s"
			//	, l_AuthenticationCommand
			//	, g_RedisConnectionRTLogServer.AuthenticationPassword
			//	);

			//l_redisReply = this->Command(l_commandtext);

			//if (l_redisReply)
			//{
			//	if (l_redisReply->type == REDIS_REPLY_STATUS)  // if not the there was a problem
			//	{
			//		l_result = (l_redisReply->len == 2); /* "OK" */
			//	}

			//	freeReplyObject(l_redisReply);
			//}

			//if (!l_result) this->Disconnect();

			l_result = true;

		}
		catch (...)
		{
			printf_s("try...catch %s", __FUNCTION__);
		}

		return l_result;
	}

	//redisReply* Command(char* CommandText)
	//{
	//	redisReply* l_redisReply = nullptr;

	//	// Try four times if either the conection failover mechanism can't get a hold of a redis server or if the command returns empty
	//	for (unsigned int i = 0; ((i < 4) && (l_redisReply == nullptr)); ++i)
	//	{
	//		if (this->Connect())
	//		{
	//			//RedisDebugLog
	//			//pLogger->Log(ELEVEL_1
	//			//	, "Thread[%10I64u] RedisDebugLog: %s { "
	//			//	" CommandText \"%s\""
	//			//	" PoolObjectID %llu"
	//			//	" } "
	//			//	, GetCurrentThreadId() * 1000 + ApplicationID
	//			//	, __FUNCTION__
	//			//	, CommandText
	//			//	, this->PoolObjectID
	//			//	);

	//			l_redisReply = (redisReply*)redisCommand(this->m_redisContext, CommandText);

	//			//RedisDebugLog
	//			//pLogger->Log(ELEVEL_1
	//			//	, "Thread[%10I64u] RedisDebugLog: %s { "
	//			//	" CommandText \"%s\""
	//			//	" l_redisReply = 0x%p"
	//			//	" l_redisReply->type = %li"
	//			//	" l_redisReply->integer = %lli"
	//			//	" PoolObjectID %llu"
	//			//	" } "
	//			//	, GetCurrentThreadId() * 1000 + ApplicationID
	//			//	, __FUNCTION__
	//			//	, CommandText
	//			//	, (void*)l_redisReply
	//			//	, (l_redisReply != nullptr) ? l_redisReply->type : 0
	//			//	, (l_redisReply != nullptr) ? l_redisReply->integer : 0
	//			//	, this->PoolObjectID
	//			//	);

	//		}
	//	}

	//	return l_redisReply;
	//}

	redisReply* PublishLogstream(LPCSTR Text)
	{
		redisReply* l_redisReply = nullptr;

		// Try four times if either the conection failover mechanism can't get a hold of a redis server or if the command returns empty
		for (unsigned int i = 0; ((i < 4) && (l_redisReply == nullptr)); ++i)
		{

			if (this->Connect())
			{
				l_redisReply = (redisReply*)redisCommand(this->m_redisContext, "publish logstreamaw %s", Text);
			}

		}

		return l_redisReply;
	}


};

int _tmain(int argc, _TCHAR* argv[])
{
	strncpy_s(g_RedisConnectionRTLogServer.Host, "logstream.wasras.com", _TRUNCATE);
	g_RedisConnectionRTLogServer.Port = 6379;

	CRTLogtoREDIS l_CRTLogtoREDIS;

	l_CRTLogtoREDIS.Log("This is a fucking test by Vic the Master. Be advised!");
	l_CRTLogtoREDIS.Log("Hello!World");

	system("pause");
	//getchar();

	return 0;
}

