import { Controller } from './lib';
import { DirChangeEvent, NavStateEvent, HistoryEvent, DirPushEvent,
  DirPopEvent, FileLoadEvent, FileUnloadEvent, DevToolsVisibilityChangedEvent }
from './events';

export class BrowserController extends Controller {
  hasFileView = false;

  ready() {
    this.refresh();
    this.getDevToolsVisibility();
  }

  refresh() {
    fetch('/ls')
      .then(r => r.json())
      .then(r => this.emit(DirChangeEvent, r.name, r.entries));
    return false;
  }

  getNavState() {
    fetch('/navstate')
      .then(x => x.json())
      .then(j => this.emit(NavStateEvent, j));
    return false;
  }

  /** @param {'back'|'forward'} direction */
  getHistory(direction) {
    fetch(`/history/${direction}`)
      .then(x => x.json())
      .then(j => this.emit(HistoryEvent, direction, j));
    return false;
  }

  _go(where) {
    fetch(`/go/${where}`)
      .then(x => x.json())
      .then(j => this.emit(DirChangeEvent, j.name, j.entries));
    return false;
  }

  back(count) {
    if (this.hasFileView) {
      this.unloadFile();
      if (!count) {
        return;
      }
    }

    return this._go('back' + (count ? `/${count}` : ''));
  }

  forward(count) {
    return this._go('forward' + (count ? `/${count}` : ''));
  }

  up() {
    fetch('/go/up')
      .then(x => x.json())
      .then(j => this.emit(DirPopEvent, j.entries));
    return false;
  }

  enter(subdir) {
    fetch(`/go/in/${subdir}`)
      .then(x => x.json())
      .then(j => this.emit(DirPushEvent, subdir, j.entries))
    return false;
  }

  cd(path) {
    return this._go(`to${path}`);
  }

  loadFile(name) {
    fetch(`/view/${name}`)
      .then(async r => {
        this.emit(FileLoadEvent, name, r.headers.get('Content-Type'),
          await r.arrayBuffer());
        this.hasFileView = true;
      });
    return false;
  }

  unloadFile() {
    this.emit(FileUnloadEvent);
    this.hasFileView = false;
    return false;
  }

  toggleDevTools() {
    fetch('/dev/toggle')
      .then(r => r.text())
      .then(r => this.emit(DevToolsVisibilityChangedEvent, r === 'show'));
    return false;
  }

  getDevToolsVisibility() {
    fetch('/dev/visible')
      .then(r => r.text())
      .then(r => this.emit(DevToolsVisibilityChangedEvent, r === 'true'));
    return false;
  }
}
