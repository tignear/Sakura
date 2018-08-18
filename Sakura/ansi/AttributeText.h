#pragma once
#include <string>
namespace tignear::ansi {
	enum Blink {
		None, Slow, Fast
	};
	struct AttributeText {
		AttributeText(std::wstring text,size_t start):
			m_text(text),
			m_startIndex(start){}
		std::wstring_view text()const{
			return m_text;
		}
		std::wstring& text() {
			return m_text;
		}
		std::wstring::size_type startIndex()const {
			return m_startIndex;
		}
		std::wstring::size_type length()const {
			return m_text.length();
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
		void text(std::wstring& text) {
			m_text = text;
		}
		void text(std::wstring&& text) {
			m_text = text;
		}
		void startIndex(size_t index) {
			m_startIndex = index;
		}

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
		std::wstring m_text;
		std::wstring::size_type m_startIndex;
		std::uint32_t m_textColor;
		std::uint32_t m_backgroundColor;
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