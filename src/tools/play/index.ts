import MainView from './views/mainview';

import './style.css';

document.addEventListener('DOMContentLoaded', () => {
  new MainView().attach(document.body);
});
