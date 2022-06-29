export default function getTemplateObject(id) {
  return {
    id,
    attach(element) {
      element
        .attachShadow({mode: 'open'})
        .appendChild(
          document.getElementById(id).content.cloneNode(true)
        );
    }
  };
}
