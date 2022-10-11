import './style.css';

import browser from './browser';

import MainElement from './components/main';
import ToolbarElement from './components/toolbar';

interface CustomElementClass extends CustomElementConstructor {
  elementName: string;
}

const customElementClasses: CustomElementClass[] = [
  MainElement,
  ToolbarElement,
];

customElementClasses.forEach(Class => customElements.define(Class.elementName, Class));

document.addEventListener('DOMContentLoaded', async () => {
  A8_DEBUG('DOMContentLoaded');
  browser.reset();
});
