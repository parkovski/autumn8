/**
 * Same interface as EventTarget, but events are saved and dispatched together
 * at the next animation frame.
 */
export default class AsyncEventTarget extends EventTarget {
  dispatchEventImmediate: (e: Event) => boolean;

  constructor() {
    super();

    this.dispatchEventImmediate = this.dispatchEvent;

    let pendingEvents: Event[] = [];

    const dispatchEvent = this.dispatchEvent.bind(this);
    const dispatch = (function() {
      const events = pendingEvents;
      pendingEvents = [];
      events.forEach(dispatchEvent);
    }).bind(this);

    this.dispatchEvent = function(e: Event): boolean {
      pendingEvents.push(e);
      if (pendingEvents.length === 1) {
        window.requestAnimationFrame(dispatch);
      }
      return true;
    }
  }
}
