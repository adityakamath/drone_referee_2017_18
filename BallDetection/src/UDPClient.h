#ifndef UDP_CLIENT
#define UDP_CLIENT

#include <iostream>
//#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "DroneState.h"

using boost::asio::ip::udp;

class UDPClient
{
public:
	UDPClient(
		const std::string& host,
		const std::string& port
	) : io_service_(io_service_udp), socket_(io_service_udp, udp::endpoint(udp::v4(), 0)) {
		udp::resolver resolver(io_service_);
		udp::resolver::query query(udp::v4(), host, port);
		udp::resolver::iterator iter = resolver.resolve(query);
		endpoint_ = *iter;
	}

	~UDPClient()
	{
		socket_.close();
	}

	void send(boost::asio::mutable_buffers_1 msg) {
		socket_.send_to(msg, endpoint_);
	}

	boost::asio::mutable_buffers_1 my_serialize(float x, float y) {
		float *fbuf = new float[2];
		fbuf[0] = x;
		fbuf[1] = y;
		return boost::asio::buffer((void *)fbuf, sizeof(fbuf));
	}

	void send_my_state(float x, float y) {
		boost::asio::mutable_buffers_1 buf = my_serialize(x, y);
		send(buf);
		free(boost::asio::buffer_cast<void *>(buf));
	}

private:
	boost::asio::io_service io_service_udp;
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint endpoint_;

};

#endif
