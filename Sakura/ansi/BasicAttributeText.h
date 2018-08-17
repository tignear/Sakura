#pragma once
#include "AttributeText.h"
namespace tignear::ansi {
	struct BasicAttributeText:public AttributeText {
		BasicAttributeText(std::wstring text,std::wstring raw,size_t start):
			m_text(text),
			m_rawtext(raw),
			m_startIndex(start){}
		std::wstring_view text()const override{
			return m_text;
		}

		std::wstring_view rawText()const override {
			return m_rawtext;
		}
		std::wstring& rawText() {
			return m_rawtext;
		}
		size_t startIndex()const override {
			return m_startIndex;
		}
		size_t length()const override {
			return m_text.length();
		}
		std::uint32_t textColor()const  override {
			return m_textColor;
		}
		std::uint32_t backgroundColor()const override {
			return m_backgroundColor;
		}
		bool bold()const  override {
			return m_bold;
		}
		bool faint()const  override {
			return m_faint;
		}
		bool italic()const  override {
			return m_italic;
		}
		bool underline()const  override {
			return m_underline;
		}
		bool fluktur()const override {
			return m_fluktur;
		}
		Blink blink()const  override {
			return m_blink;
		}
		bool conceal() const override {
			return m_conceal;
		}//‰B‚·
		unsigned int font()const  override {
			return m_font;
		}//0-9
		void text(std::wstring& text) {
			m_text = text;
		}
		void text(std::wstring&& text) {
			m_text = text;
		}
		void rawText(std::wstring& rawtext) {
			m_rawtext = rawtext;
		}
		void rawText(std::wstring&& rawtext) {
			m_rawtext = rawtext;
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
		std::wstring m_rawtext;
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