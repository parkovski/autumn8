import { View } from './lib';

import fragSource from './wave.frag.glsl';
import vertSource from './wave.vert.glsl';

import { mat4 } from 'gl-matrix';

export class WaveView extends View {
  canvas: HTMLCanvasElement;

  constructor(el: HTMLElement/*, data: ArrayBuffer*/) {
    super(document.createElement('div'));
    this.el.classList.add('grid', 'place-content-center');
    this.canvas = document.createElement('canvas');
    this.canvas.classList.add('border', 'border-slate-900', 'w-3/4', 'h-3/4');
    this.el.appendChild(this.canvas);
    el.replaceChildren(this.el);
    this.setupCanvas(/*data*/);
  }

  async setupCanvas(/*data: ArrayBuffer*/) {
    const gl = this.canvas.getContext('webgl');
    if (!gl) {
      console.log('No WebGL context');
      return;
    }
    //const buffer = await window.AudioContext.decodeAudioData(data);
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
