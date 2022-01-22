import Container from './views/container';

import './style.css';

document.addEventListener('DOMContentLoaded', () => {
  new Container().attach(document.body);
});
