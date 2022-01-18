export class View {
  el: HTMLElement;

  constructor(el: HTMLElement) {
    this.el = el;
  }

  destroy() {
    this.el.remove();
  }
}

export class Controller extends EventTarget {
  private readonly _schedule: (e: Event) => void;
  private __views: View[] = [];

  constructor() {
    super();

    let pendingEvents: Event[] = [];

    const dispatchEvent = this.dispatchEvent.bind(this);

    const dispatch = (function() {
      const events = pendingEvents;
      pendingEvents = [];
      events.forEach(dispatchEvent)
    }).bind(this);

    this._schedule = function(e) {
      pendingEvents.push(e);
      if (pendingEvents.length === 1) {
        window.requestAnimationFrame(dispatch);
      }
    };
  }

  emit<E extends Event>(E: new(...args: any[]) => E, ...args: any[]) {
    this._schedule(new (Function.prototype.bind.apply(E, [null, ...args])));
  }

  emitNow<E extends Event>(E: new(...args: any[]) => E, ...args: any[]) {
    this.dispatchEvent(new (Function.prototype.bind.apply(E, [null, ...args])));
  }

  subscribe<E extends Event>(
    E: (new(...args: any[]) => E) & { TYPE: string },
    callback: (e: E) => void
  ) {
    this.addEventListener(E.TYPE, callback as EventListener);
    return [E.TYPE, callback];
  }

  unsubscribe<E extends Event>(
    E: [(new(...args: any[]) => E) & { TYPE: string }, (e: E) => void]
  ): void;
  unsubscribe<E extends Event>(
    E: (new(...args: any[]) => E) & { TYPE: string },
    callback: (e: E) => void
  ): void;
  unsubscribe(E: any, callback?: any) {
    if (Array.isArray(E)) {
      this.removeEventListener.apply(this, E as [string, (e: Event) => void]);
    } else {
      this.removeEventListener(E.TYPE, callback as EventListener);
    }
  }

  /**
   * @param V View class
   * @param {string|HTMLElement} elem Element controlled by view
   */
  createView<V extends View>(
    V: new(e: HTMLElement, c: Controller) => V,
    elem: HTMLElement | string
  ) {
    if (typeof elem === 'string') {
      elem = document.getElementById(elem) ||
        (() => {throw `Element ${elem} does not exist!`})();
    }
    this.__views.push(new V(elem, this));
  }

  // Arguments are a list of view constructor params, e.g.
  // `createViews(FooView, 'foo', BarView, 'bar')`
  // Also accepts an array:
  // `createViews([FooView, 'foo', BarView, 'bar'])`
  // Also accepts pairs as arrays:
  // `createViews([FooView, 'foo'], [BarView, 'bar'])`
  createViews(...args: any[]): void;
  createViews() {
    let list = arguments;
    if (arguments.length === 1) {
      list = arguments[0];
    }
    for (let i = 0; i < list.length; ++i) {
      if (Array.isArray(list[i])) {
        this.createView(list[i][0], list[i][1]);
      } else {
        this.createView(list[i], list[i+1]);
        ++i;
      }
    }
  }
}
