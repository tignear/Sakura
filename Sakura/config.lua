local conf={}
conf.shells={
    {
        display="cmd",
        factory="ConPTYShellContextFactory",
        cmd=[[cmd]],
        fontsize=16.0,
        fonts={"Cica"}
    },
    {
        display="cmd(admin)",
        factory="ConPTYShellContextFactory",
        admin=true,
        cmd=[[cmd]],
        fontsize=16.0,
        fonts={"Cica"}
    },
    {
        display="cmd(ConsoleRead)",
        factory="ConsoleReadShellContextFactory",
        cmd=[[cmd]],
        fontsize=16.0,
        font="Cica"
    },
    {
        display="cmd(ConsoleRead&admin)",
        factory="ConsoleReadShellContextFactory",
        admin=true,
        cmd=[[cmd]],
        fontsize=16.0,
        font="Cica"
    },
}
conf.initshell=conf.shells[3]

return conf