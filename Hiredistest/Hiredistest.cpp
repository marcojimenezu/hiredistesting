#include "stdafx.h"
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
	}
	virtual ~CConnectionInformation()
	{}

	char Host[129];
	unsigned long Port;

};

CConnectionInformation g_RedisConnectionRTLogServer;

// No synchronization needed - this will called in teh Thread reading the Log Queue which is already synchronized
class CRTLogtoREDIS
{
	CRTLogtoREDIS()
	:m_redisContext(nullptr)
	{

	}

	virtual ~CRTLogtoREDIS()
	{

	}

	bool Connect()
	{
		bool l_Result(false);

		if (!this->m_redisContext)
		{
			timeval timeout = { 0, 500000 }; // 0.5 seconds
			this->m_redisContext = redisConnectWithTimeout(g_RedisConnectionRTLogServer.Host, g_RedisConnectionRTLogServer.Port, timeout);

			if ((this->m_redisContext != nullptr) && (this->m_redisContext->err == REDIS_OK))
			{
				redisEnableKeepAlive(this->m_redisContext);

				if (!this->Authenticate())
				{
					redisFree(this->m_redisContext);
					this->m_redisContext = nullptr;
				}
			}
		}

		if ((this->m_redisContext != nullptr) && (this->m_redisContext->err == REDIS_OK))
			l_Result = true;
		else
		{
			redisFree(this->m_redisContext);
			this->m_redisContext = nullptr;
		}

		return (l_Result);
	}


private:
	redisContext* m_redisContext;


};



int hiredisConnection()
{
	timeval timeout = { 0, 500000 }; // 0.5 seconds
	this->Context = redisConnectWithTimeout(l_ConnectionInformation.Host, l_ConnectionInformation.Port, timeout);


	if (context == nullptr || context->err) {
		if (context) {
			std::cerr << "Error: " << context->errstr << std::endl;
			redisFree(context);
		}
		else {
			std::cerr << "Can't allocate Redis context" << std::endl;
		}
		getchar();
		return 1;
	}

	redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", "Marco", "Testing2");
	if (reply == nullptr) {
		std::cerr << "SET command failed" << std::endl;
		redisFree(context);
		getchar();
		return 1;
	}
	std::cout << "SET: " << reply->str << std::endl;
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(context, "GET %s", "Marco");
	if (reply->type == REDIS_REPLY_STRING) {
		std::cout << "GET: " << reply->str << std::endl;
		std::cout << "The test was successful" << std::endl;
	}
	else {
		std::cerr << "GET command failed" << std::endl;
	}
	freeReplyObject(reply);
	redisFree(context);

	getchar();
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	strncpy_s(g_RedisConnectionRTLogServer.Host, "logstream.wasras.com", _TRUNCATE);
	g_RedisConnectionRTLogServer.Port = 6379;
	hiredisConnection();
	getchar();
	return 0;
}

