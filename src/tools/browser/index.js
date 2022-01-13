let __cx;
document.addEventListener('DOMContentLoaded', async () => {
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
