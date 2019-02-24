const stages = require('./babelStages');
module.exports = {
    presets: [
        [
            '@babel/preset-env',
            {
                targets: {
                    esmodules: true
                }
            }
        ],
        '@babel/preset-react'
    ],
    plugins: [
        ...stages.Stage1,
        ...stages.Stage2,
        ...stages.Stage3
    ]
};
