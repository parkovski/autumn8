R"=(
(window.__a8_logger = {
  __level: 0,
  __levels: ['trace', 'debug', 'info', 'warn', 'error', 'critical'],
  __loggers: {
    console: function(level, args) {
      if (level < 0) {
        console[['error', 'log'][level+2]].apply(null, args);
        return;
      }
      if (level < this.__level) return;
      console.log.apply(null, [this.__levels[level]].concat(args));
    },
    spdlog: function(level, args) {
      if (level < 0) {
        level = ['eprint', 'print'][level+2];
      } else {
        level = this.__levels[level];
      }
      fetch(`//app/log/${level}/${encodeURIComponent(args)}`, {
        method: 'GET',
        //body: JSON.stringify(args),
      }).then(res => {
        if (!res.ok) {
          console.error(`Log ${this.__levels[level]} HTTP ${res.status}`);
        }
      }).catch(err => {
        console.error(`Log ${this.__levels[level]} ${err.toString()}`);
      });
    }
  },
  __logger: null,
  setLogger(name) {
    const logger = this.__loggers[name];
    if (!logger) throw new Error(`Logger ${name} not defined`);
    this.__logger = logger.bind(this);
  },
  setLevel(level) {
    if (level < 0 || level >= this.__levels.length) {
      throw new Error(`Level ${level} out of range`);
    }
    this.__level = level;
  },
}).setLogger('spdlog');
['EPRINT', 'PRINT', 'TRACE', 'DEBUG', 'INFO', 'WARN', 'ERROR', 'CRITICAL']
.forEach((m,i) => window['A8_'+m] = function() {
  window.__a8_logger.__logger(i-2, Array.prototype.slice.call(arguments))
});
)="
