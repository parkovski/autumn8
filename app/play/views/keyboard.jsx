import View from './view';
import {css, cx} from '@emotion/css';

// const keyboardHeightPx = 100;
// const whiteKeyWidthPx = 20;
// const blackKeyWidthPx = 15;
// const blackKeyBottomPx = 8;
const keyboardHeightPx = 180;
const whiteKeyWidthPx = 36;
const blackKeyWidthPx = 24;
const blackKeyBottomPx = 12;

const KeyboardClass = css({
  position: 'relative',
  display: 'flex',
  flexDirection: 'row',
  height: `${keyboardHeightPx}px`,
});

const WhiteKey = css({
  color: 'black',
  verticalAlign: 'bottom',
  position: 'relative',
  flex: `0 0 ${whiteKeyWidthPx}px`,
  backgroundColor: '#efefef',
  //background: 'linear-gradient(to top, #a0a0a0, #efefef 3px)',
  border: '1px solid #282828',
  borderLeftWidth: '0',
  zIndex: 1,
  '&:first-child': {
    borderLeftWidth: '1px',
  },
  '&::after': {
    position: 'absolute',
    top: 0,
    bottom: 0,
    left: 0,
    right: 0,
    display: 'inherit',
    content: '" "',
    borderRight: '1px solid #808080',
    borderBottom: '2px solid #808080',
  },
});

const BlackKey = css({
  color: 'white',
  fontSize: '66%',
  lineHeight: 1,
  verticalAlign: 'bottom',
  position: 'relative',
  flex: `0 0 ${blackKeyWidthPx}px`,
  background: 'linear-gradient(to right, #000, #202020 30% 70%, #000)',
  borderWidth: '0 3px 3px',
  borderStyle: 'solid',
  borderColor: '#101010',
  height: '66%',
  zIndex: 2,
  '&::before': {
    position: 'absolute',
    zIndex: 3,
    top: 0,
    bottom: 0,
    left: 0,
    right: 0,
    display: 'inherit',
    content: '" "',
    border: '1px solid #303030',
    borderTopColor: '#000',
    borderLeftColor: '#181818',
    background: `linear-gradient(to top,
      #202020, #000 ${blackKeyBottomPx}px, #30303080 ${blackKeyBottomPx + 1}px,
      transparent ${blackKeyBottomPx + 2}px)`,
  },
});

const KeyOffset2_3 = css({
  marginLeft: `-${2 * blackKeyWidthPx / 3}px`,
});

const KeyOffset1_2 = css({
  marginLeft: `-${blackKeyWidthPx / 2}px`,
});

const KeyOffset1_3 = css({
  marginLeft: `-${blackKeyWidthPx / 3}px`,
});

const keyinfo = {
  'a':  { next: 'a#', class: cx(WhiteKey, KeyOffset1_2) },
  'a#': { next: 'b',  class: cx(BlackKey, KeyOffset1_3) },
  'b':  { next: 'c',  class: cx(WhiteKey, KeyOffset2_3) },
  'c':  { next: 'c#', class: WhiteKey },
  'c#': { next: 'd',  class: cx(BlackKey, KeyOffset2_3) },
  'd':  { next: 'd#', class: cx(WhiteKey, KeyOffset1_3) },
  'd#': { next: 'e',  class: cx(BlackKey, KeyOffset1_3) },
  'e':  { next: 'f',  class: cx(WhiteKey, KeyOffset2_3) },
  'f':  { next: 'f#', class: WhiteKey },
  'f#': { next: 'g',  class: cx(BlackKey, KeyOffset2_3) },
  'g':  { next: 'g#', class: cx(WhiteKey, KeyOffset1_3) },
  'g#': { next: 'a',  class: cx(BlackKey, KeyOffset1_2) },
};

const WhiteKeyLabelClass = css({
  position: 'absolute',
  bottom: '3px',
  width: '100%',
  textAlign: 'center',
  overflowX: 'hidden',
  wordWrap: 'unset',
  zIndex: 4,
});
const BlackKeyLabelClass = cx(WhiteKeyLabelClass, css({
  marginBottom: `${blackKeyBottomPx}px`,
}));

function nextKey(name, oct, label, keys) {
  let keyclass = keyinfo[name].class;
  if (!keys.length && name[1] !== '#') {
    // Left most key doesn't adjust for black key.
    keyclass = WhiteKey;
  }
  if (label) {
    let labelclass = WhiteKeyLabelClass;
    if (name[1] === '#') {
      labelclass = BlackKeyLabelClass;
    }
    keys.push(<div class={keyclass}><span class={labelclass}>{name+oct}</span></div>);
  } else {
    keys.push(<div class={keyclass}></div>);
  }
  return keyinfo[name].next;
}

function getKeys(start, n) {
  const keys = [];
  const match = /([a-gA-G]#?)(-1|[0-9]|1[0123])/.exec(start);
  if (!match) { return null; }
  let name = match[1].toLowerCase();
  let oct = parseInt(match[2]);
  while (n--) {
    name = nextKey(name, oct, false, keys);
    if (name === 'c') {
      ++oct;
    }
  }
  return keys;
}

export default class Keyboard extends View {
  onCreate() {
    return (
      <div class={KeyboardClass}>
        {getKeys(this.props.start, this.props.keys)}
      </div>
    );
  }
}
