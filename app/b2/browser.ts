//import {ControllerBase} from './lib';
import {Directory, File} from './model';
import {Subject} from 'rxjs';

export class Browser {
  _cwd: Directory;
  _backList: Directory[];
  _forwardList: Directory[];

  readonly cwd: Subject<Directory>;
  readonly file: Subject<File>;

  constructor() {
    this._cwd = {
      kind: 'directory',
      name: '',
    };
    this._backList = [];
    this._forwardList = [];

    this.cwd = new Subject;
    this.file = new Subject;
  }

  async reset() {
    this._cwd = await (await fetch('//app/ls')).json();
    this.cwd.next(this._cwd);
  }

  async go(path: string) {
    this._cwd = await (await fetch('//app/go' + path)).json();
    this.cwd.next(this._cwd);
  }

  async toggleDevTools() {
    const mode = await (await fetch('//app/dev/toggle')).text();
    if (mode === 'true') return true;
    return false;
  }
};

export default new Browser;
