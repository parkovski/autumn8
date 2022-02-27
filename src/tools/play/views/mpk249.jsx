import View from 'swoll/view';
import {css, cx} from '@emotion/css';

import Menu from 'swoll/menu';

import Keyboard from './keyboard';
import Wheel from './wheel';
import PadBank from './padbank';
import LCD from './lcd';
import Knob from './knob';
import Fader from './fader';
import Switch from './switch';

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
  position: 'relative',
  padding: '5px',
  verticalAlign: 'middle',
  zIndex: 1,
});

const KnobBorderClass = css({
  position: 'absolute',
  float: 'left',
  width: '100%',
  height: '50%',
  top: '25%',
  marginLeft: '-5px',
  borderWidth: '1px 0',
  borderStyle: 'solid',
  borderColor: '#eee',
  zIndex: -1,
});

const KnobBorderLeftClass = cx(KnobBorderClass, css({
  borderTopLeftRadius: '5px',
  borderBottomLeftRadius: '5px',
  borderLeftWidth: '1px',
}));

const KnobBorderRightClass = cx(KnobBorderClass, css({
  borderTopRightRadius: '5px',
  borderBottomRightRadius: '5px',
  borderRightWidth: '1px',
}));

const FSBorderClass = css({
  borderRight: '2px solid #411',
});

const FSBorderLeftClass = css({
  borderLeft: '2px solid #411',
});

const FaderPanelClass = cx(FSBorderClass, css({
  position: 'relative',
}));

const SwitchPanelClass = cx(FSBorderClass, css({
  padding: '10px 8px 0',
}));

const LCDPanelClass = css({
  display: 'inline-block',
  position: 'relative',
  height: '100%',
  width: '250px',
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

function knobBorderClass(i) {
  if (i === 0) { return KnobBorderLeftClass; }
  if (i === 7) { return KnobBorderRightClass; }
  return KnobBorderClass;
}

function getKnobs() {
  return Array(8).fill(0).map((_, i) => (
    <div class={KnobPanelClass}>
      <div class={knobBorderClass(i)}></div>
      {new Knob().render()}
    </div>
  ));
}

function faderPanelClass(i) {
  if (i === 0) { return cx(FSBorderLeftClass, FaderPanelClass); }
  return FaderPanelClass;
}

function getFaders() {
  return Array(8).fill(0).map((_, i) => (
    <div class={faderPanelClass(i)}>
      {new Fader().render()}
    </div>
  ));
}

function switchPanelClass(i) {
  if (i === 0) { return cx(FSBorderLeftClass, SwitchPanelClass); }
  return SwitchPanelClass;
}

function getSwitches() {
  return Array(8).fill(0).map((_, i) => (
    <div class={switchPanelClass(i)}>
      {new Switch().render()}
    </div>
  ));
}

export default class MPK249 extends View {
  cmenu;

  constructor() {
    super();

    this.cmenu = new Menu(
      new Menu.Title('View'),
      new Menu.Item('Keyboard'),
      new Menu.Item('Pitch/Mod Wheels'),
      new Menu.Item('Pad Bank'),
      new Menu.Item('KFS Bank'),
      new Menu.Title('Options'),
      new Menu.Item('Lock Keyboard'),
    );
  }

  onCreate() {
    return (
      <div class={FrameClass} onContextMenu={e => this.cmenu.show(e)}>
        <div class={TopClass}>
          <div class={WheelPanelClass}>
            {new Wheel().render()}
            {new Wheel().render()}
          </div>
          <div class={SpacerClass} style="width: 150px"></div>
          {new PadBank().render()}
          <div class={LCDPanelClass}>
            {new LCD().render()}
          </div>
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
