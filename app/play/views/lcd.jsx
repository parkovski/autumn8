import View from 'swoll/view';
import {css} from '@emotion/css';

const LCDClass = css({
  display: 'block',
  position: 'relative',
  color: '#ddd',
  backgroundColor: '#38f',
  fontFamily: 'monospace',
  fontWeight: 'bold',
  marginLeft: '30px',
  right: '15px',
  height: '10em',
  padding: '5px',
  verticalAlign: 'top',
  textAlign: 'left',
  border: '10px solid #000',
  borderTopWidth: '15px',
  borderBottomWidth: '40px',
  borderRadius: '10px',
});

export default class LCD extends View {
  onCreate() {
    return <div class={LCDClass}>Hi there :). I'm hungry for music! Play with me!</div>;
  }
}
