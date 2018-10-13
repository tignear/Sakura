local ret={
	manifestVersion=1,
	main=function(info)
		local dir=string.match(info.manifestPath,"(.+%\\)manifest.lua")
		tignear.sakura.loadPluginDLL(dir..'plugin.dll')
    end,
	name='tignear.sakura.DefaultShellContext',
	dependencies={'tignear.sakura.ICULoader'},
}
return ret