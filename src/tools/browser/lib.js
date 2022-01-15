class View {
  /** @type {HTMLElement} */
  el;

  /**
   * @param {HTMLElement} el
   */
  constructor(el) {
    this.el = el;
  }

  destroy() {
    this.el.remove();
  }
}

class Controller extends EventTarget {
  /** @type {(e: Event) => void} */
  _schedule;
  __views = [];

  constructor() {
    super();

    let pendingEvents = [];

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

  emit(E) {
    const args = [null].concat(Array.prototype.slice.call(arguments, 1));
    this._schedule(new (Function.prototype.bind.apply(E, args)));
  }

  emitNow(E) {
    const args = [null].concat(Array.prototype.slice.call(arguments, 1));
    this.dispatchEvent(new (Function.prototype.bind.apply(E, args)));
  }

  subscribe(E, callback) {
    this.addEventListener(E.TYPE, callback);
    return [E.TYPE, callback];
  }

  unsubscribe(E, callback) {
    if (Array.isArray(E)) {
      this.removeEventListener.apply(this, E);
    } else {
      this.removeEventListener(E.TYPE, callback);
    }
  }

  /**
   * @param V View class
   * @param {string|HTMLElement} elem Element controlled by view
   */
  createView(V, elem) {
    if (typeof elem === 'string') {
      elem = document.getElementById(elem);
    }
    this.__views.push(new V(elem, this));
  }

  // Arguments are a list of view constructor params, e.g.
  // `createViews(FooView, 'foo', BarView, 'bar')`
  // Also accepts an array:
  // `createViews([FooView, 'foo', BarView, 'bar'])`
  // Also accepts pairs as arrays:
  // `createViews([FooView, 'foo'], [BarView, 'bar'])`
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
