let version = Fnm.Fnm__Package.version;

module Commands = {
  let use = (version, quiet) => Lwt_main.run(Use.run(~version, ~quiet));
  let alias = (version, name) => Lwt_main.run(Alias.run(~name, ~version));
  let default = version =>
    Lwt_main.run(Alias.run(~name="default", ~version));
  let listRemote = () => Lwt_main.run(ListRemote.run());
  let listLocal = () => Lwt_main.run(ListLocal.run());
  let install = version => Lwt_main.run(Install.run(~version));
  let uninstall = version => Lwt_main.run(Uninstall.run(~version));
  let env =
      (
        isFishShell,
        isMultishell,
        nodeDistMirror,
        fnmDir,
        shell,
        useOnCd,
        logLevel,
      ) =>
    Lwt_main.run(
      Env.run(
        ~forceShell=Fnm.System.Shell.(isFishShell ? Some(Fish) : shell),
        ~multishell=isMultishell,
        ~nodeDistMirror,
        ~fnmDir,
        ~useOnCd,
        ~logLevel,
      ),
    );
};

open Cmdliner;

let help_secs = [
  `S(Manpage.s_common_options),
  `S(Manpage.s_environment),
  `P("These options are common to all commands."),
  `S("MORE HELP"),
  `P("Use `$(mname) $(i,COMMAND) --help' for help on a single command."),
  `Noblank,
  `S(Manpage.s_bugs),
  `P("File bug reports at https://github.com/Schniz/fnm"),
];

let envs =
  Fnm.Config.getDocs()
  |> List.map(envVar =>
       Fnm.Config.(
         Term.env_info(
           ~doc=
             Printf.sprintf(
               "%s\ndefaults to \"%s\"",
               envVar.doc,
               envVar.default,
             ),
           envVar.name,
         )
       )
     );

let install = {
  let doc = "Install another node version";
  let man = help_secs;
  let sdocs = Manpage.s_common_options;

  let selectedVersion = {
    let doc = "Install another version specified in $(docv).";
    Arg.(
      value & pos(0, some(string), None) & info([], ~docv="VERSION", ~doc)
    );
  };

  (
    Term.(const(Commands.install) $ selectedVersion),
    Term.info(
      "install",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let uninstall = {
  let doc = "Uninstall a node version";
  let sdocs = Manpage.s_common_options;
  let man = help_secs;

  let selectedVersion = {
    let doc = "Uninstall the node version specified in $(docv).";
    Arg.(
      required
      & pos(0, some(string), None)
      & info([], ~docv="VERSION", ~doc)
    );
  };

  (
    Term.(const(Commands.uninstall) $ selectedVersion),
    Term.info(
      "uninstall",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let listLocal = {
  let doc = "List all the installed versions";
  let sdocs = Manpage.s_common_options;
  let man = help_secs;

  (
    Term.(app(const(Commands.listLocal), const())),
    Term.info(
      "ls",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let listRemote = {
  let doc = "List all the versions upstream";
  let sdocs = Manpage.s_common_options;
  let man = help_secs;

  (
    Term.(app(const(Commands.listRemote), const())),
    Term.info(
      "ls-remote",
      ~version,
      ~envs,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let use = {
  let doc = "Switch to another installed node version";
  let man = help_secs;
  let sdocs = Manpage.s_common_options;

  let quiet = {
    let doc = "Don't print stuff";
    Arg.(value & flag & info(["quiet"], ~doc));
  };

  let selectedVersion = {
    let doc = "Switch to version $(docv).\nLeave empty to look for value from an `.nvmrc` or `.node-version` file";
    Arg.(
      value & pos(0, some(string), None) & info([], ~docv="VERSION", ~doc)
    );
  };

  (
    Term.(const(Commands.use) $ selectedVersion $ quiet),
    Term.info(
      "use",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let alias = {
  let doc = "Alias a version";
  let sdocs = Manpage.s_common_options;
  let man = help_secs;

  let selectedVersion = {
    let doc = "The version to be aliased";
    Arg.(
      required
      & pos(0, some(string), None)
      & info([], ~docv="VERSION", ~doc)
    );
  };

  let aliasName = {
    let doc = "The alias name";
    Arg.(
      required & pos(1, some(string), None) & info([], ~docv="NAME", ~doc)
    );
  };

  (
    Term.(const(Commands.alias) $ selectedVersion $ aliasName),
    Term.info(
      "alias",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let default = {
  let doc = "Alias a version as default";
  let man = help_secs;
  let sdocs = Manpage.s_common_options;

  let selectedVersion = {
    let doc = "The version to be aliased as default";
    Arg.(
      required
      & pos(0, some(string), None)
      & info([], ~docv="VERSION", ~doc)
    );
  };

  (
    Term.(const(Commands.default) $ selectedVersion),
    Term.info(
      "default",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let env = {
  let doc = "Show env configurations";
  let sdocs = Manpage.s_common_options;
  let man = help_secs;

  let isFishShell = {
    let doc = "Output an env configuration for fish shell.";
    Arg.(value & flag & info(["fish"], ~doc));
  };

  let shell = {
    open Fnm.System.Shell;
    let doc = "Specifies a specific shell type. If omitted, it will be inferred based on the process tree. $(docv)";
    let shellChoices =
      Arg.enum([("fish", Fish), ("bash", Bash), ("zsh", Zsh)]);
    Arg.(value & opt(some(shellChoices), None) & info(["shell"], ~doc));
  };

  let nodeDistMirror = {
    let doc = "https://nodejs.org/dist mirror";
    Arg.(
      value
      & opt(string, "https://nodejs.org/dist")
      & info(["node-dist-mirror"], ~doc)
    );
  };

  let fnmDir = {
    let doc = "The directory to store internal fnm data";
    Arg.(
      value
      & opt(string, Fnm.Config.FNM_DIR.get())
      & info(["fnm-dir"], ~doc)
    );
  };

  let isMultishell = {
    let doc = "Allow different Node versions for each shell";
    Arg.(value & flag & info(["multi"], ~doc));
  };

  let useOnCd = {
    let doc = "Hook into the shell `cd` and automatically use the specified version for the project";
    Arg.(value & flag & info(["use-on-cd"], ~doc));
  };

  let logLevel = {
    let doc = "The log level of fnm commands, can be 'quiet', 'error' or 'all'";
    Arg.(
      value
      & opt(
          enum([
            ("quiet", Fnm.LogLevel.Quiet),
            ("error", Fnm.LogLevel.Error),
            ("info", Fnm.LogLevel.Info),
            ("all", Fnm.LogLevel.Debug),
            ("debug", Fnm.LogLevel.Debug),
          ]),
          Fnm.Config.FNM_LOGLEVEL.get(),
        )
      & info(["log-level"], ~doc)
    );
  };

  (
    Term.(
      const(Commands.env)
      $ isFishShell
      $ isMultishell
      $ nodeDistMirror
      $ fnmDir
      $ shell
      $ useOnCd
      $ logLevel
    ),
    Term.info(
      "env",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let defaultCmd = {
  let doc = "Manage Node.js installations";
  let sdocs = Manpage.s_common_options;
  let man = help_secs;
  (
    Term.(ret(const(_ => `Help((`Pager, None))) $ const())),
    Term.info(
      "fnm",
      ~envs,
      ~version,
      ~doc,
      ~exits=Term.default_exits,
      ~man,
      ~sdocs,
    ),
  );
};

let _ =
  Term.eval_choice(
    defaultCmd,
    [install, uninstall, use, alias, default, listLocal, listRemote, env],
  )
  |> Term.exit;
