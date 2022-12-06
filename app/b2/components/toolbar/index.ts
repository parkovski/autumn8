import {attach} from './template.html';
import './style.css';
import browser from '../../browser';
import {Directory} from 'app/b2/model';
import ToggleButton from '../../views/toggle';

export default class ToolbarElement extends HTMLElement {
  static elementName = 'b2-toolbar';

  dev: ToggleButton;
  curdir: HTMLSpanElement;

  constructor() {
    super();
    attach(this);
    this.dev = new ToggleButton(this.querySelector('#dev')!);
    this.curdir = this.querySelector('#curdir')!;

    this.dev.element.addEventListener('click', this.toggleDevTools.bind(this));

    A8_DEBUG('Toolbar subscribe to cwd');
    browser.cwd.subscribe(this.setCurdir.bind(this));
  }

  async toggleDevTools() {
    this.dev.state = await browser.toggleDevTools();
  }

  setCurdir(cwd: Directory) {
    let children: HTMLElement[] = [];
    let path = cwd.name.split('/');
    if (path[0] === '') path = path.slice(1);

    const link = document.createElement('a');
    const icon = document.createElement('i');
    icon.classList.add('fas', 'fa-desktop');
    link.appendChild(icon);
    children.push(link);

    let fullpath = '';
    for (const p of path) {
      const slash = document.createElement('span');
      slash.innerText = '/';
      children.push(slash);

      const link = document.createElement('a');
      link.innerText = p;
      fullpath += '/' + p;
      link.onclick = (path => () => browser.go(path))(fullpath);
      children.push(link);
    }

    this.curdir.replaceChildren(...children);
  }
}
