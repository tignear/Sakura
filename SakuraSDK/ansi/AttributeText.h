#pragma once
#include <string>
#include <functional>
namespace tignear::ansi {

	enum class Blink:unsigned char{
		None=0, Slow=1, Fast=2
	};
	struct AttributeText {
		virtual std::wstring_view textW()const=0;
		virtual size_t length()const=0;
		virtual std::uint32_t textColor()const = 0;
		virtual std::uint32_t backgroundColor()const = 0;
		virtual bool bold()const = 0;
		virtual bool faint()const = 0;
		virtual bool italic()const = 0;
		virtual bool underline()const = 0;
		virtual bool fluktur()const = 0;
		virtual Blink blink()const = 0;
		virtual bool conceal() const = 0;
		virtual bool crossed_out()const = 0;
		virtual const std::wstring& font()const = 0;
		virtual ~AttributeText() {};
	};
#pragma warning(disable:4505)
	static bool EqAttr(AttributeText& a, AttributeText& b) {
		return a.bold() == b.bold() &&
			a.faint() == b.faint() &&
			a.italic() == b.italic() &&
			a.underline() == b.underline() &&
			a.fluktur() == b.fluktur() &&
			a.blink() == b.blink() &&
			a.conceal() == b.conceal() &&
			a.crossed_out() == b.crossed_out() &&
			a.font() == b.font();
	}
#pragma warning(default:4505)

}