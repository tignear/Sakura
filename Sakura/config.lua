local conf={
    shells={
        {
            display="Cmd",
            factory="ConPTYShellContextFactory",
            cmd=[[cmd]],
            codepage=65001,
            use_terminal_echoback=false,
            fontsize=16.0,
            fonts={"Cica"}
        },
        {
            display="cmd(admin)",
            factory="ConPTYShellContextFactory",
            admin=true,
            cmd=[[cmd]],
            codepage=932,
            use_terminal_echoback=false,
            fontsize=16.0,
            fonts={"Cica"}
        }
    },
    initshell=0
}

return conf