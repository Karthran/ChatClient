#pragma once

enum class OperationCode
{
	STOP,
	CHECK_SIZE,
	CHECK_NAME,
	CHECK_LOGIN,
	REGISTRATION,
	SIGN_IN,
	READY,
	ERROR,
};

const int HEADER_SIZE = 256;
