import {attach} from './template.html';
import './style.css';
import browser from '../../browser';
import {Directory} from 'app/b2/model';

const icons = {
  directory: 'fa-folder',
  symlink: 'fa-up-right-from-square',
  fifo: 'fa-layer-group',
  socket: 'fa-network-wired',
  file: 'fa-file',
  reparse: 'fa-square-question',
};

export default class MainElement extends HTMLElement {
  static elementName = 'b2-main';

  #main: HTMLDivElement;
  list?: HTMLUListElement;

  constructor() {
    super();
    attach(this);
    this.#main = document.getElementById('main') as HTMLDivElement;
    A8_DEBUG('Main subscribe to cwd');
    browser.cwd.subscribe(this.setCurdir.bind(this));
  }

  setCurdir(cwd: Directory) {
    if (cwd.name.endsWith('/')) {
      cwd.name = cwd.name.substring(0, cwd.name.length - 1);
    }
    A8_DEBUG("Main: setCurdir: " + cwd.name);
    if (!cwd.entries) return;
    if (this.list) {
      this.#main.removeChild(this.list);
    }

    this.list = document.createElement('ul');
    for (let i = 0; i < cwd.entries.length; ++i) {
      const e = cwd.entries[i];
      const li = document.createElement('li');
      const a = document.createElement('a');
      const icon = document.createElement('i');
      li.appendChild(a);
      a.appendChild(icon);
      icon.classList.add('fas', icons[e.kind!]);
      if (e.kind === 'directory') {
        a.onclick = () => browser.go(cwd.name + '/' + e.name);
      } else {
        a.onclick = () => false;
      }
      a.appendChild(document.createTextNode(e.name));
      this.list.appendChild(li);
    }
    this.#main.appendChild(this.list);
  }
};
