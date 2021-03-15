#include "s_shax.hpp"

int main( void ) {

	auto shx = new shax();

	while ( !shx->connect( "192.168.1.32", NET_PORT ) ) {
		Sleep( 100 );
	}

	getchar();

	shx->dispose();
	return 0;
}