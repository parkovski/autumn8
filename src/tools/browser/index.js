class ModelReadyEvent extends Event {
  static TYPE = 'model-ready';

  constructor() {
    super(ModelReadyEvent.TYPE);
  }
}

class DirChangeEvent extends Event {
  static TYPE = 'dir-change';

  /** @type {string} */
  path;
  /** @type {{name: string, isDir: boolean}[]} */
  entries;

  constructor(path, entries) {
    super(DirChangeEvent.TYPE);
    this.path = path;
    this.entries = entries;
  }
}

class DirPushEvent extends Event {
  static TYPE = 'dir-push';

  /** @type {string} */
  name;
  /** @type {{name: string, isDir: boolean}[]} */
  entries;

  constructor(name, entries) {
    super(DirPushEvent.TYPE);
    this.name = name;
    this.entries = entries;
  }
}

class DirPopEvent extends Event {
  static TYPE = 'dir-pop';

  /** @type {{name: string, isDir: boolean}[]} */
  entries;

  constructor(entries) {
    super(DirPopEvent.TYPE);
    this.entries = entries;
  }
}

class FileLoadEvent extends Event {
  static TYPE = 'file-load';

  name;
  contentType;
  content;

  constructor(name, contentType, content) {
    super(FileLoadEvent.TYPE);
    this.name = name;
    this.contentType = contentType;
    this.content = content;
  }
}

class FileUnloadEvent extends Event {
  static TYPE = 'file-unload';

  constructor() {
    super(FileUnloadEvent.TYPE);
  }
}

class DevToolsVisibilityChangedEvent extends Event {
  static TYPE = 'dev-tools-vis-change';

  /** @type {boolean} */
  visible;

  constructor(visible) {
    super(DevToolsVisibilityChangedEvent.TYPE);
    this.visible = visible;
  }
}

class ControllerBase extends EventTarget {
  /** @type {(e: Event) => void} */
  _schedule;

  constructor() {
    super();

    let pendingEvents = [];

    const dispatchEvent = this.dispatchEvent.bind(this);

    const dispatch = (function() {
      const events = pendingEvents;
      pendingEvents = [];
      events.forEach(dispatchEvent)
    }).bind(this);

    this._schedule = function(e) {
      pendingEvents.push(e);
      if (pendingEvents.length === 1) {
        window.requestAnimationFrame(dispatch);
      }
    };
  }

  emit(E) {
    const args = [null].concat(Array.prototype.slice.call(arguments, 1));
    this._schedule(new (Function.prototype.bind.apply(E, args)));
  }

  emitNow(E) {
    const args = [null].concat(Array.prototype.slice.call(arguments, 1));
    this.dispatchEvent(new (Function.prototype.bind.apply(E, args)));
  }

  subscribe(E, callback) {
    this.addEventListener(E.TYPE, callback);
    return [E.TYPE, callback];
  }

  unsubscribe(E, callback) {
    if (Array.isArray(E)) {
      this.removeEventListener.apply(this, E);
    } else {
      this.removeEventListener(E.TYPE, callback);
    }
  }
}

class Controller extends ControllerBase {
  refresh() {
    fetch('/dev/visible')
      .then(r => r.text())
      .then(r => this.emit(DevToolsVisibilityChangedEvent, r === 'true'));
    fetch('/ls')
      .then(r => r.json())
      .then(r => this.emit(DirChangeEvent, r.name, r.entries));

    return false;
  }

  up() {
    fetch('/up')
      .then(x => x.json())
      .then(j => this.emit(DirPopEvent, j.entries));

    return false;
  }

  enter(subdir) {
    fetch(`/enter/${subdir}`)
      .then(x => x.json())
      .then(j => this.emit(DirPushEvent, subdir, j.entries));

    return false;
  }

  cd(path) {
    fetch(`/cd${path}`)
      .then(x => x.json())
      .then(j => this.emit(DirChangeEvent, j.name, j.entries));

    return false;
  }

  loadFile(name) {
    fetch(`/view/${name}`)
      .then(async r => {
        console.log(r.headers);
        this.emit(
        FileLoadEvent, name, r.headers.get('Content-Type'),
        await r.arrayBuffer())
      });

    return false;
  }

  unloadFile() {
    this.emit(FileUnloadEvent);

    return false;
  }

