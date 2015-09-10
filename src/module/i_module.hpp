#ifndef _I_MODULE_HPP_
#define _I_MODULE_HPP_

#include <string>

namespace face2wind
{
	class IModule
	{
		friend class ModuleDriver;

	public:
		enum
		{
			Succeed = 0,
			Fail,
			Pending,
		};
		enum State
		{
			ST_Created,
			ST_Inited,
			ST_Started,
			ST_Running,
			ST_Stoped,
			ST_Released,

			ST_Invalid,
		};

		IModule() : m_module_state(ST_Created) {}
		virtual ~IModule(){}

		/*
		 * 初始化数据，读取
		 */
		virtual int Init() = 0;

		/*
		 * 开始运行
		 */
		virtual int Start() = 0;

		/*
		 * 每帧调用一次
		 */
		virtual int Update() = 0;

		/*
		 * 停止运行
		 */
		virtual int Stop() = 0;

		/*
		 * 释放当前数据，持久存储
		 */
		virtual int Release() = 0;

		int GetState() const { return m_module_state;}

	private:
		State m_module_state;

	};
}

#endif