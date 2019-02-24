import merge from 'webpack-merge';
import common from './webpack.config.common.babel';
import path from 'path';
import webpack from 'webpack';
import HtmlWebpackPlugin from 'html-webpack-plugin';
import CleanWebpackPlugin from 'clean-webpack-plugin';

const devConfig = merge(common,
    {
        entry: [
            'webpack-hot-middleware/client?reyload=true',
            'react-hot-loader/patch'
        ],
        mode: 'development',
        devtool: 'inline-source-map',
        plugins: [
            new webpack.HotModuleReplacementPlugin()
        ]
    }
);


module.exports = devConfig;