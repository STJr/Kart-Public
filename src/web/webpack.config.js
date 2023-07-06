const path = require("path");

const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  entry: './js/index.js',
  mode: process.env.NODE_ENV === "production" ? "production" : "development",
  output: {
    filename: 'index.js',
    path: process.env.OUTPUT_PATH || path.resolve(__dirname, 'dist'),
  },
  resolve: {
    extensions: [".js", ".elm"]
  },
  plugins: [new HtmlWebpackPlugin({
    title: "SRB2Kart v1.6",
    xhtml: true,
    template: "index.html"
  })],
  module: {
    rules: [{
      test: /\.elm$/,
      exclude: [/elm-stuff/, /node_modules/],
      use: [
        {
            loader: 'elm-webpack-loader'
        }
      ]
    }]
  }
};
