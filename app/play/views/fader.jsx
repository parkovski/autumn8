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
  zIndex: 1,
});

const HandleClass = css({
  position: 'absolute',
  top: '80%',
  height: '20%',
  left: '2px',
  right: '2px',
  border: '1px solid #333',
  background: 'linear-gradient(#000, #333 33% 44%, #eee 44% 56%, #333 56% 67%, #000)',
});

const TrackClass = css({
  position: 'absolute',
  top: 0,
  bottom: 0,
  left: '50%',
  width: '6px',
  marginLeft: '-3px',
  border: '2px solid #333',
  borderRadius: '3px',
  backgroundColor: '#000',
  zIndex: -1,
});

export default class Fader extends View {
  onCreate() {
    return (
      <div class={FaderClass}>
        <div class={TrackClass}></div>
        <div class={HandleClass}></div>
      </div>
    );
  }
}
