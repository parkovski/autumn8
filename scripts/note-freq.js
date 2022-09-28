#!/usr/bin/env node

console.log('A4 = 440Hz');

function parseNote(note) {
  let num = {
    a: 21,
    b: 23,
    c: 12,
    d: 14,
    e: 16,
    f: 17,
    g: 19,
  }[note[0].toLowerCase()];

  if (typeof num === 'undefined') {
    throw `Not a note: '${note[0]}'.`;
  }

  let octave;
  const setOctave = i => {
    if (note[i] === '-') {
      if (note.length !== i + 2 || note[i + 1] !== '1') {
        throw '-1 is the lowest octave';
      }
      octave = -1;
    } else if (note.length !== i + 1) {
      throw 'Octaves go up to 9';
    } else {
      octave = +note[i];
      if (isNaN(octave)) {
        throw `Octave not a number: '${note[i]}'`;
      }
    }
  };

  if (note[1] == '#') {
    ++num;
    setOctave(2);
  } else if (note[1] == 'b') {
    --num;
    setOctave(2);
  } else {
    setOctave(1);
  }

  if (octave == -1 && num < 12) {
    throw 'C-1 is the lowest note';
  }

  num += 12 * octave;
  if (num > 127) {
    throw 'G9 is the highest note';
  }

  return num;
}

function toHz(num) {
  // A4 = 69 = 440Hz.
  return 440 * 2 ** ((num - 69) / 12)
}

function shoopDaWhoop(note) {
  if (note) {
    try {
      const num = parseNote(note);
      process.stdout.write(`MIDI note number: ${num} (0x${num.toString(16)})\n`);
      process.stdout.write(`Frequency: ${toHz(num).toFixed(3)} Hz.\n`);
    } catch (e) {
      process.stderr.write(`Error: ${e.toString()}\n`);
    }
  }
  process.stdout.write('Note? ');
}
process.stdin.on('data', data => {
  shoopDaWhoop(data.toString().trim());
});
shoopDaWhoop();
process.stdin.resume();
