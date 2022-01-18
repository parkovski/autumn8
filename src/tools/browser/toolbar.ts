import {ContextMenuView} from './contextmenu';
import {BrowserController} from './controller';
import {DevToolsVisibilityChangedEvent, DirChangeEvent, DirPopEvent, DirPushEvent, HistoryEvent} from './events';
import {View} from './lib';

export class DevButtonView extends View {
  classesOn;
  classesOff;

  constructor(el: HTMLElement, cx: BrowserController) {
    super(el);
    el.onclick = cx.toggleDevTools.bind(cx);
    this.classesOn = el.getAttribute('data-class-on')!.split(/ +/);
    this.classesOff = el.getAttribute('data-class-off')!.split(/ +/);
    cx.subscribe(DevToolsVisibilityChangedEvent, this.onVisibilityChanged.bind(this));
  }

  onVisibilityChanged(e: DevToolsVisibilityChangedEvent) {
    if (e.visible) {
      this.el.classList.remove.apply(this.el.classList, this.classesOff);
      this.el.classList.add.apply(this.el.classList, this.classesOn);
    } else {
      this.el.classList.remove.apply(this.el.classList, this.classesOn);
      this.el.classList.add.apply(this.el.classList, this.classesOff);
    }
  }
}

class HistoryButtonView extends View {
  history: string[] = [];
  contextMenu;

  constructor(el: HTMLElement, cx: BrowserController, private direction: 'forward' | 'back') {
    super(el);
    this.direction = direction;
    this.contextMenu = new ContextMenuView();
    el.onclick = cx[direction].bind(cx, 0);
    el.oncontextmenu = cx.getHistory.bind(cx, direction);
    cx.subscribe(HistoryEvent, this.onHistoryUpdated.bind(this));
  }

  showMenu() {
    if (this.history.length === 0) {
      return;
    }
    this.contextMenu.show(this.el, this.history);
  }

  onHistoryUpdated(e: HistoryEvent) {
    if (e.direction !== this.direction) {
      return;
    }
    this.history = e.list;
    this.showMenu();
  }
}

export class BackButtonView extends HistoryButtonView {
  constructor(el: HTMLElement, cx: BrowserController) {
    super(el, cx, 'back');
  }
}

export class ForwardButtonView extends HistoryButtonView {
  constructor(el: HTMLElement, cx: BrowserController) {
    super(el, cx, 'forward');
  }
}

export class UpButtonView extends View {
  constructor(el: HTMLElement, cx: BrowserController) {
    super(el);
    el.onclick = cx.up.bind(cx);
  }
}

export class DirNameView extends View {
  path: string[] = [];

  constructor(el: HTMLElement, cx: BrowserController) {
    super(el);
    cx.subscribe(DirPopEvent, this.onGoUp.bind(this));
    cx.subscribe(DirPushEvent, this.onEnter.bind(this));
    cx.subscribe(DirChangeEvent, this.onCd.bind(this));
  }

  makePart(cx: BrowserController, name: string, path: string) {
    const a = document.createElement('a');
    a.innerText = name;
    a.classList.add('cursor-pointer', 'text-white', 'hover:underline');
    a.onclick = () => cx.cd(path);
    return a;
  }

  onGoUp() {
    this.path.pop();
    this.el.children.item(this.el.children.length - 1)!.remove();
  }

  onEnter(e: DirPushEvent) {
    let subdir = e.name + '/';
    this.path.push(e.name);
    this.el.appendChild(this.makePart(e.target as BrowserController, subdir, this.path.join('/')));
  }

  onCd(e: DirChangeEvent) {
    this.path = e.path.split('/');
    let currentPath = '';
    this.el.replaceChildren.apply(this.el, this.path.map(p => {
      p += '/';
      currentPath += p;
      return this.makePart(e.target as BrowserController, p, currentPath);
    }));
  }
}
