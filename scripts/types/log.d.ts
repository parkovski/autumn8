interface Logger {
  setLogger(name: string);
  setLevel(level: number);
}

declare global {
  var A8_LOGGER: Logger;

  function A8_TRACE(...args: any[]): void;
  function A8_DEBUG(...args: any[]): void;
  function A8_INFO(...args: any[]): void;
  function A8_WARN(...args: any[]): void;
  function A8_ERROR(...args: any[]): void;
  function A8_CRITICAL(...args: any[]): void;

  function A8_PRINT(...args: any[]): void;
  function A8_EPRINT(...args: any[]): void;
}

export {}
