import {ContextMenuView} from './contextmenu';
import {BrowserController} from './controller';
import {DirChangeEvent, DirPopEvent, DirPushEvent} from './events';
import {View} from './lib';

export class DirView extends View {
  container;
  contextMenu;

  constructor(el: HTMLElement) {
    super(document.createElement('ul'));
    this.container = el;
    this.contextMenu = new ContextMenuView;
    this.contextMenu.setItems(['Open', 'Open with...', '-', 'Cut', 'Copy',
      'Paste', '-', 'Properties...']);
  }

  show() {
    this.container.replaceChildren(this.el);
  }

  onChange(e: DirChangeEvent | DirPopEvent | DirPushEvent) {
    const cx = e.target as BrowserController;
    const children = e.entries.map(entry => {
      const li = document.createElement('li');
      const link = document.createElement('a');
      const icon = document.createElement('i');
      icon.classList.add('fas', 'pr-1');
      link.classList.add('cursor-pointer', 'hover:underline', 'text-yellow-50');
      if (entry.isDir) {
        icon.classList.add('fa-folder', 'text-yellow-200');
        link.onclick = () => cx.enter(entry.name);
      } else {
        icon.classList.add('fa-file', 'text-orange-400');
        link.onclick = () => cx.loadFile(entry.name);
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
