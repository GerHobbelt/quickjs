#!/usr/bin/env node
const path = require("path");
const child_process = require("child_process");
const { identifyCurrentPlatform } = require("..");

module.exports = function runBinary(name) {
  const platform = identifyCurrentPlatform();

  let bin = path.resolve(__dirname, "..", "..", "build", platform, "bin", name);
  if (process.platform === "win32") {
    bin += ".exe";
  }

  const argv = process.argv.slice(2);

  child_process.spawn(bin, argv, { stdio: "inherit" }).on("exit", process.exit);
};