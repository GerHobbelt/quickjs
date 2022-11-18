const core = require("./core.ninja");

const deps_host = [
  ...core.deps_host,
  builddir("inspect.host.o"),
  builddir("quickjs-libc.host.o"),
];

const deps_target = [
  ...core.deps_target,
  builddir("inspect.target.o"),
  builddir("quickjs-libc.target.o"),
];

build({
  output: builddir("quickjs-full.host.a"),
  rule: "create_host_archive",
  inputs: deps_host,
});

build({
  output: builddir("quickjs-full.target.a"),
  rule: "create_target_archive",
  inputs: deps_target,
});

module.exports = { deps_host, deps_target };
