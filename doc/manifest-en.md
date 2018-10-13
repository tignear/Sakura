```
ret={
	manifestVersion=1,
	name='pluginName',#require unique
	dependencies={'name of depends plugin'}
	main=function(info){
		local cpuarchString=info.arch #'x64' or 'x86'
		local manifestPath=info.manifestPath #path to manifest
	}
}
return ret
```