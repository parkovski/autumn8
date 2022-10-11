//import {ControllerBase} from './lib';
import {Directory, File} from './model';
import {Subject} from 'rxjs';

export class Browser {
  _cwd: Directory;
  _backList: Directory[];
  _forwardList: Directory[];

  readonly cwd: Subject<Directory>;
  readonly file: Subject<File>

  constructor() {
    this._cwd = {
      kind: 'directory',
      name: '',
      parent: null,
      entries: null,
    };
    this._backList = [];
    this._forwardList = [];

    this.cwd = new Subject;
    this.file = new Subject;
  }

  async reset() {
    const cwd = await (await fetch('//app/cwd')).text();
    const parts = cwd.split('/');
    A8_DEBUG('cwd parts', parts);
    let dir = parts.reduce<Directory>((parent, name) => ({
      kind: 'directory',
      name,
      parent,
      entries: null,
    }), null!);
    this._cwd = dir;
    this.cwd.next(this._cwd);
  }

  async toggleDevTools() {
    const mode = await (await fetch('//app/dev/toggle')).text();
    if (mode === 'true') return true;
    return false;
  }
};

export default new Browser;
