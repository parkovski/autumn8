import chroma from 'chroma-js';

interface ThemeColor<T> {
  normal: T;
  light: T;
  dark: T;
  dull: T;
}

interface ThemePalette<T> {
  fg: { normal: T; };
  bg: { normal: T; light: T; dark: T; };
  primary: ThemeColor<T>;
  secondary: ThemeColor<T>;
  tertiary: ThemeColor<T>;
  complement: ThemeColor<T>;
}

export class Theme {
  colors: ThemePalette<string>;
  chroma: ThemePalette<chroma.Color>;

  getCssVariables(prefix?: string): Record<string, string> {
    return getThemeVariables(this.colors, prefix);
  }

  getTailwindCssRefs(prefix?: string): Record<string, string> {
    return getThemeRefs(this.colors, prefix);
  }
}

export class ThemeCollection {
  themes: {
    light: Theme;
    dark: Theme;
    [name: string]: Theme;
  };
}

/** @param {object|string} baseColor Color or map of name => color. */
function makePalette(baseColor: object | string): any {
  if (typeof baseColor === 'object') {
    return Object.fromEntries(
      Object.entries(baseColor)
        .map(([k, v]) => [k, makePalette(v)])
    );
  }
  const color = chroma(baseColor);
  const palette: any = {
    normal: color,
    light: color.brighten(Math.PI / 2),
    dark: color.darken(Math.PI / 2),
    dull: color.desaturate(Math.PI * 3 / 4),
  };
  for (let k in palette) {
    palette[k] = palette[k].hex();
  }
  return palette;
}

function withOpacityValue(variable: string) {
  return ({ opacityValue }: { opacityValue?: number }) => {
    if (opacityValue === undefined) {
      return `rgb(var(${variable}))`;
    }
    return `rgb(var(${variable}) / ${opacityValue})`;
  }
}

function getThemeVariables(theme: object, prefix?: string, obj?: object):
  Record<string, string>
{
  prefix = prefix || 'color';
  obj = obj || {};
  return Object.keys(theme).reduce((a, c) => {
    if (typeof theme[c] === 'object') {
      return getThemeVariables(theme[c], `${prefix}-${c}`, a);
    }
    obj[`--${prefix}-${c}`] = chroma(theme[c]).rgb(true).join(' ');
    return obj;
  }, obj) as any;
}

function getThemeRefs(theme: object, prefix?: string): Record<string, string> {
  prefix = prefix || 'color';
  return Object.fromEntries(Object.entries(theme).map(([k, v]) => {
    if (typeof v === 'object') {
      return [k, getThemeRefs(v, `${prefix}-${k}`)];
    }
    return [k, withOpacityValue(`--${prefix}-${k}`)];
  }));
}
