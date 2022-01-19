import { View } from './lib';

import fragSource from './wave.frag.glsl';
import vertSource from './wave.vert.glsl';

export class WaveView extends View {
  canvas: HTMLCanvasElement;
  info;
  ac;

  constructor(el: HTMLElement, data: ArrayBuffer) {
    super(document.createElement('div'));
    this.el.classList.add('flex', 'flex-col',
      'absolute', 'left-0', 'right-0', 'top-0', 'bottom-0', 'items-center',
      'justify-center');

    this.canvas = document.createElement('canvas');
    this.canvas.classList.add('border', 'border-slate-100', 'w-1/2', 'h-1/2', 'flex-initial');
    this.el.appendChild(this.canvas);

    this.info = document.createElement('div');
    this.info.classList.add('text-white', 'flex-initial');
    this.el.appendChild(this.info);

    el.replaceChildren(this.el);

    this.ac = new AudioContext;
    this.setupCanvas(data);
  }

  async setupCanvas(data: ArrayBuffer) {
    const gl = this.canvas.getContext('webgl');
    if (!gl) {
      console.log('No WebGL context');
      return;
    }

    const audioPromise = this.ac.decodeAudioData(data);

    let makeShader = (source: string, type: number) => {
      let shader = gl.createShader(type)!;
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

    const program = gl.createProgram()!;
    gl.attachShader(program, fragShader);
    gl.attachShader(program, vertShader);
    gl.linkProgram(program);

    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      console.log('Link error: ', gl.getProgramInfoLog(program));
      return;
    }

    const aVertexPosition = gl.getAttribLocation(program, 'aVertexPosition');

    const audioBuffer = await audioPromise;
    this.info.innerText =
      `Length: ${audioBuffer.length}, Channels: ${audioBuffer.numberOfChannels}`;
    const ch0 = audioBuffer.getChannelData(0);

    const skip = 6;
    const nCoords = Math.floor(ch0.length / skip);
    const coords = new Float32Array(nCoords * 2);
    for (let i = 0; i < coords.length; i += 2) {
      coords[  i  ] = i * 2 / coords.length - 1;
      coords[i + 1] = ch0[(i >> 1) * skip];
    }

    gl.clearColor(0, 0, 0, 1);
    gl.clearDepth(1);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    {
      const sampleBuffer = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, sampleBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, coords, gl.STATIC_DRAW);

      const numComponents = 2;
      const type = gl.FLOAT;
      const normalize = false;
      const stride = 0;
      const offset = 0;
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
    gl.drawArrays(gl.LINE_STRIP, 0, nCoords);
  }
}
