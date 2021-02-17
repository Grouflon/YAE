module.exports = {
  cmd: "build.bat",
  name: "all",
  errorMatch:
  [
    "\\n(?<file>.+)\\((?<line>\\d+)\\):\\s+(?<message>error.+)"
  ],
  targets: {
    clean: {
      cmd: "build.bat clean",
      keymap: "ctrl-f5"
    },
    build: {
      cmd: "build.bat build",
      keymap: "f5"
    },
    run: {
      cmd: "build.bat run",
      keymap: "f6"
    },
  }
};
