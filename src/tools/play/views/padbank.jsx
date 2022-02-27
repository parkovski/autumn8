import View from 'swoll/view';
import {css} from '@emotion/css';

const BankClass = css({
  display: 'inline-grid',
  width: '192px',
  gridTemplate: 'repeat(4, auto) / repeat(4, auto)',
  gap: '5px',
  padding: '5px',
  border: '1px solid #eee',
  borderRadius: '5px',
});

const PadClass = css({
  color: '#eee',
  background: 'radial-gradient(#000, #111 67%, #222)',
  fontSize: '75%',
  paddingLeft: '2px',
  borderWidth: '1px 2px 4px 1px',
  borderStyle: 'solid',
});

const MPKPadColors = {
  off: '#555',
  red: '#e70a0a',
  orange: '#e67300',
  amber: '#f2a60c',
  yellow: '#f2f200',
  green: '#0be61e',
  greenblue: '#0bd994',
  aqua: '#159ee7',
  ltblue: '#0a55f4',
  blue: '#0a3ee7',
  purple: '#642db3',
  pink: '#be36d9',
  hotpink: '#d92bbc',
  ltpurple: '#906cd9',
  ltgreen: '#8fbf9f',
  ltpink: '#d998d9',
  grey: '#beb8cc',
};

const PadColors = [
  'aqua', 'blue', 'amber', 'yellow',
  'greenblue', 'green', 'orange', 'amber',
  'aqua', 'ltblue', 'blue', 'purple',
  'red', 'hotpink', 'pink', 'ltpink',
];

export default class PadBank extends View {
  onCreate() {
    const pads = [];
    for (let i = 0; i < 4; ++i) {
      for (let j = 0; j < 4; ++j) {
        const padnr = 4*(3 - i) + j;
        const pad = <div class={PadClass}>{1 + padnr}</div>;
        pad.style.borderColor = MPKPadColors[PadColors[padnr]];
        pads.push(pad);
      }
    }
    return <div class={BankClass}>{pads}</div>;
  }
}
