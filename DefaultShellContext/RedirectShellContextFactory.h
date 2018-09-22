#pragma once
#include <ShellContextFactory.h>
namespace tignear::sakura {
	class RedirectShellContextFactory :public ShellContextFactory {
		HINSTANCE m_hinst;
	public:
		RedirectShellContextFactory(HINSTANCE hinst) :m_hinst(hinst) {}
		std::shared_ptr<ShellContext> Create(const Information&)const override;
	};
}
