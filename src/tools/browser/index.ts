import {ContentView} from './content';
import { BrowserController } from './controller';

import './style.css';
import {BackButtonView, DevButtonView, DirNameView, ForwardButtonView, UpButtonView} from './toolbar';

let __cx;
document.addEventListener('DOMContentLoaded', () => {
  __cx = new BrowserController;
  __cx.createViews(
    DevButtonView, 'dev',
    BackButtonView, 'back',
    ForwardButtonView, 'forward',
    UpButtonView, 'up',
    DirNameView, 'dirname',
    ContentView, 'items'
  );
  __cx.ready();
});
