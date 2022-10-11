import {attach} from './template.html';
import './style.css';
import browser from '../../browser';
import {Directory} from 'app/b2/model';

export default class ToolbarElement extends HTMLElement {
  static elementName = 'b2-toolbar';

  dev: HTMLButtonElement;
  curdir: HTMLSpanElement;

  constructor() {
    super();
    attach(this);
    this.dev = this.querySelector('#dev')!;
    this.curdir = this.querySelector('#curdir')!;

    this.dev.addEventListener('click', this.toggleDevTools.bind(this));

    A8_DEBUG('Toolbar subscribe to cwd');
    browser.cwd.subscribe(this.setCurdir.bind(this));
  }

  async toggleDevTools() {
    if (await browser.toggleDevTools()) {
      this.dev.style.color = '#333';
      this.dev.style.backgroundColor = '#FFBB3D';
    } else {
      this.dev.style.color = '#f0f0f0';
      this.dev.style.backgroundColor = '';
    }
  }

  setCurdir(cwd: Directory) {
    let children: HTMLElement[] = [];
    function add(dir: Directory) {
      const link = document.createElement('a');
      if (dir.parent) {
        add(dir.parent);
        const slash = document.createElement('span');
        slash.innerText = '/';
        children.push(slash);
        link.innerText = dir.name;
      } else {
        const icon = document.createElement('i');
        icon.classList.add('fas', 'fa-desktop');
        link.appendChild(icon);
      }
      link.onclick = () => false;
      children.push(link);
    }
    add(cwd);
    this.curdir.replaceChildren(...children);
  }
}
