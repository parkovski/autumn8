export default class View<E extends Element> {
  #element: E;

  constructor(element: E) {
    this.#element = element;
  }

  destroy() {
    this.#element.remove();
  }

  get element() { return this.#element; }
}
