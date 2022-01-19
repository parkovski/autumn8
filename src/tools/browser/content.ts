import {BrowserController} from './controller';
import {DirView} from './dir';
import {DirChangeEvent, DirPopEvent, DirPushEvent, FileLoadEvent, FileUnloadEvent} from './events';
import {FileView, ImageView, TextView} from './file';
import {View} from './lib';
import {WaveView} from './wave';

export class ContentView extends View {
  file?: View;
  dir: DirView;

  constructor(el: HTMLElement, cx: BrowserController) {
    super(el);

    this.dir = new DirView(el);

    cx.subscribe(DirPopEvent, this.onDirChange.bind(this));
    cx.subscribe(DirPushEvent, this.onDirChange.bind(this));
    cx.subscribe(DirChangeEvent, this.onDirChange.bind(this));
    cx.subscribe(FileLoadEvent, this.onFileLoaded.bind(this));
    cx.subscribe(FileUnloadEvent, this.onFileUnloaded.bind(this));
  }

  onDirChange(e: DirChangeEvent | DirPushEvent | DirPopEvent) {
    this.dir.onChange(e);
  }

  onFileLoaded(e: FileLoadEvent) {
    if (e.contentType === 'text/plain') {
      this.file = new TextView(this.el, new TextDecoder('utf-8').decode(new Uint8Array(e.content)));
    } else if (e.contentType.startsWith('image')) {
      this.file = new ImageView(this.el, e.content, e.contentType);
    } else if (e.contentType.startsWith('audio')) {
      this.file = new WaveView(this.el, e.content);
    } else {
      this.file = new FileView(this.el);
      (this.file as FileView).onLoad(e);
    }
  }

  onFileUnloaded() {
    this.file!.destroy();
    this.file = undefined;
    this.dir.show();
  }
}
