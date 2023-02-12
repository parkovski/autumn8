const fs = require('fs');
const os = require('os');
const spawnSync = require('child_process').spawnSync;
const existsSync = fs.existsSync;
const rm = fs.unlinkSync;

let convertExe = 'convert';
let inkscapeExe = 'inkscape';
if (os.platform() == 'win32') {
  // Windows obnoxiously has a system program called convert.exe.
  convertExe = 'imconvert.exe';
  if (fs.existsSync('C:\\Program Files\\Inkscape\\bin\\inkscape.com')) {
    inkscapeExe = 'C:\\Program Files\\Inkscape\\bin\\inkscape.com';
  }
  // Otherwise just hope it's in the path.
}

function colorstr(str, nr) {
  if (process.stdout.isTTY) {
    return `\x1b[${nr}m${str}\x1b[m`;
  } else {
    return str;
  }
}

const argv = process.argv.slice(2);
function getarg(name, defaultValue) {
  const index = argv.findIndex(a => a == name);
  if (typeof defaultValue === 'undefined') {
    return index != -1;
  }
  if (index == -1 || !argv[index + 1]) { return defaultValue; }
  return argv[index + 1];
}

if (getarg('-h') || getarg('--help')) {
  console.log('Args: -i <input>, -o <output>, --keep = save temps');
  console.log('Requires inkscape for svg conversion, imagemagick convert for png & ico');
  process.exit();
}

const infile = getarg('-i', `${__dirname}/../res/Autumn8.svg`);
const outfile = getarg('-o', infile.replace(/\.[^.\/\\]+$/, '.ico'));
const keepIntermediates = getarg('--keep');
const sizes = [16, 24, 32, 48, 64, 128, 256];

function sh(command) {
  const args =
    Array.prototype.slice.call(arguments, 1)
    .reduce((a, v) => a.concat(v), []);
  console.log(colorstr(command, 32) + ' ' + args.join(' '));
  const result = spawnSync(command, args);
  if (result.error) {
    console.log(colorstr(result.error.message, 31));
    process.exit(1);
  }
  if (result.status != 0) {
    console.log(colorstr(result.stderr.toString(), 31));
    process.exit(result.status);
  }
}

/** @param {string} old */
function rename(old, new_) {
  const slash = os.platform() == 'win32' ? old.lastIndexOf(/\/\\/) : old.lastIndexOf('/');
  if (~slash) {
    return old.substring(0, slash + 1) + new_;
  } else {
    return new_;
  }
}

function convert() {
  for (const size of sizes) {
    const png = rename(outfile, `size${size}.png`);
    if (existsSync(png)) {
      console.log(`skipping existing file ${png}`);
    } else {
      const stuff = { $S: size, $I: infile, $O: png, $X: `${size}x${size}` };
      const args = Array.prototype.map.call(arguments, s => stuff[s] || s);
      sh.apply(null, args);
    }
  }
}

if (/\.svg$/.test(infile)) {
  convert(inkscapeExe, '-z', '-o', '$O', '-w', '$S', '-h', '$S', '$I');
} else {
  convert(convertExe, '$I', '-resize', '$X', '$O');
}

const names = sizes.map(s => rename(outfile, `size${s}.png`));
//sh(convertExe, names, '-colors', 256, outfile); <- Messes up transparency
sh(convertExe, names, outfile);
if (!keepIntermediates) {
  console.log(colorstr('rm ', 32) + names.join(' '));
  names.map(rm);
}
