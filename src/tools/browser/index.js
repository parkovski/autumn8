document.addEventListener('DOMContentLoaded', async () => {
  const dirname = document.getElementById('dirname');
  let items = document.getElementById('items');

  const cd = async function(where) {
    if (where === '..') {
      where = '/up';
    } else if (where[0] == '/') {
      where = '/cd' + where;
    } else {
      where = '/enter/' + where;
    }
    await fetch(where);
  };

  let populate;
  const setdir = function(where) {
    const parts = where.split('/');
    const elems = [];
    const putsep = () => {
      const el = document.createElement('span');
      el.innerText = '/';
      elems.push(el);
    };
    const putlink = (text, path) => {
      const el = document.createElement('a');
      el.innerText = text;
      el.classList.add('cursor-pointer', 'text-sky-700', 'hover:underline');
      el.onclick = () => { cd(path).then(populate); return false; };
      elems.push(el);
    };
    let path = '/';
    putlink('/', '/');
    if (parts.length > 1) {
      path += parts[1];
      putlink(parts[1], path);
    }
    for (let i = 2; i < parts.length; ++i) {
      path += '/' + parts[i];
      putsep();
      putlink(parts[i], path);
    }
    dirname.replaceChildren.apply(dirname, elems);
  };

  populate = async function() {
    const contents = (await (await fetch('/ls')).text()).split('\n');
    setdir(contents[0].substr(2));
    const lis = [];
    for (const entry of contents.slice(1)) {
      const li = document.createElement('li');
      if (entry[0] === 'D') {
        const link = document.createElement('a');
        link.innerText = entry.substr(2);
        link.classList.add('cursor-pointer', 'text-sky-700', 'hover:underline');
        link.onclick = () => { cd(entry.substr(2)).then(populate); return false; };
        li.appendChild(link);
      } else {
        li.innerText = entry.substr(2);
      }
      lis.push(li);
    }
    items.replaceChildren.apply(items, lis);
  };

  await populate();
});
