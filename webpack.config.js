const path = require('path');
const fs = require('fs');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');

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

  let mode = 'development';
  if (env.production) {
    mode = 'production';
  } else if (process.env.NODE_ENV) {
    mode = process.env.NODE_ENV;
  }
  const isDevMode = mode !== 'production';

  let writeLineColored;
  if (process.stdout.isTTY) {
    writeLineColored = function(text) {
      process.stdout.write(`\x1b[45m\x1b[30m${text}\x1b[m\n`);
    };
  } else {
    writeLineColored  = function(text) { process.stdout.write(text + '\n'); };
  }
  writeLineColored(`Building ${env.app} for ${mode}.`);

  return {
    mode,
    devtool: isDevMode ? 'eval' : void 0,
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
      }),
      /*new class {
        apply(compiler) {
          const PluginName = 'ImportWebComponentTemplates';

          compiler.hooks.compilation.tap(
            PluginName,
            /** @param {import('webpack').Compilation} compilation * /
            compilation => {
              HtmlWebpackPlugin.getHooks(compilation).beforeEmit.tap(
                PluginName,
                data => {
                  const templates = compilation.namedChunkGroups.get('templates');
                  const html = templates?.join('') ?? '<!-- no templates -->';
                  data.html = data.html.replace('<!-- IMPORT TEMPLATES -->', html);
                }
              );
            }
          );
        }
      },*/
    ].concat(isDevMode ? [] : new MiniCssExtractPlugin()),
    module: {
      rules: [
        {
          test: /components\/.+\.html$/,
          loader: path.resolve(__dirname, 'scripts/webcomponent-loader'),
        },
        {
          test: /\.tsx?$/,
          exclude: /node_modules/,
          loader: 'ts-loader',
        },
        {
          test: /\.([cm]js|jsx?)$/,
          exclude: /node_modules/,
          loader: 'babel-loader',
        },
        {
          test: /\.css$/,
          use: [
            isDevMode ? 'style-loader' : MiniCssExtractPlugin.loader,
            'css-loader'
          ],
        },
        {
          test: /\.html$/,
          exclude: /components/,
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
      alias: {
        swoll: path.resolve(__dirname, 'src/swoll'),
      },
      extensions: ['.js', '.jsx', '.ts', '.tsx'],
    },
  };
};