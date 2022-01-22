export default class Container {
  el;

  constructor() {
    this.el = (
      <div class="p-1 absolute inset-0 bg-zinc-800">
        <div class="bg-primary">Primary</div>
        <div class="bg-secondary">Secondary</div>
        <div class="bg-tertiary">Tertiary</div>
        <div class="bg-complement">Complement</div>
      </div>
    );
  }

  attach(el) {
    el.appendChild(this.el);
  }
}