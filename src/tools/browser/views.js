class DevButtonView extends View {
  classesOn;
  classesOff;

  constructor(el, cx) {
    super(el);
    el.onclick = cx.toggleDevTools.bind(cx);
    this.classesOn = el.getAttribute('data-class-on').split(/ +/);
    this.classesOff = el.getAttribute('data-class-off').split(/ +/);
    cx.subscribe(DevToolsVisibilityChangedEvent, this.onVisibilityChanged.bind(this));
  }

  onVisibilityChanged(e) {
    if (e.visible) {
      this.el.classList.remove.apply(this.el.classList, this.classesOff);
      this.el.classList.add.apply(this.el.classList, this.classesOn);
    } else {
      this.el.classList.remove.apply(this.el.classList, this.classesOn);
      this.el.classList.add.apply(this.el.classList, this.classesOff);
    }
  }
}

class ContextMenuView extends View {
  visible = false;
  onSelect;
  onCancel;
  _windowClickHandler;

  constructor(cx) {
    super(document.createElement('div'));
    this.cx = cx;
    this.el.style.position = 'fixed';
    this.el.classList.add('bg-zinc-700', 'border', 'border-zinc-500',
      'text-white', 'select-none', 'py-1');

    this._windowClickHandler = (function(e) {
      if (!this._isMenuElement(e.target)) {
        this.hide();
      }
    }).bind(this);
  }

  /** @param {HTMLElement} el */
  _isMenuElement(el) {
    if (!el) {
      return false;
    }
    if (el.isSameNode(this.el)) {
      return true;
    }
    return this._isMenuElement(el.parentElement);
  }

  _createItem(text, index) {
    const el = document.createElement('div');
    el.classList.add('hover:bg-sky-400/50', 'border', 'border-transparent',
      'hover:border-sky-400', 'cursor-pointer', 'p-0.5');
    el.innerText = text;
    el.tabIndex = index + 1;
    el.name = 'item-' + index;
    el.onclick = () => {
      if (typeof this.onSelect === 'function') {
        this.onSelect(i);
      }
      this.hide();
    };
    el.onmouseover = e => e.target.focus();
    el.onmouseout = e => e.target.blur();
    return el;
  }

  _createSeparator() {
    const el = document.createElement('div');
    el.classList.add('border-b', 'border-zinc-500',
      'cursor-default', 'pt-1', 'mb-1');
    el.onclick = () => false;
    return el;
  }

  setItems(items) {
    this.el.replaceChildren.apply(this.el, items.map((item, i) =>
      item === '-' ? this._createSeparator() : this._createItem(item, i)));
  }

  /**
   * @param {HTMLElement|number} x An element to use as the menu anchor
   *  reference point or the preferred x coordinate of the menu.
   * @param {number|string[]|null} y If `x` is an element, this is the
   *  preferred space between the element and the menu. Defaults to 4px if
   *  null. Otherwise it is the preferred y coordinate of the menu. Also
   *  if `x` is an element, `y` can be the items list, in which case the
   *  default offset is used.
   * @param {string[]?} items The list of menu items.
   */
  show(x, y, items) {
    if (typeof items === 'undefined' && x instanceof HTMLElement &&
        Array.isArray(y)) {
      items = y;
      y = null;
    }
    if (items) {
      this.setItems(items);
    }
    if (this.visible || this.el.childElementCount === 0) {
      return;
    }

    let left, top;
    if (typeof x === 'number') {
      left = `${x}px`;
      top = `${y}px`;
    } else {
      if (y == null) {
        y = 4;
      }
      const rc = x.getBoundingClientRect();
      left = `${rc.left + window.scrollX}px`;
      top = `${rc.bottom + window.scrollY + y}px`;
    }
    this.el.style.left = left;
    this.el.style.top = top;

    document.body.appendChild(this.el);
    window.addEventListener('click', this._windowClickHandler, true);
    window.addEventListener('contextmenu', this._windowClickHandler, true);
    window.addEventListener('auxclick', this._windowClickHandler, true);
    this.visible = true;
  }

  hide() {
    if (!this.visible) {
      return;
    }
    this.el.remove();
    window.removeEventListener('click', this._windowClickHandler, true);
    window.removeEventListener('contextmenu', this._windowClickHandler, true);
    window.removeEventListener('auxclick', this._windowClickHandler, true);
    this.visible = false;
  }
}

class HistoryButtonView extends View {
  direction;
  history;
  contextMenu;

  constructor(el, cx, dir) {
    super(el);
    this.direction = dir;
    this.contextMenu = new ContextMenuView(cx);
    el.onclick = cx[dir].bind(cx, 0);
    el.oncontextmenu = cx.getHistory.bind(cx, dir);
    cx.subscribe(HistoryEvent, this.onHistoryUpdated.bind(this));
  }

