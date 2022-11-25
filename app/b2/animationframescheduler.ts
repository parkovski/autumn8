export class AnimationFrameScheduler {
  #pending: (() => void)[];
  #dispatch: () => void;

  constructor() {
    this.#pending = [];
    this.#dispatch = (function(this: AnimationFrameScheduler) {
      const pending = this.#pending;
      this.#pending = [];
      pending.forEach(Function.prototype.apply);
    }).bind(this);
  }

  schedule(target: EventTarget, e: Event): void;
  schedule(f: () => void): void;
  schedule(t: any, e?: Event) {
    if (e) {
      this.#pending.push(() => t.dispatchEvent(e));
    } else {
      this.#pending.push(t);
    }
    if (this.#pending.length === 1) {
      window.requestAnimationFrame(this.#dispatch);
    }
  }
}
