#pragma once
#include <windows.h>
#include <tchar.h>
namespace tignear::win {

	//http://www7.plala.or.jp/kfb/program/exedir.html
	//を改変して使用
	class ExecutableFilePath
	{
	public:
		ExecutableFilePath()
		{
			if (GetModuleFileName(NULL, m_Path, MAX_PATH))    //実行ファイルのフルパスを取得
			{   //取得に成功
				TCHAR* ptmp = _tcsrchr(m_Path, _T('\\')); // \の最後の出現位置を取得
				if (ptmp != NULL)
				{   //ファイル名を削除
					ptmp = _tcsinc(ptmp);   //一文字進める
					*ptmp = _T('\0');
				}
				else
				{
					std::terminate();
				}
			}
			else
			{
				std::terminate();
			}
		}
		const TCHAR* GetPath() const
		{
			return m_Path;
		}

	protected:
		TCHAR m_Path[MAX_PATH];
	};
	static inline const TCHAR* GetExecutableFilePath()
	{
		static ExecutableFilePath path;
		return path.GetPath();
	}


}