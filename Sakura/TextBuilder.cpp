#include "stdafx.h"
#include "TextBuilder.h"
#include "FailToThrow.h"

using namespace tignear::dwrite;
TextBuilder::TextBuilder(IDWriteFactory* factory, LPCWSTR fontName, DWRITE_FONT_WEIGHT w, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, FLOAT fontSize, LPCWSTR locale):
	m_factory(factory)
{
	m_fontName = fontName;
	m_fontWeight = w;
	m_fontStyle = style;
	m_fontStretch = stretch;
	m_fontSize = fontSize;
	m_locale = locale;
}

void TextBuilder::UpdateFontName(const LPCWSTR fname)
{
	m_fontName = fname;
	ClearTextFormatCache();
}
void TextBuilder::UpdateFontCollection( IDWriteFontCollection* collection)
{
	m_fontCollection = collection;
	ClearTextFormatCache();
}
void TextBuilder::UpdateFontWeight(DWRITE_FONT_WEIGHT weight)
{
	m_fontWeight = weight;
	ClearTextFormatCache();
}

void TextBuilder::UpdateFontStyle(DWRITE_FONT_STYLE style)
{
	m_fontStyle = style;
	ClearTextFormatCache();
}
void TextBuilder::UpdateFontStretch(DWRITE_FONT_STRETCH stretch)
{
	m_fontStretch = stretch;
	ClearTextFormatCache();
}
void TextBuilder::UpdateFontSize(FLOAT size)
{
	m_fontSize = size;
	ClearTextFormatCache();
}
void TextBuilder::UpdateLocale(LPCWSTR locale) {
	m_locale = locale;
	ClearTextFormatCache();
}
void TextBuilder::ClearTextFormatCache() {
	m_textFormat.Reset();
}
Microsoft::WRL::ComPtr<IDWriteTextFormat> TextBuilder::GetTextFormat() {
	if (m_textFormat) {
		return m_textFormat;
	}
	else 
	{
		FailToThrowHR(
			m_factory->CreateTextFormat(m_fontName, m_fontCollection, m_fontWeight, m_fontStyle, m_fontStretch, m_fontSize, m_locale, &m_textFormat)
		);
		return m_textFormat;
	}
}
Microsoft::WRL::ComPtr<IDWriteTextLayout> TextBuilder::CreateTextLayout(const std::wstring& src, FLOAT maxWidth, FLOAT maxHeight) {
	Microsoft::WRL::ComPtr<IDWriteTextLayout> r;
	FailToThrowHR(m_factory->CreateTextLayout(src.c_str(),static_cast<UINT32>(src.length()),GetTextFormat().Get(),maxWidth,maxHeight,&r));
	return r;
}
TextBuilder::~TextBuilder()
{
}
