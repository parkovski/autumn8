import View from 'swoll/view';
import Toolbar from './toolbar';
import Keyboard from './keyboard';

import {css} from '@emotion/css';

const MainClass = css({
  position: 'absolute',
  left: 0,
  top: 0,
  right: 0,
  bottom: 0,
});
export default class MainView extends View {
  onCreate() {
    return (
      <div class={MainClass}>
        {new Toolbar().render()}
        {new Keyboard({start: 'c2', keys: 49}).render()}
      </div>
    );
  }
}
