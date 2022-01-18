import { BrowserController } from './controller';
import { DevButtonView, BackButtonView, ForwardButtonView, UpButtonView,
  DirNameView, ContentView } from './views';

import './style.css';

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
