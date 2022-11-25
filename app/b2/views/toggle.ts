//import {attach} from './template.html';
import View from '../view';

export default class ToggleButton extends View<HTMLButtonElement> {
  #state = false;
  #classes?: string[];

  constructor(element: HTMLButtonElement) {
    super(element);
  }

  get state() { return this.#state; }
  set state(value: boolean) {
    if (value === this.#state) return;
    const element = this.element;
    let classes = this.#classes;
    if (classes) {
      element.classList.remove(...classes);
    }
    classes = element.getAttribute(`data-class-${value ? 'on' : 'off'}`)?.split(/ +/);
    if (classes) {
      element.classList.add(...classes);
    }
    A8_DEBUG(`toggle ${value}`, classes);
    this.#classes = classes;
    this.#state = value;
  }

  toggle() {
    this.state = !this.state;
  }
}
