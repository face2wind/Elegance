#include "network/network_manager.hpp"
#include <boost/thread.hpp>
#include "game.hpp"

namespace face2wind
{
	Game::Game() : keep_running(false)
	{

	}

	Game::~Game()
	{

	}

	Game *Game::GetInstance()
	{
		static Game instance;
		return &instance;
	}

	void Game::Run()
	{
		for (ModuleMapIterator it = module_map.begin(); it != module_map.end(); ++ it)
		{
			if (IModule::ST_Invalid == it->second->m_module_state)
			{
				module_map.erase(it);
			}
			if (IModule::ST_Created == it->second->m_module_state || IModule::ST_Released == it->second->m_module_state)
			{
				it->second->Init();
				it->second->m_module_state = IModule::ST_Inited;
			}
			if (IModule::ST_Inited == it->second->m_module_state || IModule::ST_Stoped == it->second->m_module_state )
			{
				it->second->Start();
				it->second->m_module_state = IModule::ST_Started;
			}
		}

		keep_running = true;
		while(keep_running)
		{
			for (ModuleMapIterator it = module_map.begin(); it != module_map.end(); ++ it)
			{
				it->second->m_module_state = IModule::ST_Running;
				it->second->Update();
			}
			boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		}
	}

	void Game::Stop()
	{
		keep_running = false;

		for (ModuleMapIterator it = module_map.begin(); it != module_map.end(); ++ it)
		{
			it->second->Stop();
			it->second->m_module_state = IModule::ST_Stoped;

			it->second->Release();
			it->second->m_module_state = IModule::ST_Released;
		}
	}

	bool Game::RegisterModule(const std::string name, IModule* module)
	{
		if (NULL == module)
		{
			return false;
		}

		if (0 < module_map.count(name))
		{
			return false;
		}
	
		module_map[name] = IModulePtr(module);
		return true;
	}

	bool Game::UnregisterModule(const std::string name)
	{
		if (0 >= module_map.count(name))
		{
			return false;
		}

		module_map.erase(name);
		return true;
	}
}