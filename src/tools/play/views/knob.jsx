import View from 'swoll/view';
import {css} from '@emotion/css';

const KnobClass = css({
  height: '100%',
  border: '2px solid #333',
  borderRadius: '999px',
  background: 'radial-gradient(#333, #222 50%, #111 50%, #000)',
});

export default class Knob extends View {
  onCreate() {
    return <div class={KnobClass}></div>;
  }
}
