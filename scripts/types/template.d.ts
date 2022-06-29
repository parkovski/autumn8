declare module '*/template.html' {
  export const id: string;
  export function attach(el: HTMLElement): ShadowRoot;
}
