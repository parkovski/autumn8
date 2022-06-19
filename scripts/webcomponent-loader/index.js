const HtmlWebpackPlugin = require("html-webpack-plugin");
const fs = require('fs/promises');
const path = require('path');

const templates = [];

module.exports = function(source) {
  const callback = this.async();
  if (!this.resourcePath.endsWith('.html')) {
    callback('Can only handle html files');
    return;
  }

  if (!source.endsWith('\n')) {
    source += '\n';
  }
  const id = Math.floor(Math.random() * 0x1000000).toString(16);
  const styleFile = path.resolve(this.context, 'style.css');

  const finish = style => {
    if (templates.length === 0) {
      HtmlWebpackPlugin.getHooks(this._compilation)
        .afterTemplateExecution
        .tap('WebComponentLoader', pluginData => {
          pluginData.html =
            pluginData.html.replace('<!-- {{TEMPLATES}} -->', templates.join('\n'));
          return pluginData;
        });
    }

    templates.push(`<template id="${id}-template">\n${style}${source}</template>`);
    callback(null, `export default document.getElementById('${id}-template');`);
  };

  fs.readFile(styleFile, 'utf8').then(text => {
    if (!text.endsWith('\n')) {
      text += '\n';
    }
    this.addDependency(styleFile);
    finish(`<style>\n${text}</style>\n`);
  }).catch(e => {
    if (e.code === 'ENOENT') {
      finish(`<!-- ${styleFile} not found. -->\n`);
    } else {
      callback(e);
    }
  });
}
