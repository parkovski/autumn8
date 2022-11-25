import './style.css';

import browser from './browser';

import MainElement from './components/main';
import ToolbarElement from './components/toolbar';

interface CustomElementClass extends CustomElementConstructor {
  elementName: string;
}

const customElementClasses: CustomElementClass[] = [
  ToolbarElement,
  MainElement,
];

customElementClasses.forEach(C => customElements.define(C.elementName, C));

document.addEventListener('DOMContentLoaded', async () => {
  A8_DEBUG('DOMContentLoaded');
  browser.reset();
});
