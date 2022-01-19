const path = require('path');
const fs = require('fs');
const HtmlWebpackPlugin = require('html-webpack-plugin');

/**
 * @param {{app: string, root?: string, entry?: string}} env
 * `root` defaults to `src/tools/[env.app]`.
 * `entry` defauls to first defined `./index.[js|jsx|ts|tsx]`.
 */
module.exports = env => {
  if (!env.app) {
    throw 'Must define webpack env `app`';
  }

  if (env.root) {
    if (!path.isAbsolute(env.root)) {
      env.root = path.resolve(__dirname, env.root);
    }
  } else {
    env.root = path.resolve(__dirname, `src/tools/${env.app}`);
  }

  if (!env.entry) {
    for (let f of ['index.js', 'index.jsx', 'index.ts', 'index.tsx']) {
      if (fs.existsSync(path.resolve(env.root, f))) {
        env.entry = `./${f}`;
        break;
      }
    }
  }

  return {
    mode: process.env.NODE_ENV || 'development',
    context: env.root,
    entry: {
      [env.app]: env.entry,
    },
    output: {
      path: path.resolve(__dirname, 'build/web', env.app),
      filename: '[name].bundle.js',
      clean: true,
      uniqueName: `autumn8-${env.app}`,
    },
    plugins: [
      new HtmlWebpackPlugin({
        filename: 'index.html',
        template: 'index.html',
      })
    ],
    module: {
      rules: [
        {
          test: /\.m?js$/,
          exclude: /node_modules/,
          loader: 'babel-loader',
        },
        {
          test: /\.tsx?$/,
          exclude: /node_modules/,
          loader: 'babel-loader',
        },
        {
          test: /\.css$/,
          use: ['style-loader', 'css-loader', 'postcss-loader'],
        },
        {
          test: /\.html$/,
          loader: 'html-loader',
        },
        {
          test: /\.(eot|ttf|woff2?|svg|png|jpg|jpeg|gif|webp)$/i,
          type: 'asset/resource',
        },
        {
          test: /\.(glsl|frag|vert)$/,
          type: 'asset/source',
          loader: 'glslify-loader',
        },
      ],
    },
    resolve: {
      extensions: ['.js', '.jsx', '.ts', '.tsx'],
    },
  };
};