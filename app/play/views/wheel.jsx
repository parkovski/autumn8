import View from './view'

import {css} from '@emotion/css';

const WheelClass = css({
  display: 'inline-block',
  position: 'relative',
  borderWidth: '3px 3px 3px 1px',
  borderStyle: 'solid',
  borderColor: '#000',
  background: 'linear-gradient(#111, #333, #111)',
  width: '27px', // width(3 white keys)/4
  height: '100%',
  '&:not(:first-child)': {
    marginLeft: '27px',
  }
});

const NotchClass = css({
  position: 'relative',
  height: '20%',
  top: '40%',
  background:
    `linear-gradient(to bottom, #222, #333 1px, #111 51%, #0000 51%),
     linear-gradient(to top,    #222, #333 1px, #111 51%, #0000 51%)`,
});

export default class Wheel extends View {
  onCreate() {
    return (
      <div class={WheelClass}>
        <div class={NotchClass}></div>
      </div>
    );
  }
}
