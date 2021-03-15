#pragma once

#pragma comment(lib, "Ws2_32.lib")

#include <ws2tcpip.h>

#define SOCKET_FAILED( x ) x == SOCKET_ERROR
#define CMD_MAX_BUFFER 18384

#define SESSION_DIED { m_shax->died(); return false; }
#define SAFE_RET(T, B) std::pair<T, B>

namespace {
	namespace msg_type {
		const char* success = "[+] ";
		const char* error   = "[!] ";
		const char* casual  = "[~] ";
		const char* raw     = "";
	};
};

enum msg_color {
	bright        = FOREGROUND_INTENSITY,
	blue          = FOREGROUND_BLUE,
	red           = FOREGROUND_RED,
	green         = FOREGROUND_GREEN,

	cyan          = blue  | green,
	yellow        = red   | green,
	magenta       = blue  | red,
	white         = blue  | red | green,

	light_blue    = blue  | bright,
	light_red     = red   | bright,
	light_green   = green | bright,

	light_cyan    = blue  | green | bright,
	light_yellow  = red   | green | bright,
	light_magenta = blue  | red   | bright,
	light_white   = blue  | red   | green | bright
};


namespace utils {
	inline bool start_wsa( void ) {
		WSADATA wsd;
		return !WSAStartup( MAKEWORD( 2, 2 ), &wsd );
	}

	inline std::string ip_of( SOCKADDR_IN socket_addr ) {
		char ip_str[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &socket_addr.sin_addr, ip_str, INET_ADDRSTRLEN );
		return std::string( ip_str );
	}

	inline std::string stdin_to_str( void ) {
		char buffer[1024] = { 0 };
		char* context;
		fgets( buffer, sizeof( buffer ), stdin );
		return std::string( buffer );
	}

	inline bool print( const char* msg, msg_color color = msg_color::white, const char* type = msg_type::casual ) {
		auto hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
		auto scbinfo = CONSOLE_SCREEN_BUFFER_INFO{};
		if ( GetConsoleScreenBufferInfo( hstdout, &scbinfo ) ) {
			if ( SetConsoleTextAttribute( hstdout, color ) ) {
				std::cout << type << msg;
				if ( SetConsoleTextAttribute( hstdout, scbinfo.wAttributes ) ) {
					return true;
				}
			}
		}
		return false;
	}

	inline bool print( std::string msg, msg_color color = msg_color::white, const char* type = msg_type::casual ) {
		return print( msg.c_str(), color, type );
	}

	inline bool println( const char* msg, msg_color color = msg_color::white, const char* type = msg_type::casual ) {
		return print( std::string( msg ) + "\n", color, type );
	}

	inline bool println( std::string msg, msg_color color = msg_color::white, const char* type = msg_type::casual ) {
		return println( msg.c_str(), color, type );
	}
}