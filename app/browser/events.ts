export class ModelReadyEvent extends Event {
  static TYPE = 'model-ready';

  constructor() {
    super(ModelReadyEvent.TYPE);
  }
}

export class ContextMenuShowEvent extends Event {
  static TYPE = 'context-menu-show';

  constructor(public object: object, public element: HTMLElement) {
    super(ContextMenuShowEvent.TYPE);
    this.object = object;
    this.element = element;
  }
}

interface DirEntry {
  name: string;
  isDir: boolean
}

export class DirChangeEvent extends Event {
  static TYPE = 'dir-change';

  constructor(public path: string, public entries: DirEntry[]) {
    super(DirChangeEvent.TYPE);
    this.path = path;
    this.entries = entries;
  }
}

export class DirPushEvent extends Event {
  static TYPE = 'dir-push';

  constructor(public name: string, public entries: DirEntry[]) {
    super(DirPushEvent.TYPE);
    this.name = name;
    this.entries = entries;
  }
}

export class DirPopEvent extends Event {
  static TYPE = 'dir-pop';

  constructor(public entries: DirEntry[]) {
    super(DirPopEvent.TYPE);
    this.entries = entries;
  }
}

export class FileLoadEvent extends Event {
  static TYPE = 'file-load';

  private inferContentType() {
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
    }[ext] || 'application/octet-stream';
  }

  constructor(public name: string, public contentType: string, public content: ArrayBuffer) {
    super(FileLoadEvent.TYPE);
    this.name = name;
    this.contentType = contentType;
    this.inferContentType();
    this.content = content;
  }
}

export class FileUnloadEvent extends Event {
  static TYPE = 'file-unload';

  constructor() {
    super(FileUnloadEvent.TYPE);
  }
}

export class HistoryEvent extends Event {
  static TYPE = 'history-update';

  constructor(public direction: 'back' | 'forward', public list: string[]) {
    super(HistoryEvent.TYPE);
    this.direction = direction;
    this.list = list;
  }
}

interface NavState {
  canGoUp: boolean,
  canGoBack: boolean,
  canGoForward: boolean,
}

export class NavStateEvent extends Event {
  static TYPE = 'nav-state-change';

  constructor(public state: NavState) {
    super(NavStateEvent.TYPE);
    this.state = state;
  }
}

export class DevToolsVisibilityChangedEvent extends Event {
  static TYPE = 'dev-tools-vis-change';

  constructor(public visible: boolean) {
    super(DevToolsVisibilityChangedEvent.TYPE);
    this.visible = visible;
  }
}