  toggleDevTools() {
    fetch('/dev/toggle')
      .then(r => r.text())
      .then(r => this.emit(DevToolsVisibilityChangedEvent, r === 'show'));

    return false;
  }
}

class View {
  /** @type {HTMLElement} */
  el;

  /**
   * @param {HTMLElement} el
   */
  constructor(el) {
    this.el = el;
  }
}

class DevButtonView extends View {
  constructor(el, cx) {
    super(el);
    el.onclick = cx.toggleDevTools.bind(cx);
    cx.subscribe(DevToolsVisibilityChangedEvent, this.onVisibilityChanged.bind(this));
  }

  onVisibilityChanged(e) {
    if (e.visible) {
      this.el.classList.remove('bg-slate-200');
      this.el.classList.add('bg-orange-400');
    } else {
      this.el.classList.remove('bg-orange-400');
      this.el.classList.add('bg-slate-200');
    }
  }
}

class DirNameView extends View {
  /** @type {string[]} */
  path;

  constructor(el, cx) {
    super(el);
    cx.subscribe(DirPopEvent, this.onGoUp.bind(this));
    cx.subscribe(DirPushEvent, this.onEnter.bind(this));
    cx.subscribe(DirChangeEvent, this.onCd.bind(this));
  }

  makePart(cx, name, path) {
    const a = document.createElement('a');
    a.innerText = name;
    a.classList.add('cursor-pointer', 'text-sky-700', 'hover:underline');
    a.onclick = () => cx.cd(path);
    return a;
  }

  onGoUp() {
    this.path.pop();
    this.el.children.item(this.el.children.length - 1).remove();
  }

  /** @param {DirPushEvent} e */
  onEnter(e) {
    let subdir = e.name + '/';
    this.path.push(e.name);
    this.el.appendChild(this.makePart(e.target, subdir, this.path.join('/')));
  }

  /** @param {DirChangeEvent} e */
  onCd(e) {
    this.path = e.path.split('/');
    let currentPath = '';
    this.el.replaceChildren.apply(this.el, this.path.map(p => {
      p += '/';
      currentPath += p;
      return this.makePart(e.target, p, currentPath);
    }));
  }
}

class DirView extends View {
  onChange(e) {
    this.el.replaceChildren.apply(this.el, e.entries.map(entry => {
      const li = document.createElement('li');
      const link = document.createElement('a');
      link.innerText = entry.name;
      link.classList.add('cursor-pointer', 'hover:underline');
      if (entry.isDir) {
        link.classList.add('text-sky-700');
        if (entry.name === '..') {
          link.onclick = () => { e.target.up(); return false; };
        } else {
          link.onclick = () => e.target.enter(entry.name);
        }
      } else {
        link.classList.add('text-emerald-700');
        link.onclick = () => e.target.loadFile(entry.name);
      }
      li.appendChild(link);
      return li;
    }));
  }
}

class FileView extends View {
  onLoad(e) {
    const li1 = document.createElement('li');
    const back = document.createElement('a');
    back.innerText = '< back';
    back.classList.add('cursor-pointer', 'text-sky-700', 'hover:underline');
    back.onclick = () => e.target.unloadFile();
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

class ContentView extends View {
  /** @type {FileView?} */
  file;
  /** @type {DirView} */
  dir;

  constructor(el, cx) {
    super(el);

    this.file = new FileView(el);
    this.dir = new DirView(el);

    cx.subscribe(DirPopEvent, this.onDirChange.bind(this));
    cx.subscribe(DirPushEvent, this.onDirChange.bind(this));
    cx.subscribe(DirChangeEvent, this.onDirChange.bind(this));
    cx.subscribe(FileLoadEvent, this.onFileLoaded.bind(this));
    cx.subscribe(FileUnloadEvent, this.onFileUnloaded.bind(this));
  }

  onDirChange(e) {
    this.dir.onChange(e);
  }

  onFileLoaded(e) {
    this.file.onLoad(e);
  }

  onFileUnloaded(e) {
    e.target.refresh();
  }
}

function initView(cx) {
  new DevButtonView(document.getElementById('dev'), cx);
  new DirNameView(document.getElementById('dirname'), cx);
  new ContentView(document.getElementById('items'), cx);
}

let __controller;
document.addEventListener('DOMContentLoaded', async () => {
  __controller = new Controller;
  initView(__controller);
  __controller.refresh();
});
