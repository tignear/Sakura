local ret={
	manifestVersion=1,
	main=function(info)
		if not tignear.sakura.DLLWrapper.LoadLibrary("Kernel32.dll"):IsDefined("CreatePseudoConsole") then
			return
		end
		local dir=string.match(info.manifestPath,"(.+%\\)manifest.lua")
		tignear.sakura.loadPluginDLL(dir..'plugin.dll')
	end,
	name='tignear.sakura.ConPTYShellContext',
	dependencies={'tignear.sakura.ICULoader'}
}
return ret