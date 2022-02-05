import {ContextMenuView} from './contextmenu';
import {BrowserController} from './controller';
import {DirChangeEvent, DirPopEvent, DirPushEvent} from './events';
import {View} from './lib';

import {css} from '@emotion/css';

export class DirView extends View {
  container;
  contextMenu;

  constructor(el: HTMLElement) {
    super(document.createElement('ul'));
    this.el.classList.add(css({
      listStyleType: 'none',
      margin: 0,
      padding: 0,
    }));
    this.container = el;
    this.contextMenu = new ContextMenuView;
    this.contextMenu.setItems(['Open', 'Open with...', '-', 'Cut', 'Copy',
      'Paste', '-', 'Properties...']);
  }

  show() {
    this.container.replaceChildren(this.el);
  }

  onChange(e: DirChangeEvent | DirPopEvent | DirPushEvent) {
    const clr = e.target as BrowserController;
    const children = e.entries.map(entry => {
      const li = document.createElement('li');
      const link = document.createElement('a');
      const icon = document.createElement('i');
      icon.classList.add('fas', css({ paddingRight: '0.5rem' }));
      link.classList.add(css({
        cursor: 'pointer',
        color: '#FEFCE8', /* yellow-50 */
        '&:hover': {
          textDecoration: 'underline',
        },
      }));
      if (entry.isDir) {
        icon.classList.add('fa-folder', css({ color: '#FEF08A' /* yellow-200 */ }));
        link.onclick = () => clr.enter(entry.name);
      } else {
        icon.classList.add('fa-file', css({ color: '#FB923C' /* orange-400 */ }));
        link.onclick = () => clr.loadFile(entry.name);
      }
      link.oncontextmenu = e => { this.contextMenu.show(e.x, e.y); return false; };
      link.append(icon, entry.name);
      li.appendChild(link);
      return li;
    });
    this.el.replaceChildren.apply(this.el, children);
    //await e.target.renderReady();
    this.show();
  }
}
