import {View} from './lib';

export class ContextMenuView extends View {
  private visible = false;
  private _windowClickHandler;

  public onSelect?: (index: number) => void;

  constructor() {
    super(document.createElement('div'));
    this.el.style.position = 'fixed';
    this.el.classList.add('bg-zinc-700', 'border', 'border-zinc-500',
      'text-white', 'select-none', 'py-1');

    this._windowClickHandler = (function(this: ContextMenuView, e: MouseEvent) {
      if (e.target instanceof HTMLElement && !this._isMenuElement(e.target)) {
        this.hide();
      }
    }).bind(this);
  }

  private _isMenuElement(el: HTMLElement | null): boolean {
    if (!el) {
      return false;
    }
    if (el.isSameNode(this.el)) {
      return true;
    }
    return this._isMenuElement(el.parentElement);
  }

  private _createItem(text: string, index: number) {
    const el = document.createElement('div');
    el.classList.add('hover:bg-sky-400/50', 'border', 'border-transparent',
      'hover:border-sky-400', 'cursor-pointer', 'p-0.5');
    el.innerText = text;
    el.tabIndex = index + 1;
    el.setAttribute('name', 'item-' + index);
    el.onclick = () => {
      if (typeof this.onSelect === 'function') {
        this.onSelect(index);
      }
      this.hide();
    };
    el.onmouseover = () => el.focus();
    el.onmouseout = () => el.blur();
    return el;
  }

  private _createSeparator() {
    const el = document.createElement('div');
    el.classList.add('border-b', 'border-zinc-500',
      'cursor-default', 'pt-1', 'mb-1');
    el.onclick = () => false;
    return el;
  }

  setItems(items: string[]) {
    this.el.replaceChildren.apply(this.el, items.map((item, i) =>
      item === '-' ? this._createSeparator() : this._createItem(item, i)));
  }

  /**
   * @param x The preferred x coordinate of the menu.
   * @param y The preferred y coordinate of the menu.
   * @param items The list of menu items to show.
   */
  show(x: number, y: number, items?: string[]): void;

  /**
   * @param ref The element to position the menu near.
   * @param space The preferred space to offset the menu from `ref`.
   * @param items The list of menu items to show.
   */
  show(ref: HTMLElement, space: number, items?: string[]): void;

  /**
   * @param ref The element to position the menu near.
   * @param items The list of menu items to show.
   */
  show(ref: HTMLElement, items?: string[]): void;

  show(x: number | HTMLElement, y?: number | string[], items?: string[]) {
    if (typeof items === 'undefined' && x instanceof HTMLElement &&
        Array.isArray(y)) {
      items = y;
      y = undefined;
    }
    if (items) {
      this.setItems(items);
    }
    if (this.visible || this.el.childElementCount === 0) {
      return;
    }

    let left, top;
    if (typeof x === 'number') {
      left = `${x}px`;
      top = `${y}px`;
    } else {
      if (y == null) {
        y = 4;
      }
      const rc = x.getBoundingClientRect();
      left = `${rc.left + window.scrollX}px`;
      top = `${rc.bottom + window.scrollY + (y as number)}px`;
    }
    this.el.style.left = left;
    this.el.style.top = top;

    document.body.appendChild(this.el);
    window.addEventListener('click', this._windowClickHandler, true);
    window.addEventListener('contextmenu', this._windowClickHandler, true);
    window.addEventListener('auxclick', this._windowClickHandler, true);
    this.visible = true;
  }

  hide() {
    if (!this.visible) {
      return;
    }
    this.el.remove();
    window.removeEventListener('click', this._windowClickHandler, true);
    window.removeEventListener('contextmenu', this._windowClickHandler, true);
    window.removeEventListener('auxclick', this._windowClickHandler, true);
    this.visible = false;
  }
}
