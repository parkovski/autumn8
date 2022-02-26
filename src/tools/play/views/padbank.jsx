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
  fontSize: '75%',
  paddingLeft: '2px',
  border: '1px solid #888',
  borderBottomWidth: '3px',
});

export default class PadBank extends View {
  onCreate() {
    const pads = [];
    for (let i = 0; i < 4; ++i) {
      for (let j = 0; j < 4; ++j) {
        pads.push(<div class={PadClass}>{1 + j + 4*(3 - i)}</div>);
      }
    }
    return <div class={BankClass}>{pads}</div>;
  }
}
