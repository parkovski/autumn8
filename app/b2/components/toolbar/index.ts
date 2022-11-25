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
    function add(dir: Directory, depth: number) {
      const link = document.createElement('a');
      if (dir.parent) {
        add(dir.parent, depth + 1);
        const slash = document.createElement('span');
        slash.innerText = '/';
        children.push(slash);
        link.innerText = dir.name;
      } else {
        const icon = document.createElement('i');
        icon.classList.add('fas', 'fa-desktop');
        link.appendChild(icon);
      }
      link.dataset.depth = depth.toString();
      link.onclick = () => false;
      children.push(link);
    }
    add(cwd, 0);
    this.curdir.replaceChildren(...children);
  }
}
