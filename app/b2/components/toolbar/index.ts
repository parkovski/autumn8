import {attach} from './template.html';
import './style.css';

export default class ToolbarElement extends HTMLElement {
  static elementName = 'b2-toolbar';

  constructor() {
    super();
    attach(this);
  }
}
