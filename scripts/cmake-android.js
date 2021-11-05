#!/usr/bin/env node

const spawnSync = require('child_process').spawnSync;
const path = require('path');
const fs = require('fs');
const os = require('os');

const args = [];

function fixpath(p) {
  if (!p) return;
  if (os.platform() === 'win32') {
    return p.replace(/\\/g, '/');
  }
  return p;
}

function envpath(name) {
  return fixpath(process.env[name]);
}

const ndk = envpath('ANDROID_NDK');
if (!ndk) {
  console.log('You need to set ANDROID_NDK');
  process.exit(1);
}

const arch = process.argv[2];
if (!arch) {
  console.log('usage: node cmake-android <arch> [debug|release] <args>...');
  process.exit(1);
}
if (!~['x64', 'x86', 'arm', 'arm64'].indexOf(arch)) {
  console.log('invalid arch, expected x64|x86|arm|arm64');
  process.exit(1);
}

const abi = {
  x64: 'x86_64',
  x86: 'x86',
  arm: 'armeabi-v7a',
  arm64: 'arm64-v8a',
}[arch];

if (process.env.VCPKG_ROOT) {
  console.log(`found vcpkg in ${process.env.VCPKG_ROOT}`);
  args.push(`-DCMAKE_TOOLCHAIN_FILE=${envpath('VCPKG_ROOT')}/scripts/buildsystems/vcpkg.cmake`);
  args.push(`-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${ndk}/build/cmake/android.toolchain.cmake`);
  args.push(`-DVCPKG_TARGET_TRIPLET=${arch}-android`);
} else {
  args.push(`-DCMAKE_TOOLCHAIN_FILE=${ndk}/build/cmake/android.toolchain.cmake`);
}

args.push(`-DANDROID_ABI=${abi}`);

if (arch === 'arm') {
  args.push('-DANDROID_ARM_NEON=ON');
}

let kind = 'debug';
let i = 3;
if (process.argv[3] === 'release') {
  kind = 'release';
  args.push('-DCMAKE_BUILD_TYPE=Release');
  ++i;
} else if (process.argv[3] === 'debug') {
  args.push('-DCMAKE_BUILD_TYPE=Debug');
  ++i;
} else if (process.argv[3] === 'relwithdebinfo') {
  kind = 'relwithdebinfo';
  args.push('-DCMAKE_BUILD_TYPE=RelWithDebInfo');
  ++i;
} else if (process.argv[3] === 'minsizerel') {
  kind = 'minsizerel';
  args.push('-DCMAKE_BUILD_TYPE=MinSizeRel');
  ++i;
}

for (; i < process.argv.length; ++i) {
  args.push(process.argv[i]);
}

const builddir =
  fixpath(path.join(__dirname, '..', 'build', 'android', kind, arch));

if (!fs.existsSync(builddir)) {
  fs.mkdirSync(builddir, { recursive: true });
}
console.log(`building in ${builddir}`);

args.push('-S', fixpath(path.join(__dirname, '..')));
args.push('-B', builddir);

process.exit(
  spawnSync('cmake', args, { stdio: 'inherit', encoding: 'utf8' }).status
);
