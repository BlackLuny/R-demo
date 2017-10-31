#pragma once
#define DLL_EXPORT
#include"Preinclude.h"
#include"NetworkDef.h"

namespace GuruLib
{
	class SocketBase
	{
	private:
		WSADATA wsa;
	protected:
		char msg[100];
		SocketErrorMessageHandler handler;
		bool is_socket_initialized;
#ifdef STATIC_LINK
		bool wsa_startup = false;
#endif // STATIC_LINK

		SOCKET master_socket;

		bool UnitSendForText(SOCKET s, const char* data, int* bytes_totally_sent, int text_len);//return value is actually bytes sent;text len does not count the NULL terminator;����UDP���ӣ�text_len���ֵΪ������������д�뻺�棨��ͨ��getsockopt��ѡ��SO_MAX_MSG_SIZE��ã�������TCP���ӣ�text_len����Ҫ��
	public:
		char recv_buffer[2000];
		int recv_size;

		SocketBase(InternetProtocolVersion ip_ver = IPv4, SocketErrorMessageHandler _handler = NULL);
		~SocketBase();
	};
}
