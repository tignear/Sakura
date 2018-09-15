#pragma once
#include "ShellContextFactory.h"
namespace tignear::sakura {
	class BasicShellContextFactory :public ShellContextFactory {
	public:
		std::shared_ptr<ShellContext> Create(const Information&)const override;
	};
}
