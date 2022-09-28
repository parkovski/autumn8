/**
 * @param {string} id Document element ID.
 */
export default function getTemplate(id) {
  const attach = el => {
    const node = document.getElementById(id).content.cloneNode(true);
    el.appendChild(node);
    return node;
  };

  return {
    get id() { return id; },
    get element() { return document.getElementById(id); },
    attach,
    attachShadow(el, open) {
      return attach(el.attachShadow({mode: open ? 'open' : 'closed'}));
    },
  };
}
