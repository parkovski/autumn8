const path = require('path');
const fs = require('fs');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');

function writeTitle(text) {
  if (process.stdout.isTTY) {
    process.stdout.write(`\x1b[33m${text}\x1b[m\n`);
  } else {
    process.stdout.write(text + '\n');
  }
}

// TODO: Support multiple pages.
/**
 * @param {{
 *  app: string,
 *  root?: string,
 *  entry?: string,
 *  outputPath?: string,
 * }} env
 * `app` is the app name.
 * `root` defaults to `app/[env.app]`.
 * `entry` defauls to first defined `./index.[js|jsx|ts|tsx]`.
 * `outputPath` defaults to `./build/web/[env.app]`.
 */
module.exports = (env, argv) => {
  if (!env.app) {
    throw 'Must define webpack env `app`';
  }

  if (env.root) {
    if (!path.isAbsolute(env.root)) {
      env.root = path.resolve(__dirname, env.root);
    }
  } else {
    env.root = path.resolve(__dirname, `app/${env.app}`);
  }

  if (!env.entry) {
    for (let f of ['index.js', 'index.jsx', 'index.ts', 'index.tsx']) {
      if (fs.existsSync(path.resolve(env.root, f))) {
        env.entry = `./${f}`;
        break;
      }
    }
  }

  const mode = argv.mode || 'development';
  const isDevMode = mode !== 'production';

  const outputPath = env.outputPath
    || path.resolve(__dirname, 'build/web', env.app);

  writeTitle(`Building ${env.app} for ${mode}.`);

  return {
    mode,
    devtool: isDevMode ? 'eval' : void 0,
    context: env.root,
    entry: {
      [env.app]: env.entry,
    },
    output: {
      path: outputPath,
      filename: 'index.js',
      clean: true,
      uniqueName: `autumn8-${env.app}`,
    },
    plugins: [
      new HtmlWebpackPlugin({
        filename: 'index.html',
        template: 'index.html',
      }),
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
            'css-loader',
            {
              loader: 'postcss-loader',
              options: {
                postcssOptions: {
                  plugins: [
                    [
                      'postcss-preset-env',
                      {
                        stage: 3,
                        minimumVendorImplementations: 2,
                        preserve: false,
                      },
                    ],
                  ].concat(isDevMode ? [] : [
                    [
                      'cssnano',
                      {
                      }
                    ]
                  ]),
                },
              },
            },
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
        swoll: path.resolve(__dirname, 'lib/swoll'),
      },
      extensions: ['.js', '.jsx', '.ts', '.tsx'],
    },
  };
};