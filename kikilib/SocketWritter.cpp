#include "SocketWritter.h"
#include "EventService.h"
#include "LogManager.h"

#include <string.h>
#include <sys/epoll.h>

using namespace kikilib;

SocketWtitter::SocketWtitter(Socket sock, EventService* pEvServe)
	: _sock(sock), _pEvServe(pEvServe), _leftBorder(0), _rightBorder(0)
{
	_buffer.resize(Parameter::bufferInitLen);
}

//写一个int
void SocketWtitter::SendInt32(int res)
{
	Send(std::move(std::to_string(res)));
}

//写一个字符串
void SocketWtitter::Send(std::string& str)
{
	WriteBufToSock();
	if (_rightBorder - _leftBorder)
	{//一次没写完
		if (_buffer.size() - _rightBorder < str.size())
		{
			_buffer.resize(_rightBorder + str.size());
		}
		memmove(&(_buffer[_rightBorder]), &(*str.begin()), str.size());
	}
	else
	{
		auto ret = _sock.Send(&(*str.begin()), str.size());
		if (ret < 0)
		{//error
			RecordLog(ERROR_DATA_INFORMATION, "write socket error!");
			return;
		}
		else if (ret < str.size())
		{//内容没发完
			_pEvServe->SetInteresEv(_pEvServe->GetInteresEv() | EPOLLOUT);
			auto leftLen = str.size() - ret;
			if (_buffer.size() - _rightBorder < leftLen)
			{
				_buffer.resize(_rightBorder + leftLen);
			}
			memmove(&(_buffer[_rightBorder]), &(*(str.begin() + ret)), leftLen);
		}
		else
		{//一次成功发送所有内容出去
			return;
		}
	}
}

//写一个字符串
void SocketWtitter::Send(std::string&& str)
{
	WriteBufToSock();
	if (_rightBorder - _leftBorder)
	{//一次没写完
		if (_buffer.size() - _rightBorder < str.size())
		{
			_buffer.resize(_rightBorder + str.size());
		}
		memmove(&(_buffer[_rightBorder]), &(*str.begin()), str.size());
	}
	else
	{
		auto ret = _sock.Send(&(*str.begin()), str.size());
		if (ret < 0)
		{//error
			RecordLog(ERROR_DATA_INFORMATION, "write socket error!");
			return;
		}
		else if (ret < str.size())
		{//内容没发完
			_pEvServe->SetInteresEv(_pEvServe->GetInteresEv() | EPOLLOUT);
			auto leftLen = str.size() - ret;
			if (_buffer.size() - _rightBorder < leftLen)
			{
				_buffer.resize(_rightBorder + leftLen);
			}
			memmove(&(_buffer[_rightBorder]), &(*(str.begin() + ret)), leftLen);
		}
		else
		{//一次成功发送所有内容出去
			return;
		}
	}
}

//将缓冲区内容写进socket中
void SocketWtitter::WriteBufToSock()
{
	int curLen = _rightBorder - _leftBorder;
	if (!curLen)
	{//没有东西写
		return;
	}
	auto ret = _sock.Send(&(_buffer[_leftBorder]), curLen);
	if (ret < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "write socket error!");
		return;
	}
	else if (ret < curLen)
	{//没写完f
		_pEvServe->SetInteresEv(_pEvServe->GetInteresEv() | EPOLLOUT);
	}
	_leftBorder += ret;

	if (_leftBorder >= (_buffer.size() / Parameter::bufMoveCriterion))
	{
		memmove(&(_buffer.front()), &(_buffer[_leftBorder]), _rightBorder - _leftBorder);
		_rightBorder -= _leftBorder;
		_leftBorder = 0;
	}
}
