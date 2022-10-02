import './style.css';

import {browser} from './browser';

import MainElement from './components/main';
import ToolbarElement from './components/toolbar';

interface CustomElementClass extends CustomElementConstructor {
  elementName: string;
}

const customElementClasses: CustomElementClass[] = [
  MainElement,
  ToolbarElement,
];

const customElementRegistry = window.customElements;
function defineCustomElement(Class: CustomElementClass) {
  customElementRegistry.define(Class.elementName, Class);
}

customElementClasses.forEach(defineCustomElement);

document.addEventListener('DOMContentLoaded', async () => {
  (document.querySelector('b2-toolbar') as ToolbarElement)
    .curdir = await browser.getCurdir();
});
