import View from 'swoll/view';
import {css} from '@emotion/css';

const SwitchClass = css({
  height: '100%',
  backgroundColor: '#888',
  border: '1px solid #555',
  borderBottomWidth: '2px',
  borderLeftWidth: '2px',
});

export default class Switch extends View {
  onCreate() {
    return <div class={SwitchClass}></div>;
  }
}
