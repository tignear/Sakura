local ret={
	manifestVersion=1,
	main=function(info)
		local dir=string.match(info.manifestPath,"(.+%\\)manifest.lua")..info.arch.."\\"
		tignear.sakura.AddDllDirectory(dir)
	end,
	name='tignear.sakura.ICULoader',
	dependencies={},
}
return ret