  showMenu() {
    if (this.history.length === 0) {
      return;
    }
    this.contextMenu.show(this.el, this.history);
  }

  onHistoryUpdated(e) {
    if (e.direction !== this.direction) {
      return;
    }
    this.history = e.list;
    this.showMenu();
  }
}

class BackButtonView extends HistoryButtonView {
  constructor(el, cx) {
    super(el, cx, 'back');
  }
}

class ForwardButtonView extends HistoryButtonView {
  constructor(el, cx) {
    super(el, cx, 'forward');
  }
}

class UpButtonView extends View {
  constructor(el, cx) {
    super(el);
    el.onclick = cx.up.bind(cx);
  }
}

class DirNameView extends View {
  /** @type {string[]} */
  path;

  constructor(el, cx) {
    super(el);
    cx.subscribe(DirPopEvent, this.onGoUp.bind(this));
    cx.subscribe(DirPushEvent, this.onEnter.bind(this));
    cx.subscribe(DirChangeEvent, this.onCd.bind(this));
  }

  makePart(cx, name, path) {
    const a = document.createElement('a');
    a.innerText = name;
    a.classList.add('cursor-pointer', 'text-white', 'hover:underline');
    a.onclick = () => cx.cd(path);
    return a;
  }

  onGoUp() {
    this.path.pop();
    this.el.children.item(this.el.children.length - 1).remove();
  }

  /** @param {DirPushEvent} e */
  onEnter(e) {
    let subdir = e.name + '/';
    this.path.push(e.name);
    this.el.appendChild(this.makePart(e.target, subdir, this.path.join('/')));
  }

  /** @param {DirChangeEvent} e */
  onCd(e) {
    this.path = e.path.split('/');
    let currentPath = '';
    this.el.replaceChildren.apply(this.el, this.path.map(p => {
      p += '/';
      currentPath += p;
      return this.makePart(e.target, p, currentPath);
    }));
  }
}

class DirView extends View {
  container;
  contextMenu;

  constructor(el, cx) {
    super(document.createElement('ul'));
    this.container = el;
    this.contextMenu = new ContextMenuView(cx);
    this.contextMenu.setItems(['Open', 'Open with...', '-', 'Cut', 'Copy',
      'Paste', '-', 'Properties...']);
  }

  show() {
    this.container.replaceChildren(this.el);
  }

  onChange(e) {
    const children = e.entries.map(entry => {
      const li = document.createElement('li');
      const link = document.createElement('a');
      const icon = document.createElement('i');
      icon.classList.add('fas', 'pr-1');
      link.classList.add('cursor-pointer', 'hover:underline', 'text-yellow-50');
      if (entry.isDir) {
        icon.classList.add('fa-folder', 'text-yellow-200');
        link.onclick = () => e.target.enter(entry.name);
      } else {
        icon.classList.add('fa-file', 'text-orange-400');
        link.onclick = () => e.target.loadFile(entry.name);
      }
      link.oncontextmenu = e => { this.contextMenu.show(e.x, e.y); return false; };
      link.append(icon, entry.name);
      li.appendChild(link);
      return li;
    });
    this.el.replaceChildren.apply(this.el, children);
    //await e.target.renderReady();
    this.show();
  }
}

class FileView extends View {
  constructor(el) {
    super(document.createElement('ul'));
    el.replaceChildren(this.el);
  }

  onLoad(e) {
    const li1 = document.createElement('li');
    const back = document.createElement('a');
    back.innerText = '< back';
    back.classList.add('cursor-pointer', 'text-sky-700', 'hover:underline');
    back.onclick = () => e.target.unloadFile();
    li1.appendChild(back);

    const li2 = document.createElement('li');
    li2.innerText = 'name: ' + e.name;

    const li3 = document.createElement('li');
    li3.innerText = 'type: ' + e.contentType;

    const li4 = document.createElement('li');
    li4.innerText = 'size: ' + e.content.byteLength;

    this.el.replaceChildren(li1, li2, li3, li4);
  }
}

class TextView extends View {
  constructor(el, text) {
    super(document.createElement('pre'));
    this.el.classList.add('text-white');
    this.el.innerText = text;
    el.replaceChildren(this.el);
  }
}

class ImageView extends View {
  constructor(el, data, type) {
    super(document.createElement('div'));
    this.el.classList.add('grid', 'place-content-center');
    const img = document.createElement('img');
    const blob = new Blob([new Uint8Array(data)], { type });
    const url = window.URL.createObjectURL(blob);
    img.src = url;
    this.el.appendChild(img);
    el.replaceChildren(this.el);
  }
}

