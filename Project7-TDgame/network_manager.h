#pragma once

#include "manager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <queue>
#include <string>
#include <sstream>

enum class NetworkRole
{
	Offline,
	Rpg,
	Tower
};

class NetworkManager : public Manager<NetworkManager>
{
	friend class Manager<NetworkManager>;

public:
	bool start_host(unsigned short port, NetworkRole local_role)
	{
		reset();
		if (!ensure_winsock()) return false;

		role = local_role;
		peer_role = NetworkRole::Offline;
		is_host = true;
		listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listen_socket == INVALID_SOCKET)
		{
			last_error = "socket failed";
			return false;
		}

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		if (bind(listen_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR ||
			listen(listen_socket, 1) == SOCKET_ERROR)
		{
			last_error = "bind/listen failed";
			reset();
			return false;
		}

		u_long non_blocking = 1;
		ioctlsocket(listen_socket, FIONBIO, &non_blocking);
		status = "Hosting on port " + std::to_string(port);
		return true;
	}

	bool join_host(const std::string& ip, unsigned short port, NetworkRole local_role)
	{
		reset();
		if (!ensure_winsock()) return false;

		role = local_role;
		peer_role = NetworkRole::Offline;
		peer_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (peer_socket == INVALID_SOCKET)
		{
			last_error = "socket failed";
			return false;
		}

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

		if (connect(peer_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			last_error = "connect failed";
			reset();
			return false;
		}

		u_long non_blocking = 1;
		ioctlsocket(peer_socket, FIONBIO, &non_blocking);
		connected = true;
		status = "Connected to " + ip;
		send_line("HELLO");
		send_line(std::string("ROLE ") + role_to_text(role));
		return true;
	}

	void on_update()
	{
		if (is_host && !connected && listen_socket != INVALID_SOCKET)
		{
			peer_socket = accept(listen_socket, nullptr, nullptr);
			if (peer_socket != INVALID_SOCKET)
			{
				u_long non_blocking = 1;
				ioctlsocket(peer_socket, FIONBIO, &non_blocking);
				connected = true;
				status = "Peer connected";
				send_line("JOIN");
				send_line(std::string("ROLE ") + role_to_text(role));
			}
		}

		if (connected)
			receive_pending();
	}

	void send_line(const std::string& line)
	{
		if (!connected || peer_socket == INVALID_SOCKET) return;
		std::string packet = line + "\n";
		send(peer_socket, packet.c_str(), (int)packet.size(), 0);
	}

	unsigned int next_sequence()
	{
		return ++local_sequence;
	}

	void mark_peer_role(NetworkRole role)
	{
		peer_role = role;
	}

	void set_start_level_id(int level_id)
	{
		start_level_id = level_id;
	}

	bool roles_ready() const
	{
		return connected && role != NetworkRole::Offline && peer_role != NetworkRole::Offline && role != peer_role;
	}

	bool poll_message(std::string& message)
	{
		if (messages.empty()) return false;
		message = messages.front();
		messages.pop();
		return true;
	}

	void reset()
	{
		if (connected && peer_socket != INVALID_SOCKET)
			send_line("DISCONNECT");
		if (peer_socket != INVALID_SOCKET)
		{
			closesocket(peer_socket);
			peer_socket = INVALID_SOCKET;
		}
		if (listen_socket != INVALID_SOCKET)
		{
			closesocket(listen_socket);
			listen_socket = INVALID_SOCKET;
		}
		connected = false;
		is_host = false;
		role = NetworkRole::Offline;
		peer_role = NetworkRole::Offline;
		start_level_id = 1;
		local_sequence = 0;
		recv_buffer.clear();
		while (!messages.empty())
			messages.pop();
		status = "Offline";
	}

	bool is_connected() const { return connected; }
	bool hosting() const { return is_host; }
	NetworkRole get_role() const { return role; }
	NetworkRole get_peer_role() const { return peer_role; }
	int get_start_level_id() const { return start_level_id; }
	const std::string& get_status() const { return status; }
	const std::string& get_last_error() const { return last_error; }

	static const char* role_to_text(NetworkRole role)
	{
		switch (role)
		{
		case NetworkRole::Rpg:
			return "RPG";
		case NetworkRole::Tower:
			return "TOWER";
		default:
			return "OFFLINE";
		}
	}

	static NetworkRole role_from_text(const std::string& text)
	{
		if (text == "RPG") return NetworkRole::Rpg;
		if (text == "TOWER") return NetworkRole::Tower;
		return NetworkRole::Offline;
	}

protected:
	NetworkManager() = default;
	~NetworkManager()
	{
		reset();
		if (winsock_ready)
			WSACleanup();
	}

private:
	bool winsock_ready = false;
	bool connected = false;
	bool is_host = false;
	NetworkRole role = NetworkRole::Offline;
	NetworkRole peer_role = NetworkRole::Offline;
	int start_level_id = 1;
	unsigned int local_sequence = 0;
	SOCKET listen_socket = INVALID_SOCKET;
	SOCKET peer_socket = INVALID_SOCKET;
	std::string status = "Offline";
	std::string last_error;
	std::string recv_buffer;
	std::queue<std::string> messages;

private:
	bool ensure_winsock()
	{
		if (winsock_ready) return true;
		WSADATA data = {};
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		{
			last_error = "WSAStartup failed";
			return false;
		}
		winsock_ready = true;
		return true;
	}

	void receive_pending()
	{
		char buffer[512] = {};
		int count = recv(peer_socket, buffer, sizeof(buffer) - 1, 0);
		if (count > 0)
		{
			recv_buffer.append(buffer, count);
			size_t pos = std::string::npos;
			while ((pos = recv_buffer.find('\n')) != std::string::npos)
			{
				messages.push(recv_buffer.substr(0, pos));
				recv_buffer.erase(0, pos + 1);
			}
		}
		else if (count == 0)
		{
			handle_peer_disconnected("Peer disconnected");
		}
		else
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				handle_peer_disconnected("recv failed");
			}
		}
	}

	void handle_peer_disconnected(const std::string& reason)
	{
		messages.push("DISCONNECT");
		last_error = reason;
		status = reason;
		if (peer_socket != INVALID_SOCKET)
		{
			closesocket(peer_socket);
			peer_socket = INVALID_SOCKET;
		}
		if (listen_socket != INVALID_SOCKET)
		{
			closesocket(listen_socket);
			listen_socket = INVALID_SOCKET;
		}
		connected = false;
		is_host = false;
		peer_role = NetworkRole::Offline;
		recv_buffer.clear();
	}
};
