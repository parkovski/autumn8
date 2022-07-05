/**
 * @param {string} id Template element `id` attribute.
 * @return {{
 *  id: string,
 *  getElement(): HTMLTemplateElement,
 *  attach(element: HTMLElement): void,
 * }}
*/
export default function getTemplate(id) {
  return {
    id,
    getElement() {
      return document.getElementById(id);
    },
    attach(element) {
      element
        .attachShadow({mode: 'open'})
        .appendChild(
          document.getElementById(id).content.cloneNode(true)
        );
    }
  };
}
