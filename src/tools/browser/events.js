class ModelReadyEvent extends Event {
  static TYPE = 'model-ready';

  constructor() {
    super(ModelReadyEvent.TYPE);
  }
}

class ContextMenuShowEvent extends Event {
  static TYPE = 'context-menu-show';

  /** @type {object} */
  object;
  /** @type {HTMLElement} */
  element;

  constructor(o, el) {
    this.object = o;
    this.element = el;
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

  inferContentType() {
    if (this.contentType) {
      return;
    }
    const dot = this.name.lastIndexOf('.');
    if (dot === -1) {
      return;
    }
    const ext = this.name.substring(dot + 1);

    this.contentType = {
      'txt': 'text/plain',
      'c': 'text/plain',
      'cpp': 'text/plain',
      'cxx': 'text/plain',
      'cc': 'text/plain',
      'h': 'text/plain',
      'hpp': 'text/plain',
      'hxx': 'text/plain',
      'hh': 'text/plain',
      'js': 'application/javascript',
      'json': 'application/json',
      'png': 'image/png',
      'jpg': 'image/jpeg',
      'jpeg': 'image/jpeg',
      'wav': 'audio/x-wav',
      'mp3': 'audio/mpeg',
      'ogg': 'audio/ogg',
      'flac': 'audio/x-flac',
    }[ext];
  }

  constructor(name, contentType, content) {
    super(FileLoadEvent.TYPE);
    this.name = name;
    this.contentType = contentType;
    this.inferContentType();
    this.content = content;
  }
}

class FileUnloadEvent extends Event {
  static TYPE = 'file-unload';

  constructor() {
    super(FileUnloadEvent.TYPE);
  }
}

class HistoryEvent extends Event {
  static TYPE = 'history-update';

  /** @type {'back'|'forward'} */
  direction;
  /** @type {string[]} */
  list;

  constructor(direction, list) {
    super(HistoryEvent.TYPE);
    this.direction = direction;
    this.list = list;
  }
}

class NavStateEvent extends Event {
  static TYPE = 'nav-state-change';

  /** @type {{canGoUp: boolean, canGoBack: boolean, canGoForward: boolean}} */
  state;

  constructor(state) {
    this.state = state;
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
