import View from './view';
import {css} from '@emotion/css';

const SwitchClass = css({
  height: '100%',
  border: '1px solid #555',
  borderBottomWidth: '2px',
  borderLeftWidth: '2px',
});

const SwitchOffClass = css({
  background: 'linear-gradient(#888, #aaa, #888)',
});

const SwitchOnClass = css({
  background: 'radial-gradient(#fe9a24 25%, #ce8c2c)',
  borderColor: '#c1751c',
});

export default class Switch extends View {
  #state = false;

  onCreate() {
    const el = <div class={SwitchClass} onClick={() => this.toggle()}></div>;
    el.classList.add(SwitchOffClass);
    return el;
  }

  getState() {
    return this.#state;
  }

  setState(state) {
    this.#state = state;
    if (state) {
      this.el.classList.replace(SwitchOffClass, SwitchOnClass);
    } else {
      this.el.classList.replace(SwitchOnClass, SwitchOffClass);
    }
  }

  toggle() {
    this.setState(!this.#state);
  }
}
