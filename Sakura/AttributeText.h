#pragma once
#include <string>
#include <cstdint>
namespace tignear::sakura {
	enum Blink {
		None,Slow,Fast
	};
	struct AttributeText {
		virtual std::wstring_view text()const =0;
		virtual std::wstring_view rawText()const = 0;
		virtual size_t startIndex()const = 0;
		virtual size_t length()const = 0;
		virtual std::uint32_t textColor()const =0;
		virtual std::uint32_t backgroundColor()const =0;
		virtual bool bold()const =0;
		virtual bool faint() const = 0;
		virtual bool italic() const = 0;
		virtual bool underline() const = 0;
		virtual bool fluktur()const = 0;//サポートしなくてもいいんじゃね?
		virtual Blink blink()const = 0;
		virtual bool conceal()const =0;//隠す
		virtual unsigned int font()const =0;//0-9
		/*virtual void text(std::wstring_view) = 0;
		virtual void rawText(std::wstring_view) = 0;
		virtual void startIndex(size_t index)=0;
		virtual void length(size_t) = 0;
		virtual void textColor(std::uint32_t)=0;
		virtual void backgroundColor(std::uint32_t)=0;
		virtual void bold(bool)=0;
		virtual void faint(bool) = 0;
		virtual void italic(bool) = 0;
		virtual void underline(bool) = 0;
		virtual void fluktur(bool) = 0;
		virtual void blink(Blink) = 0; 
		virtual void conceal(bool)=0;
		virtual void font(unsigned int)=0;*/
	};
}