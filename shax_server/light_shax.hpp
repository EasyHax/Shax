#pragma once

#define SOCKET_FAILED( x ) x == SOCKET_ERROR
#pragma comment(lib, "Ws2_32.lib")

#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>

#define NET_PORT 51337

class shax_base {
public:
    shax_base() {

        WSADATA wsd;
        if ( WSAStartup( MAKEWORD( 2, 2 ), &wsd ) ) {
            printf( "[!] Cannot start WSA!\n" );
            exit( 0 );
        }

        m_sock = NULL;
        m_addr = SOCKADDR_IN{};
    }

    template<typename T>
    std::pair<T, bool> rcv( bool ensure_data = true ) {
        auto buffer = new char[sizeof( T )]();

        auto success = ensure_data ?
            recv( m_sock, buffer, sizeof( T ), NULL ) == sizeof( T ) :
            recv( m_sock, buffer, sizeof( T ), NULL ) >= 0;

        return success ?
            std::make_pair( *reinterpret_cast<T*>(buffer), true ) :
            std::make_pair( *reinterpret_cast<T*>(buffer), false );
    }

    template<typename T>
    std::pair<T*, bool> rcv_arr( bool ensure_data = true ) {

        auto data = rcv<size_t>();

        if ( !data.second )
            return std::make_pair( nullptr, false );

        auto buffer = new char[data.first];

        auto success = ensure_data ?
            recv( m_sock, buffer, data.first, NULL ) == data.first :
            recv( m_sock, buffer, data.first, NULL ) >= 0;

        return success ?
            std::make_pair( reinterpret_cast<T*>(buffer), true ) :
            std::make_pair( reinterpret_cast<T*>(buffer), false );
    }

    void dispose() {
        closesocket( m_sock );
        m_addr = {};
        m_sock = {};
    }

    SOCKADDR_IN m_addr;
    SOCKET m_sock;
};