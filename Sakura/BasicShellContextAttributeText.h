#pragma once
#include <unicode/ustring.h>
#include <unordered_map>
#include "ansi/AttributeText.h"
namespace tignear::sakura {
	using ColorTable = std::unordered_map<unsigned int, std::uint32_t>;
	enum class ColorType :unsigned char{
		ColorTrue=0,Color256=1,ColorSystem=2
	};
	struct Color{
		ColorType type;
		union {
			struct {
				unsigned char r;
				unsigned char g;
				unsigned char b;
			}color_true;
			unsigned char color_256;
			unsigned char color_system;
		};
	};
	static constexpr bool operator==(Color a, Color b) {
		if (a.type != b.type) {
			return false;
		}
		switch (a.type)
		{
		case ColorType::ColorTrue:
			return a.color_true.r == b.color_true.r&&a.color_true.g == b.color_true.g&&a.color_true.b == b.color_true.b;
		case ColorType::Color256:
			return a.color_256 == b.color_256;
		case ColorType::ColorSystem:
			return a.color_system == b.color_system;
		default:
			return false;
		}
		
	}
	struct Attribute {
		Color frColor;
		Color bgColor;
		bool bold:1;
		bool faint:1;
		bool italic:1;
		bool underline:1;
		bool fluktur:1;
		bool conceal:1;//‰B‚·
		bool crossed_out:1;
		bool reverse:1;
		ansi::Blink blink:2;
		unsigned int font:4;//0-9
	};
	static constexpr bool operator==(const Attribute& a,const Attribute& b) {
		return
			a.frColor == b.frColor&&
			a.bgColor == b.bgColor&&
			a.bold == b.bold&&
			a.faint == b.faint&&
			a.italic == b.italic&&
			a.underline == b.underline&&
			a.fluktur == b.fluktur&&
			a.conceal == b.conceal&&
			a.crossed_out == b.crossed_out&&
			a.reverse == b.reverse&&
			a.blink == b.blink&&
			a.font == b.font;
	}
	class AttributeTextImpl:public ansi::AttributeText{
		std::uint32_t ColorHelper(Color c) const;
		const Attribute m_attr;
		const ColorTable& m_system_color_table;
		const ColorTable& m_256_color_table;
		mutable icu::UnicodeString m_ustr;
	public:
		AttributeTextImpl(const icu::UnicodeString& ustr, const ColorTable& c_system, const ColorTable&c_256) :m_ustr(ustr),m_system_color_table(c_system),m_256_color_table(c_256),m_attr(){}
		AttributeTextImpl(const icu::UnicodeString& ustr, const ColorTable& c_system, const ColorTable&c_256,const  Attribute& attr) :m_ustr(ustr), m_system_color_table(c_system), m_256_color_table(c_256), m_attr(attr) {}

		std::wstring_view textW()const override;
		int32_t length()const override;
		std::uint32_t textColor()const override;
		std::uint32_t backgroundColor()const override;
		bool bold()const override;
		bool faint()const override;
		bool italic()const override;
		bool underline()const override;
		bool fluktur()const override;
		ansi::Blink blink()const override;
		bool conceal() const override;
		bool crossed_out()const override;
		unsigned int font()const override;
		const Attribute& attribute()const;
		const icu::UnicodeString& ustr()const;
		icu::UnicodeString& ustr();
	};
}
