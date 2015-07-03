#ifndef _GAME_HPP_
#define _GAME_HPP_

#include "network/network_basic.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <map>
#include <string>

namespace face2wind
{
	class Game
	{
		typedef boost::shared_ptr<IModule> IModulePtr;
		typedef std::map<std::string, IModulePtr> ModuleMap;
		typedef std::map<std::string, IModulePtr>::iterator ModuleMapIterator;

	public:
		~Game();

		static Game *GetInstance();

		void Run();
		void Stop();

		bool RegisterModule(const std::string name, IModule* module);
		bool UnregisterModule(const std::string name);

	private:
		Game();
		Game(const Game&);
		Game& operator=(const Game&);

	protected:
		bool keep_running;
		ModuleMap module_map;
	};
}

#endif