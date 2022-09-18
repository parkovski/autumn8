import 'sanitize.css';
import 'sanitize.css/forms.css';
import 'sanitize.css/typography.css';
import './style.css';

import MainElement from './components/main';

interface CustomElementClass extends CustomElementConstructor {
  elementName: string;
}

const customElementClasses: CustomElementClass[] = [
  MainElement,
];

const customElementRegistry = window.customElements;
function defineCustomElement(Class: CustomElementClass) {
  customElementRegistry.define(Class.elementName, Class);
}

customElementClasses.forEach(defineCustomElement);
