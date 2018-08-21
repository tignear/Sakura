#pragma once
#include <string>
#include "../EastAsianWidth.h"
#include <unicode/unistr.h>
#include <functional>
namespace tignear::ansi {
	enum Blink {
		None, Slow, Fast
	};
	struct AttributeText {
		AttributeText(icu::UnicodeString text):
			m_text(text),
			m_update_length_eaw(true),
			m_textColor(0x000000),
			m_backgroundColor(0xffffff),
			m_bold(false),
			m_faint(false),
			m_italic(false),
			m_underline(false),
			m_blink(None),
			m_conceal(false),
			m_font(0){}
		AttributeText(icu::UnicodeString text, std::uint32_t textcolor, std::uint32_t bgcolor,bool bold,bool faint,bool italic,bool underline, Blink blink,bool conceal,unsigned int font) :
			m_text(text),
			m_textColor(textcolor),
			m_backgroundColor(bgcolor),
			m_bold(bold),
			m_faint(faint),
			m_italic(italic),
			m_underline(underline),
			m_blink(blink),
			m_conceal(conceal),
			m_font(font) {}
		const icu::UnicodeString& text()const{
			return m_text;
		}
		void textE(std::function<bool(AttributeText&,icu::UnicodeString&)> fn) {
			if (fn(*this,m_text)) {
				m_update_length_eaw=true;
			}
		}
#if U_SIZEOF_WCHAR_T==2
		std::wstring_view textW()const {
			return std::wstring_view(reinterpret_cast<const wchar_t*>(m_text.getTerminatedBuffer()),length());
		}
#endif

		int32_t length()const {
			return m_text.length();
		}
		uint32_t lengthEAW() const{
			if (m_update_length_eaw) {
				m_length_eaw = tignear::icuex::EastAsianWidth(text());
				m_update_length_eaw = false;
			}
			return m_length_eaw;
		}
		std::uint32_t textColor()const  {
			return m_textColor;
		}
		std::uint32_t backgroundColor()const {
			return m_backgroundColor;
		}
		bool bold()const  {
			return m_bold;
		}
		bool faint()const  {
			return m_faint;
		}
		bool italic()const  {
			return m_italic;
		}
		bool underline()const  {
			return m_underline;
		}
		bool fluktur()const {
			return m_fluktur;
		}
		Blink blink()const  {
			return m_blink;
		}
		bool conceal() const {
			return m_conceal;
		}//‰B‚·
		unsigned int font()const  {
			return m_font;
		}//0-9

		void textColor(std::uint32_t color) {
			m_textColor = color;
		}
		void backgroundColor(std::uint32_t color) {
			m_backgroundColor = color;
		}
		void bold(bool bold) {
			m_bold = bold;
		}
		void faint(bool faint) {
			m_faint = faint;
		}
		void italic(bool italic) {
			m_italic = italic;
		}
		void underline(bool underline) {
			m_underline = underline;
		}
		void fluktur(bool fluktur) {
			m_fluktur = fluktur;
		}
		void blink(Blink blink) {
			m_blink = blink;
		}
		void conceal(bool conceal) {
			m_conceal = conceal;
		}
		void font(unsigned int font) {
			m_font = font;
		}
	private:
		mutable icu::UnicodeString m_text;
		std::uint32_t m_textColor;
		std::uint32_t m_backgroundColor;
		mutable bool m_update_length_eaw;
		mutable uint32_t m_length_eaw;
		bool m_bold;
		bool m_faint;
		bool m_italic;
		bool m_underline;
		bool m_fluktur;
		Blink m_blink;
		bool m_conceal;//‰B‚·
		unsigned int m_font;//0-9
	};
}