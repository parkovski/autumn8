const fs = require('fs');
const spawnSync = require('child_process').spawnSync;
const existsSync = fs.existsSync;
const rm = fs.unlinkSync;

const argv = process.argv.slice(2);
function getarg(name, defaultValue) {
  const index = argv.findIndex(a => a == name);
  if (index == -1 || !argv[index + 1]) { return defaultValue; }
  return argv[index + 1];
}

if (getarg('-h') || getarg('--help')) {
  console.log('Args: -i <input>, -o <output>, --keep = save temps');
  console.log('Requires inkscape for svg conversion, imagemagick convert for png & ico');
}

const infile = getarg('-i', `${__dirname}/../res/Autumn8.svg`);
const outfile = getarg('-o', infile.replace(/\.[^.\/\\]+$/, '.ico'));
const keepIntermediates = getarg('--keep', false);
const sizes = [16, 24, 32, 48, 64, 128, 256];

function sh(command) {
  const args =
    Array.prototype.slice.call(arguments, 1)
    .reduce((a, v) => a.concat(v), []);
  console.log('\033[92m' + command + ' ' + args.join(' ') + '\033[m');
  const result = spawnSync(command, args);
  if (result.error) {
    console.error('\033[31m' + result.error.message + '\033[m');
    process.exit(1);
  }
  if (result.status != 0) {
    console.error('\033[31m' + result.stderr.toString() + '\033[m');
    process.exit(result.status);
  }
}

function convert() {
  for (const size of sizes) {
    const png = `${__dirname}/size${size}.png`;
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
  convert('inkscape', '-z', '-e', '$O', '-w', '$S', '-h', '$S', '$I');
} else {
  convert('convert', '$I', '-resize', '$X', '$O');
}

const names = sizes.map(s => `size${s}.png`);
sh('convert', names, '-colors', 256, outfile);
if (!keepIntermediates) {
  console.log('\033[92mrm ' + names.join(' ') + '\033[m');
  names.map(rm);
}
