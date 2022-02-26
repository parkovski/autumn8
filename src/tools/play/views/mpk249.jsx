import View from 'swoll/view';
import {css, cx} from '@emotion/css';

import Keyboard from './keyboard';
import Wheel from './wheel';
import PadBank from './padbank';

const FrameClass = css({
  display: 'inline-block',
  padding: '0 1.25rem',
  background:
    `linear-gradient(to right, #111, #222 1rem, #111 1rem 51%, #0000 51%),
     linear-gradient(to left,  #111, #222 1rem, #111 1rem 51%, #0000 51%)`,
});

const TopClass = css({
  position: 'relative',
  borderWidth: '0 2px',
  borderStyle: 'solid',
  borderColor: '#111',
  height: '240px',
  padding: '1rem 0 2rem',
  '& > div': {
    verticalAlign: 'top',
    height: '100%',
  }
});

const WheelPanelClass = css({
  display: 'inline-block',
  padding: '3rem 2px 0 18px', // left align w/ 1st black key.
});

const KFSPanelClass = css({
  display: 'inline-grid',
  width: '350px',
  gridTemplate: '45px 1fr 24px / repeat(8, auto)',
});

const KnobPanelClass = css({
});

const FSBorderClass = css({
  borderRight: '2px solid #411',
});

const FSBorderLeftClass = css({
  borderLeft: '2px solid #411',
});

const FaderPanelClass = cx(FSBorderClass, css({
}));

const SwitchPanelClass = cx(FSBorderClass, css({
  padding: '8px 8px 0',
}));

const SwitchClass = css({
  height: '100%',
  backgroundColor: '#888',
  border: '1px solid #555',
  borderBottomWidth: '2px',
  borderLeftWidth: '2px',
});

const SpacerClass = css({
  display: 'inline-block',
});

const BotClass = css({
  paddingTop: '1rem',
  borderWidth: '0 2px',
  borderStyle: 'solid',
  borderColor: '#111',
  background: `linear-gradient(to bottom, #333, #222 2px, #111 1rem)`,
});

function getKnobs() {
  return Array(8).fill(0).map(() => <div class={KnobPanelClass}></div>);
}

function getFaders() {
  return [<div class={cx(FSBorderLeftClass, FaderPanelClass)}></div>]
    .concat(Array(7).fill(0).map(() => <div class={FaderPanelClass}></div>));
}

function getSwitches() {
  return [
    <div class={cx(FSBorderLeftClass, SwitchPanelClass)}>
      <div class={SwitchClass}/>
    </div>
  ].concat(Array(7).fill(0).map(
      () => (
        <div class={SwitchPanelClass}>
          <div class={SwitchClass}/>
        </div>
      )
  ));
}

export default class MPK249 extends View {
  onCreate() {
    return (
      <div class={FrameClass}>
        <div class={TopClass}>
          <div class={WheelPanelClass}>
            {new Wheel().render()}
            {new Wheel().render()}
          </div>
          <div class={SpacerClass} style="width: 150px"></div>
          {new PadBank().render()}
          <div class={SpacerClass} style="width: 250px"></div>
          <div class={KFSPanelClass}>
            {getKnobs()}
            {getFaders()}
            {getSwitches()}
          </div>
        </div>
        <div class={BotClass}>
          {new Keyboard({start: 'c2', keys: 49}).render()}
        </div>
      </div>
    );
  }
}
