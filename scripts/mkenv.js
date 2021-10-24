#!/usr/bin/env node

const os = require('os');
const path = require('path');
const fs = require('fs');
const spawnSync = require('child_process').spawnSync;

class ShError extends Error {
  constructor(command, status, output, error) {
    super(`${command[0]} returned ${status}`);
    this.command = Array.prototype.slice.call(command);
    this.status = status;
    this.output = output;
    this.error = error;
  }
}

function sh() {
  const s = spawnSync(arguments[0], Array.prototype.slice.call(arguments, 1),
    { encoding: 'utf8', stdio: 'pipe' });
  if (s.status != 0) {
    throw new ShError(arguments, s.status, s.output, s.error);
  }
  return s.output.join('\n').trim();
}

function pathRemoveDups(p) {
  p = p.split(path.delimiter);
  let s = new Set;
  let a = [];
  p.forEach(x => {
    if (!s.has(x)) {
      s.add(x);
      a.push(x);
    }
  });
  return a.join(path.delimiter);
}

function removeDups(env) {
  return env
    .split(/\r?\n/g)
    .filter(ln => {
      const eq = ln.indexOf('=');
      if (eq < 1) {
        return false;
      }
      const key = ln.substring(0, eq);
      let value = ln.substring(eq + 1);
      if ((os.platform() === 'win32' && key === 'Path') || key === 'PATH') {
        value = pathRemoveDups(value);
      }
      return process.env[key] != value;
    })
    .join('\n');
}

const builders = {
  help(name) {
    if (name && builders[name]) {
      builders[name]('help');
    } else {
      console.log('usage: mkenv <toolkit> <args>...');
      console.log(`toolkits: ${Object.keys(builders).filter(n => n !== 'help').join(', ')}`);
    }
  },

  msvc(arch) {
    if (!arch) {
      arch = os.arch();
    }
    if (arch === 'help') {
      console.log('usage: node mkenv msvc <arch>');
      return;
    }
    const vswhere = path.join(
      sh('pwsh.exe', '-c',
        'Get-ItemPropertyValue ' +
        'HKLM:\\SOFTWARE\\Microsoft\\VisualStudio\\Setup ' +
        'SharedInstallationPath'),
      '..',
      'Installer',
      'vswhere.exe'
    );
    const vspath = sh(vswhere, '-latest', '-property', 'installationPath');
    const vsdevcmd = path.join(vspath, 'Common7', 'Tools', 'VsDevCmd.bat');
    const vars = sh('cmd.exe', '/Q', '/C',
      'set', 'VSCMD_SKIP_SENDTELEMETRY=1',
      '&', vsdevcmd, '-no_logo', `-host_arch=${os.arch()}`, `-arch=${arch}`,
      '&', 'set');
    return removeDups(vars);
  },

  android() {
    const home = {
      android: process.env.ANDROID_HOME,
      ndk: process.env.ANDROID_NDK,
      java: process.env.JAVA_HOME,
    };
    const paths = {
      'platform-tools': [['android', 'platform-tools']],
      'build-tools': [['android', 'build-tools', '$SDKVER']],
      'cmdline-tools': [['android', 'cmdline-tools/latest/bin']],
      'tools': [['android', 'tools'], ['android', 'tools/bin']],
      'emulator': [['android', 'emulator'], ['android', 'emulator/bin64'], ['android', 'emulator/qemu', '$HOSTARCH']],
      'ndkroot': [['ndk', 'ndk', '$NDKVER']],
      'prebuilt': [['ndk', 'ndk', '$NDKVER', 'prebuilt', '$HOSTARCH', 'bin']],
      'shader-tools': [['ndk', 'ndk', '$NDKVER', 'shader-tools', '$HOSTARCH']],
      'llvm': [['ndk', 'ndk', '$NDKVER', 'toolchains/llvm/prebuilt', '$HOSTARCH', 'bin']],
      'renderscript': [['ndk', 'ndk', '$NDKVER', 'toolchains/renderscript/prebuilt', '$HOSTARCH', 'bin']],
      'java': [['java', 'bin']],
    };
    const defaults = {
      sdk: ['platform-tools', 'cmdline-tools'],
      ndk: ['ndk', 'llvm'],
      java: ['java'],
    };
    const vars = {
      SDKVER: null,
      NDKVER: null,
      HOSTARCH: null,
    };

    if (arguments[0] === 'help' || arguments.length === 0) {
      console.log('usage: node mkenv android [VAR=VALUE]... [components]...');
      console.log(`vars: ${Object.keys(vars).join(', ')}`);
      console.log(`component sets: ${Object.keys(defaults).join(', ')}`);
      console.log(`components: ${Object.keys(paths).join(', ')}`);
      if (arguments.length === 0) {
        process.exit(1);
      }
      return;
    }

    const components = [];
    let newpath = '';
    const append = (parts) => {
      parts[0] = home[parts[0]];
      newpath += path.join.apply(null, parts);
      newpath += path.delimiter;
    };

    for (let i = 0; i < arguments.length; ++i) {
      const arg = arguments[i];
      const eq = arg.indexOf('=');
      if (eq > 0) {
        const name = arg.substring(0, eq);
        if (!vars.hasOwnProperty(name)) {
          throw 'Unknown variable ' + name;
        }
        vars[name] = arg.substring(eq + 1);
      } else if (defaults.hasOwnProperty(arg)) {
        defaults[arg].forEach(c => {
          if (!~components.indexOf(c)) {
            components.push(c);
          }
        });
      } else if (paths.hasOwnProperty(arg)) {
        if (!~components.indexOf(arg)) {
          components.push(arg);
        }
      } else {
        throw 'Unknown component ' + arg;
      }
    }

    for (let cx of components) {
      const lists = paths[cx];
      for (let list of lists) {
        list = list.slice();
        for (let i = 0; i < list.length; ++i) {
          if (list[i][0] == '$' && list[i].substring(1) in vars) {
            const name = list[i].substring(1);
            if (vars[name]) {
              list[i] = vars[name];
            } else {
              throw 'Defaults not yet implemented: ' + name;
            }
          }
        }
        append(list);
      }
    }

    newpath += process.env.PATH;
    newpath = pathRemoveDups(newpath);
    if (os.platform() === 'win32') {
      return 'Path=' + newpath;
    } else {
      return 'PATH=' + newpath;
    }
  },
};

const builder = builders[process.argv[2]];
if (!builder) {
  builders.help();
  process.exit(1);
}

try {
  console.log(builder.apply(null, process.argv.slice(3)));
} catch (e) {
  if (e instanceof Error) {
    if (e.stack) {
      console.log(e.stack);
    } else {
      console.log(`\x1b[31mError:\x1b[m ${e.message}`);
    }
    console.log(`x1b[36mfile:\x1b[m ${e.fileName}`)
    if (e instanceof ShError) {
      console.log(`\x1b[32mcommand:\x1b[m ${e.command.join(' ')}`);
      if (e.output) {
        console.log(`\x1b[33mstdout:\x1b[m ${e.output}`);
      }
      if (e.error) {
        console.log(`\x1b[31mstderr:\x1b[m ${e.error}`);
      }
      process.exit(e.status);
    }
  } else {
    console.log(`\x1b[31mError:\x1b[m ${e}`);
  }
  process.exit(1);
}