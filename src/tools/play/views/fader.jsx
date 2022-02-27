import View from 'swoll/view';
import {css} from '@emotion/css';

// 6 bold lines, with 5 thin lines in between.
// the middle of the fader matches up with the top and bottom bold lines
// at the max/min.
const FaderClass = css({
  position: 'absolute',
  top: '10%',
  bottom: '10%',
  left: '3px',
  right: '3px',
  borderWidth: '1px',
  borderStyle: 'solid',
  borderColor: '#444',
});

const HandleClass = css({
  position: 'absolute',
  top: '40%',
  height: '20%',
  left: '2px',
  right: '2px',
  border: '1px solid #444',
  background: 'linear-gradient(#000, #333 33% 44%, #eee 44% 56%, #333 56% 67%, #000)',
});

export default class Fader extends View {
  onCreate() {
    return (
      <div class={FaderClass}>
        <div class={HandleClass}></div>
      </div>
    );
  }
}
