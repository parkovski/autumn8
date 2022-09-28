import './style.css';

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
