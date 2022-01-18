const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const app = {
  name: 'browser',
  title: 'Browser',
  root: 'src/tools/browser',
  entry: './index.ts',
};

module.exports = {
  mode: process.env.NODE_ENV || 'development',
  context: path.resolve(__dirname, app.root),
  entry: {
    [app.name]: app.entry,
  },
  output: {
    path: path.resolve(__dirname, 'build/web'),
    filename: '[name].bundle.js',
    clean: true,
    uniqueName: `autumn8-${app.name}`,
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