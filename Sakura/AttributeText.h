#pragma once
#include <string>
#include <cstdint>
namespace tignear::sakura {
	enum Blink {
		None,Slow,Fast
	};
	struct AttributeText {
		std::wstring raw_text;
		std::wstring text;
		std::uint32_t textColor;
		std::uint32_t backgroundColor;
		bool bold;
		bool faint;
		bool italic;
		bool underline;
		bool fluktur;//サポートしなくてもいいんじゃね?
		Blink blink;
		bool conceal;//隠す
		unsigned int font;//0-9
	};
	using LineText = std::list<AttributeText>;
}