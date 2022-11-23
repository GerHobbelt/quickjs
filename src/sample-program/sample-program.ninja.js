const { deps_target } = require("../archives/full.ninja");

const sum_c = build({
  output: builddir("sum.c"),
  rule: "qjsc",
  inputs: [rel("sum.js")],
  implicitInputs: [builddir("qjsc.host")],
  ruleVariables: {
    qjsc_args: `-e -m`,
  },
});

const sum_target_o = build({
  output: builddir("sum.target.o"),
  rule: "cc_target",
  inputs: [sum_c],
});

build({
  output: builddir("sum.target"),
  rule: "link_target",
  inputs: [sum_target_o, ...deps_target],
});
