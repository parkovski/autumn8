//import {ControllerBase} from './lib';
import {Subject} from 'rxjs';

export class Browser {
  readonly curdir: Subject<string>;

  constructor() {
    this.curdir = new Subject;
  }

  async getCurdir() {
    return await (await fetch('//app/pwd')).text();
  }

  async goBack() {
    this.curdir.next(await (await fetch('//app/go/back')).text());
  }

  async goForward() {
    this.curdir.next(await (await fetch('//app/go/forward')).text());
  }

  async goUp() {
    this.curdir.next(await (await fetch('//app/go/up')).text());
  }

  async enter(path: string) {
    this.curdir.next(await (await fetch(`//app/go/in/${path}`)).text());
  }
};

export const browser = new Browser;
