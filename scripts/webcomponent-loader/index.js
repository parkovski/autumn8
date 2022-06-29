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
  const id = `tmpl-${Math.floor(Math.random() * 0x80000000).toString(16)}`;
  const styleFile = path.resolve(this.context, 'style.css');

  const finish = style => {
    if (templates.length === 0) {
      HtmlWebpackPlugin.getHooks(this._compilation)
        .alterAssetTagGroups
        .tap('WebComponentLoader', pluginData => {
          pluginData.bodyTags = pluginData.bodyTags.concat(templates);
          return pluginData;
        });
    }

    let innerHTML = style ? `${style}${source}` : source;
    templates.push({
      tagName: 'template',
      innerHTML,
      attributes: { id },
    });
    callback(null, `module.exports=require('_gettmpobj')('${id}')`);
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

  // How to add the CSS file to the build and get its ID here?
  /*
  fs.access(styleFile, fs.constants.R_OK).then(err => {
    if (!err) {
      this.importModule(styleFile)
        .then(m => finish(`<link rel="stylesheet" href="???">`))
        .catch(err => callback(err));
    } else {
      finish();
    }
  });
  */
}
