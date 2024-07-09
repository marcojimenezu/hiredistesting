#include "stdafx.h"
#include <iostream>
#include <hiredis\hiredis.h>


int hiredisConnection(){
	//timeval timeout = { 60, 500000 };
	redisContext* context = redisConnect("logstream.wasras.com", 6379); // Auris
	//redisContext* context = redisConnectWithTimeout("172.25.25.19", 6379, timeout); // Auris

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
	hiredisConnection();
	getchar();
	return 0;
}

