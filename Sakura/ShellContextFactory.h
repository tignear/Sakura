#pragma once
#include <variant>
#include <functional>
#include <string>
#include "ShellContext.h"
namespace tignear::sakura {
	class ShellContextFactory {
	public:
		struct Information {
			uint32_t width;
			uint32_t height;
			std::function<void*(std::u16string_view)> getResource;
		};
		virtual std::shared_ptr<ShellContext> Create(const Information&)=0;
	};
}