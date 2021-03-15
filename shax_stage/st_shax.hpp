#include "../shared/shax.hpp"
#include <vector>

class shax : public shax_base {
public:
	shax( SOCKADDR_IN addr, SOCKET sock ) : shax_base() {
		m_sock = sock;
		m_addr = addr;
	}
};