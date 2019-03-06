import merge from 'webpack-merge';
import common from './webpack.config.common.babel';
import webpack from 'webpack';

const prodConfig = merge(common, {
    mode: 'production',
    plugins:[
        new webpack.DefinePlugin({
            DEVELOPMENT: JSON.stringify(false)
        })
    ]
});

module.exports = prodConfig;
