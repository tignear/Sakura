#pragma once
#include <string>
#include <d2d1_2.h>
#include <dwrite_1.h>
#include <wrl.h>
namespace tignear::dwrite {
	/// <summary>
	/// update系の関数は容赦なくキャッシュをクリアします。
	/// </summary>
	class TextBuilder
	{
	public:
		TextBuilder(IDWriteFactory*, LPCWSTR fontName, DWRITE_FONT_WEIGHT, DWRITE_FONT_STYLE, DWRITE_FONT_STRETCH, FLOAT fontSize, LPCWSTR locale);
		IDWriteFactory* GetDWriteFactory() const { return m_factory; }
		/// <summary>
	   /// キャッシュの更新のためにconstではありません。
	   /// </summary>
	   /// <returns></returns>
		Microsoft::WRL::ComPtr<IDWriteTextFormat> GetTextFormat();
		void UpdateFontName(LPCWSTR);
		void UpdateFontCollection(IDWriteFontCollection*);
		void UpdateFontWeight(DWRITE_FONT_WEIGHT);
		void UpdateFontStyle(DWRITE_FONT_STYLE);
		void UpdateFontStretch(DWRITE_FONT_STRETCH);
		void UpdateFontSize(FLOAT);
		void UpdateLocale(LPCWSTR);
		Microsoft::WRL::ComPtr<IDWriteTextLayout1> CreateTextLayout(const std::wstring&,FLOAT maxWidth,FLOAT maxHeight);
		~TextBuilder();
	private:
		IDWriteFactory * m_factory;
		LPCWSTR m_fontName;
		IDWriteFontCollection* m_fontCollection = nullptr;
		DWRITE_FONT_WEIGHT m_fontWeight;
		DWRITE_FONT_STYLE m_fontStyle;
		DWRITE_FONT_STRETCH m_fontStretch;
		FLOAT m_fontSize;
		LPCWSTR m_locale;
		void ClearTextFormatCache();
		Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
	};

}