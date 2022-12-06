import {attach} from './template.html';
import './style.css';
import browser from '../../browser';
import {Directory} from 'app/b2/model';

export default class MainElement extends HTMLElement {
  static elementName = 'b2-main';
  list?: HTMLUListElement;

  constructor() {
    super();
    attach(this);
    A8_DEBUG('Main subscribe to cwd');
    browser.cwd.subscribe(this.setCurdir.bind(this));
  }

  setCurdir(cwd: Directory) {
    A8_DEBUG("Main: setCurdir");
    if (!cwd.entries) return;
    if (this.list) {
      this.removeChild(this.list);
    }

    this.list = document.createElement('ul');
    console.log('cwd:',cwd);
    for (let i = 0; i < cwd.entries.length; ++i) {
      const e = cwd.entries[i];
      const li = document.createElement('li');
      if (e.kind === 'directory') {
        const a = document.createElement('a');
        a.innerText = e.name;
        a.onclick = () => browser.go(cwd.name + '/' + e.name);
        li.appendChild(a);
      } else {
        li.innerText = e.name;
      }
      this.list.appendChild(li);
    }
    this.appendChild(this.list);
  }
};
