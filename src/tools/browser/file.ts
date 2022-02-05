import {BrowserController} from './controller';
import {FileLoadEvent} from './events';
import {View} from './lib';

import {css} from '@emotion/css';

export class FileView extends View {
  constructor(el: HTMLElement) {
    super(document.createElement('ul'));
    el.replaceChildren(this.el);
  }

  onLoad(e: FileLoadEvent) {
    const li1 = document.createElement('li');
    const back = document.createElement('a');
    back.innerText = '< back';
    back.classList.add(css({
      cursor: 'pointer',
      color: '#0369A1', /* sky-700 */
      '&:hover': {
        textDecoration: 'underline',
      },
    }));
    back.onclick = () => (e.target as BrowserController).unloadFile();
    li1.appendChild(back);

    const li2 = document.createElement('li');
    li2.innerText = 'name: ' + e.name;

    const li3 = document.createElement('li');
    li3.innerText = 'type: ' + e.contentType;

    const li4 = document.createElement('li');
    li4.innerText = 'size: ' + e.content.byteLength;

    this.el.replaceChildren(li1, li2, li3, li4);
  }
}

export class TextView extends View {
  constructor(el: HTMLElement, text: string) {
    super(document.createElement('pre'));
    this.el.classList.add(css({ color: 'white' }));
    this.el.innerText = text;
    el.replaceChildren(this.el);
  }
}

export class ImageView extends View {
  constructor(el: HTMLElement, data: ArrayBuffer, type: string) {
    super(document.createElement('div'));
    this.el.classList.add(css({
      display: 'grid',
      placeContent: 'center',
    }));
    const img = document.createElement('img');
    const blob = new Blob([new Uint8Array(data)], { type });
    const url = window.URL.createObjectURL(blob);
    img.src = url;
    this.el.appendChild(img);
    el.replaceChildren(this.el);
  }
}
