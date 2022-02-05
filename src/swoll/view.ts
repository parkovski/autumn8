import {createElement} from 'jsx-dom';
export type {FC} from 'jsx-dom';

export class ViewBase {
  el: HTMLElement | null = null;

  onCreate(): HTMLElement | null {
    return null;
  }

  render(): HTMLElement | null {
    if (!this.el) {
      this.el = this.onCreate();
    }
    return this.el;
  }

  attach(el: HTMLElement) {
    const this_el = this.render();
    if (this_el) {
      //this.onWillAttach();
      el.append(this_el);
      //this.onDidAttach();
    }
  }

  detach() {
    if (this.el && this.el.parentNode) {
      //this.onWillDetach();
      this.el.remove();
      //this.onDidDetach();
    }
  }

  destroy() {
    if (this.el) {
      this.detach();
      //this.onDestroy();
      this.el = null;
    }
  }
}

type SimpleChild =
  | undefined
  | null
  | boolean
  | string
  | number;

export type DOMChild =
  | SimpleChild
  | Node
  | NodeList
  | HTMLCollection;

export type ViewChild =
  | SimpleChild
  | DOMChild
  | ViewBase;

export type MaybeArray<T> = T | T[];

export type DOMChildren = MaybeArray<DOMChild>;
export type ViewChildren = MaybeArray<ViewChild>;

export type WithChildren<C = DOMChildren> = { children?: C };
export type PropsWithChildren<P = {}, C = DOMChildren> = P & WithChildren<C>;

function nodeify(child: ViewChildren, nodes?: (string | Node)[]): (string | Node)[] {
  if (child == null || typeof child === 'boolean') {
    return [];
  }

  if (!nodes) {
    nodes = [];
  }

  if (typeof child === 'number') {
    nodes.push(String(child));
  } else if (typeof child === 'string' || child instanceof Node) {
    nodes.push(child);
  } else if (child instanceof NodeList || child instanceof HTMLCollection) {
    Array.prototype.forEach.call(child, c => nodeify(c, nodes));
  } else if (Array.isArray(child)) {
    child.forEach(c => nodeify(c, nodes));
  } else if (child instanceof ViewBase) {
    const el = child.render();
    if (el) {
      nodes.push(el);
    }
  }

  return nodes;
}

export default class View<P = {}, C = DOMChild> extends ViewBase {
  readonly props: Readonly<PropsWithChildren<P, C[]>>;

  static createElement = createElement;
  static nodeify = nodeify;

  constructor(props: PropsWithChildren<P, MaybeArray<C>>, ...children: C[]);
  constructor(...children: C[]);
  constructor(props: any, ...children: C[]) {
    super();

    if (typeof props !== 'object' || (props as any).__proto__ !== Object.prototype) {
      // It's part of children.
      props = { children: [props as C, ...children] };
    } else if (children) {
      if (props.children) {
        throw new Error('Pass only children or props.children');
      }
      props.children = children;
    } else if (props.children == null) {
      props.children = [];
    } else if (!Array.isArray(props.children)) {
      props.children = [props.children];
    }

    this.props = props;
  }

  /**
   * Only compatible with C is subset of ViewChild. Basically flatmaps the
   * children to string|node, compatible with append/replaceChildren.
   */
  nodeChildren(): (string | Node)[] {
    return nodeify(this.props.children as any);
  }
}
