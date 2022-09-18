import {attach} from './template.html';
import './style.css';

export default class MainElement extends HTMLElement {
  static elementName = 'b2-main';

  constructor() {
    super();
    attach(this);
  }
};
