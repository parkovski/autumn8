import View, {PropsWithChildren} from './view';
import {css} from '@emotion/css';

type AnyMenuItem = Item | SubMenu | Title | Separator;

function itemOnClick(this: any, e: MouseEvent): boolean {
  if (this.props.onClick?.call(this, e) !== false) {
    this.menu.hide();
  }
  return false;
}

const ItemClass = css({
  border: '1px solid transparent',
  cursor: 'pointer',
  padding: '0.125rem 2rem',
  '&:hover': {
    backgroundColor: 'rgb(var(--color-primary-light) / 0.5)',
    borderColor: 'rgb(var(--color-primary-normal))',
  },
});
interface ItemProps {
  icon?: string | HTMLElement;
  disabled?: boolean;
  onClick?: (this: Item, e: MouseEvent) => boolean;
}
class Item extends View<ItemProps> {
  menu!: Menu;

  onCreate(): HTMLDivElement {
    const onClick = itemOnClick.bind(this);

    return View.createElement('div', {
      class: ItemClass,
      tabIndex: 0,
      onMouseOver: () => this.el!.focus(),
      onMouseOut: () => this.el!.blur(),
      onClick,
      onContextMenu: onClick,
      onAuxClick: onClick,
    }, this.props.children);
  }
}

interface SubMenuProps {
  text: string;
  icon?: string | HTMLElement;
  disabled?: boolean;
  onClick?: (this: SubMenu, e: MouseEvent) => boolean;
}
class SubMenu extends View<SubMenuProps, AnyMenuItem> {
  menu!: Menu;

  onCreate() {
    const onClick = itemOnClick.bind(this);

    return View.createElement('div', {
      class: ItemClass,
      tabIndex: 0,
      onMouseOver: () => this.onMouseOver(),
      onMouseOut: () => this.onMouseOut(),
      onClick,
      onContextMenu: onClick,
      onAuxClick: onClick,
    }, [this.props.text, '>']);
  }

  onMouseOver() {
    this.el!.focus();
  }

  onMouseOut() {
    this.el!.blur();
  }
}

const TitleClass = css({
  backgroundColor: 'rgb(var(--color-bg-normal))',
  fontSize: '0.875rem',
  lineHeight: '1.25rem',
  padding: '0.125rem 2rem',
  margin: '0.125rem 0',
});
interface TitleProps {
  icon?: string | HTMLElement;
}
class Title extends View<TitleProps, string> {
  menu!: Menu;

  onCreate() {
    return View.createElement('div', {
      class: TitleClass,
    }, this.props.children);
  }
}

const SeparatorClass = css({
  borderBottom: '1px solid rgb(var(--color-bg-dark))',
  cursor: 'default',
  paddingTop: '0.25rem',
  marginBottom: '0.25rem',
});
class Separator extends View<{}, undefined> {
  menu!: Menu;

  onCreate() {
    return View.createElement('div', {
      class: SeparatorClass,
    });
  }
}

const MenuClass = css({
  position: 'fixed',
  zIndex: 9999,
  userSelect: 'none',
  backgroundColor: 'rgb(var(--color-bg-light))',
  border: '1px solid rgb(var(--color-bg-dark))',
  padding: '0.25rem',
});
interface MenuProps {
}
export default class Menu extends View<MenuProps, AnyMenuItem> {
  static Item = Item;
  static SubMenu = SubMenu;
  static Title = Title;
  static Separator = Separator;

  #windowClickHandler;

  constructor(props: PropsWithChildren<MenuProps, AnyMenuItem>, ...children: AnyMenuItem[]) {
    super(props, ...children);
    this.props.children!.forEach(c => c.menu = this);

    this.#windowClickHandler = (function(this: Menu, e: MouseEvent) {
      if (e.target instanceof HTMLElement && !this.#isMenuElement(e.target)) {
        this.hide();
      }
    }).bind(this);

    this.attach(document.body);
  }

  onCreate() {
    return View.createElement('nav', {
      style: { visibility: 'hidden' },
      class: MenuClass,
    }, this.nodeChildren());
  }

  #isMenuElement(el: HTMLElement | null): boolean {
    if (!el) {
      return false;
    }
    if (el.isSameNode(this.el!)) {
      return true;
    }
    return this.#isMenuElement(el.parentElement);
  }

  #positionInWindow(x: number, y: number): void {
    const width = this.el!.clientWidth;
    const height = this.el!.clientHeight;
    const {clientWidth, clientHeight} = document.documentElement;

    if (x + width > clientWidth) {
      //x = clientWidth - width;
      this.el!.style.right = '0px';
    } else {
      this.el!.style.left = `${x}px`;
    }

    if (y + height > clientHeight) {
      if (height > clientHeight) {
        this.el!.style.top = '0px';
        this.el!.style.overflowY = 'scroll';
      } else {
        this.el!.style.bottom = `${y}px`;
        this.el!.style.overflowY = 'inherit';
      }
    } else {
      this.el!.style.top = `${y}px`;
      this.el!.style.overflowY = 'inherit';
    }
  }

  isVisible(): boolean {
    return this.el!.style.visibility !== 'hidden';
  }

  // FIXME bubbling and all that shiz
  toggle(x: number, y: number): boolean;
  toggle(ref: HTMLElement, yoff?: number): boolean;
  toggle(e: MouseEvent): boolean;
  toggle(x: any, y?: any): boolean {
    if (this.isVisible()) {
      this.hide();
    } else {
      this.show(x, y);
    }
    return false;
  }

  /**
   * @param x The preferred x coordinate of the menu.
   * @param y The preferred y coordinate of the menu.
   */
  show(x: number, y: number): boolean;

  /**
   * @param ref The element to position the menu near.
   * @param space The preferred space to offset the menu from `ref`.
   */
  show(ref: HTMLElement, space?: number): boolean;

  /**
   * @param e The event containing the menu's preferred coordinates.
   */
  show(e: MouseEvent) : boolean;

  show(x: number | HTMLElement | MouseEvent, y?: number): boolean {
    if (this.isVisible() || this.el!.childElementCount === 0) {
      return false;
    }

    if (x instanceof HTMLElement) {
      const rc = x.getBoundingClientRect();
      this.#positionInWindow(
        rc.left + window.scrollX,
        rc.bottom + window.scrollY + (y ?? 4));
    } else if (x instanceof MouseEvent) {
      this.#positionInWindow(x.x, x.y);
    } else {
      this.#positionInWindow(x, y!);
    }
    this.el!.style.visibility = 'visible';

    window.addEventListener('click', this.#windowClickHandler, true);
    window.addEventListener('contextmenu', this.#windowClickHandler, true);
    window.addEventListener('auxclick', this.#windowClickHandler, true);
    return false;
  }

  hide() {
    if (!this.isVisible()) {
      return;
    }
    this.el!.style.visibility = 'hidden';
    window.removeEventListener('click', this.#windowClickHandler, true);
    window.removeEventListener('contextmenu', this.#windowClickHandler, true);
    window.removeEventListener('auxclick', this.#windowClickHandler, true);
  }
}
