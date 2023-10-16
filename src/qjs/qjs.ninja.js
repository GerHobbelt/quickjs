const qjs_target_o = build({
  output: builddir("intermediate/qjs.target.o"),
  rule: "cc_target",
  inputs: [rel("qjs.c")],
});

build({
  output: builddir("bin/qjs$PROGRAM_SUFFIX"),
  rule: "link_target",
  inputs: [
    qjs_target_o,
    builddir("intermediate/qjscalc.target.o"),
    builddir("intermediate/repl.target.o"),
    builddir("intermediate/quickjs-libcontext.target.o"),
    builddir("intermediate/quickjs-libbytecode.target.o"),
    builddir("intermediate/quickjs-libdl.target.o"),
    builddir("intermediate/quickjs-full.target.a"),
  ],
});
