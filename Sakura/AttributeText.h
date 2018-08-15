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
		bool fluktur;//�T�|�[�g���Ȃ��Ă������񂶂��?
		Blink blink;
		bool conceal;//�B��
		unsigned int font;//0-9
	};
	using LineText = std::list<AttributeText>;
}