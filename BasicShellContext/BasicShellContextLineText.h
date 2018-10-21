#pragma once
#include "BasicShellContextAttributeText.h"
#include "ShellContext.h"

namespace tignear::sakura {
	class attrtext_iterator_impl :public ShellContext::attrtext_iterator_innner {
	public:
		explicit attrtext_iterator_impl(std::list<AttributeTextImpl>::iterator elem)  :
			m_elem(elem) {
		}
		void operator++()override;
		attrtext_iterator_impl* operator++(int) override;
		const reference operator*()const override;
		const pointer operator->()const override;
		bool operator==(const attrtext_iterator_innner& iterator)const override;
		bool operator!=(const attrtext_iterator_innner& iterator)const override;
		attrtext_iterator_impl* clone()const override;
	private:
		attrtext_iterator_impl(const attrtext_iterator_impl&);
		std::list<AttributeTextImpl>::iterator m_elem;
	};
	class BasicShellContextLineText:public ShellContext::attrtext_line {
		const unsigned char m_ambiguous_size;//MAGIC_NUMBER

		std::list<AttributeTextImpl>& Value();
		std::shared_ptr<void> m_resource;

	public:
		BasicShellContextLineText(unsigned char ambiguous_size,const ansi::ColorTable& ct_sys,const ansi::ColorTable& ct_256, const std::vector<std::wstring>& fontmap):
			m_ambiguous_size(ambiguous_size),
			m_ct_sys(ct_sys),
			m_ct_256(ct_256),
			m_fontmap(fontmap){}
		const std::list<AttributeTextImpl>& Value()const;

		size_t Remove();
		size_t RemoveAfter(size_t p);
		size_t RemoveBefore(size_t p);
		static inline std::pair<int32_t,uint32_t> EAWtoIndex(const icu::UnicodeString&, uint32_t eaw,unsigned char ambiguous_size);
		static inline std::tuple<size_t, size_t, std::list<AttributeTextImpl>::iterator,int32_t,uint32_t> EAWtoIndexMulti(std::list<AttributeTextImpl>& ustr, size_t eaw, unsigned char ambiguous_size);

		ShellContext::attrtext_iterator begin() override;
		ShellContext::attrtext_iterator end() override;
		std::shared_ptr<void>& resource()override;
		bool operator==(const attrtext_line&)const override;
		bool operator!=(const attrtext_line&)const override;
		bool IsEmpty()const;
		size_t Insert(size_t,const icu::UnicodeString&, const Attribute& attr);
		size_t Erase(size_t,size_t);
	private:
		std::list<AttributeTextImpl> m_value;
		const ansi::ColorTable& m_ct_sys;
		const ansi::ColorTable& m_ct_256;
		const std::vector<std::wstring>& m_fontmap;
		static std::list<AttributeTextImpl> empty;
	};
}