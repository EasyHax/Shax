#pragma once

#include <string>
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <iostream>

#include "utils.hpp"

#define NET_PORT 51337

class parser {
public:
    parser( std::string raw_str ) {

        auto word = std::string( "" );

        for ( auto x : raw_str )
            if ( x == ' ' )
            {
                m_arguments.push_back( word );
                word.clear();
            }
            else { word += x; }

        m_arguments.push_back( word );
    }

    const int& arg_count( void ) const {
        return m_arguments.size() - 1;
    }

    const std::string& get_argument( const int index ) const {
        return m_arguments[index + 1 ];
    }

    const std::string& get_argument( const std::string& argument ) const {
        if ( has_argument( argument ) ) {
            auto it = std::find( m_arguments.begin(), m_arguments.end(), argument );
            return m_arguments[std::distance( m_arguments.begin(), it ) + 1];
        }
    }

    bool has_argument( const std::string& argument ) const {
        auto it = std::find( m_arguments.begin(), m_arguments.end(), argument );

        if ( it == m_arguments.end() )
            return false;

        return std::distance( m_arguments.begin(), it ) < m_arguments.size() - 1;
    }

    bool has_flag( const std::string& flag ) const {
        return std::find( m_arguments.begin(), m_arguments.end(), flag ) != m_arguments.end();
    }

    bool is_cmd( const std::string& cmd ) const {
        return m_arguments[0] == cmd;
    }

    std::vector <std::string> m_arguments;
};

struct zero_copy {
public:
    zero_copy( std::string& filename ) {
        m_filename = filename.c_str();
    }

    bool map() {
        m_handle = CreateFileA( m_filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL );
        m_size = GetFileSize( m_handle, NULL );

        if ( m_bytes = (byte*)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, m_size ) ) {
            auto read = SIZE_T( 0 );
            return ReadFile( m_handle, m_bytes, m_size, &read, NULL );
        }

        return false;
    }

    bool unmap() {
        return CloseHandle( m_handle ) && HeapFree( GetProcessHeap(), 0, m_bytes );
    }

    size_t m_size;
    byte* m_bytes;

private:
    const char* m_filename;
    void* m_handle;
};

class shax_base {
public:
    shax_base() {

        if ( !utils::start_wsa() ) {
            printf( "[!] Cannot start WSA!\n" );
            exit( 0 );
        }

        m_sock      = NULL;
        m_addr = SOCKADDR_IN{};
    }

    template<typename T>
    bool snd( T value ) {
        return send( m_sock, reinterpret_cast<char*>(&value), sizeof( T ), NULL ) == sizeof ( T );
    }

    template<typename T>
    std::pair<T, bool> rcv( bool ensure_data = true ) {
        auto buffer = new char[ sizeof( T ) ]();

        auto success = ensure_data ?
            recv( m_sock, buffer, sizeof( T ), NULL ) == sizeof( T ) :
            recv( m_sock, buffer, sizeof( T ), NULL ) >= 0;

        return std::make_pair( *reinterpret_cast<T*>(buffer), success );
    }

    template<typename T>
    bool snd_arr( T* buffer, size_t a_size, bool ensure_data = true ) {
        return snd<size_t>( a_size ) && send( m_sock, (char*)buffer, a_size, NULL ) == a_size;
    }

    bool snd_arr( zero_copy* zc ) {
        return snd_arr( zc->m_bytes, zc->m_size );
    }

    template<typename T>
    std::pair<T*, bool> rcv_arr( bool ensure_data = true ) {

        auto data = rcv<size_t>();

        if ( !data.second )
            return std::make_pair( &T(), false );

        auto buffer = new char[data.first];

        auto success = ensure_data ?
            recv( m_sock, buffer, data.first, NULL ) == data.first :
            recv( m_sock, buffer, data.first, NULL ) >= 0;

        return std::make_pair( reinterpret_cast<T*>(buffer), success );
    }

    bool snd_str( const char* str, bool ensure_data = true, bool null_terminated_str = true ) {
        auto size = strlen( str ) + int( null_terminated_str );
        return ensure_data ?
            snd<size_t>( size ) && send( m_sock, str, size, NULL ) == size :
            send( m_sock, str, size, NULL ) == size;
    }

    std::pair<std::string, bool> rcv_str( bool ensure_data = true ) {

        auto data = ensure_data ? rcv<size_t>() : std::make_pair( size_t( 4096 ), true );

        if ( !data.second )
            return std::make_pair( std::string(), false );

        auto buffer = new char[data.first]();

        auto success = ensure_data ? 
            recv( m_sock, buffer, data.first, NULL ) == data.first :
            recv( m_sock, buffer, data.first, NULL ) >= 0;

        return std::make_pair( std::string( buffer ), success );
    }

    bool upload( std::string from, std::string to ) {
        auto zc = new zero_copy( from );

        return zc->map()
            && snd_arr( zc )
            && zc->unmap()
            && snd_str( to.c_str() );
    }

    bool download( std::string from, std::string to ) {



    }

    void dispose() {
        closesocket( m_sock );
        m_addr = {};
        m_sock = {};
    }

    SOCKADDR_IN m_addr;
    SOCKET m_sock;
};