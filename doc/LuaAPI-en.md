# LuaAPI
You can use Lua standard library and the following functions.
## Sakura Functions
  
### boolean loadPluginDLL(string path)
Load plugin dll.
1.Convert path to std::filesystem::path.
2.Execute LoadLibraryExW(path.wstring().c_str(), NULL, LOAD_LIBRARY_SEARCH_USER_DIRS| LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
3.Execute GetProcAddress(HMODULE, "CreatePlugin")();
Even if step 3 fails,the application keeps holding HMODULE.
Returns true if all steps success.Otherwise it return false.

### boolean AddDllDirectory(string path)
Similar to win32api's AddDllDirectory function.
1.Convert path to std::filesystem::path.
2.Call win32api's AddDllDirectory function.
Returns true if step 2 return non NULL.Otherwise it return false.

### int MessageBox(string message,string title="Sakura",int flag=0)
Similar to win32api's MessageBox function.
Convert string from utf8 to ansi.
HWND is the apllications's main window handle.
Call and return MessageBox(HWND,message,title,flag)