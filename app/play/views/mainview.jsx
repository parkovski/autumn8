import View from './view';
import Toolbar from './toolbar';
//import Keyboard from './keyboard';
import MPK249 from './mpk249';

import LogView from './log';

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
        {new MPK249().render()}
        {new LogView().render()}
      </div>
    );
  }
}