class WaveView extends View {
  canvas;

  constructor(el, data) {
    super(document.createElement('div'));
    this.el.classList.add('grid', 'place-content-center');
    this.canvas = document.createElement('canvas');
    this.canvas.classList.add('border', 'border-slate-900', 'w-3/4', 'h-3/4');
    this.el.appendChild(this.canvas);
    el.replaceChildren(this.el);
    this.setupCanvas(data);
  }

  async setupCanvas(/*data*/) {
    /** @type {WebGLRenderingContext} */
    const gl = this.canvas.getContext('webgl');
    const [/*buffer, */vertSource, fragSource] = await Promise.all([
      //window.AudioContext.decodeAudioData(data),
      fetch('/wave.vert.glsl').then(x => x.text()),
      fetch('/wave.frag.glsl').then(x => x.text()),
    ]);
    let makeShader = (source, type) => {
      let shader = gl.createShader(type);
      gl.shaderSource(shader, source);
      gl.compileShader(shader);
      if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.log('Shader error: ', gl.getShaderInfoLog(shader));
        gl.deleteShader(shader);
        return null;
      }
      return shader;
    };
    const vertShader = makeShader(vertSource, gl.VERTEX_SHADER);
    if (!vertShader) return;
    const fragShader = makeShader(fragSource, gl.FRAGMENT_SHADER);
    if (!fragShader) return;

    const program = gl.createProgram();
    gl.attachShader(program, fragShader);
    gl.attachShader(program, vertShader);
    gl.linkProgram(program);

    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      console.log('Link error: ', gl.getProgramInfoLog(program));
      return;
    }

    const aVertexPosition = gl.getAttribLocation(program, 'aVertexPosition');
    const uProjectionMatrix = gl.getUniformLocation(program, 'uProjectionMatrix');
    const uModelViewMatrix = gl.getUniformLocation(program, 'uModelViewMatrix');

    const positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);

    const positions = [
       1.0,  1.0,
      -1.0,  1.0,
       1.0, -1.0,
      -1.0, -1.0,
    ];

    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);

    gl.clearColor(0, 0, 0, 1);
    gl.clearDepth(1);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);;

    const fieldOfView = 45 * Math.PI / 180;
    const aspect = gl.canvas.clientWidth / gl.canvas.clientHeight;
    const zNear = 0.1;
    const zFar = 100.0;
    const projectionMatrix = mat4.create();
    mat4.perspective(projectionMatrix, fieldOfView, aspect, zNear, zFar);

    const modelViewMatrix = mat4.create();
    mat4.translate(modelViewMatrix, modelViewMatrix, [0, 0, -6.0]);

    {
      const numComponents = 2;
      const type = gl.FLOAT;
      const normalize = false;
      const stride = 0;
      const offset = 0;
      gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
      gl.vertexAttribPointer(
        aVertexPosition,
        numComponents,
        type,
        normalize,
        stride,
        offset);
      gl.enableVertexAttribArray(aVertexPosition);
    }

    gl.useProgram(program);

    gl.uniformMatrix4fv(uProjectionMatrix, false, projectionMatrix);
    gl.uniformMatrix4fv(uModelViewMatrix, false, modelViewMatrix);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
  }
}

class ContentView extends View {
  /** @type {FileView?} */
  file;
  /** @type {DirView} */
  dir;

  constructor(el, cx) {
    super(el);

    this.file = new FileView(el);
    this.dir = new DirView(el);

    cx.subscribe(DirPopEvent, this.onDirChange.bind(this));
    cx.subscribe(DirPushEvent, this.onDirChange.bind(this));
    cx.subscribe(DirChangeEvent, this.onDirChange.bind(this));
    cx.subscribe(FileLoadEvent, this.onFileLoaded.bind(this));
    cx.subscribe(FileUnloadEvent, this.onFileUnloaded.bind(this));
  }

  onDirChange(e) {
    this.dir.onChange(e);
  }

  onFileLoaded(e) {
    if (e.contentType === 'text/plain') {
      this.file = new TextView(this.el, new TextDecoder('utf-8').decode(new Uint8Array(e.content)));
    } else if (e.contentType.startsWith('image')) {
      this.file = new ImageView(this.el, e.content, e.contentType);
    } else if (e.contentType.startsWith('audio')) {
      this.file = new WaveView(this.el, e.content, e.contentType);
    } else {
      this.file = new FileView(this.el);
      this.file.onLoad(e);
    }
  }

  onFileUnloaded() {
    this.file.destroy();
    this.dir.show();
  }
}
