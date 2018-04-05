
const path = require('path');
const assert  = require('assert');
const fs = require('fs');
const wav = require('wav');

const mimic = require('../lib/mimic');

function main() {
    mimic.init();
    mimic.loadVoice(path.resolve(module.filename, '../cmu_us_awb.flitevox'), (err, voice) => {
        if (err)
            throw err;
        if (!voice)
            throw new Error('Failed to load voice file');

        console.log('Obtained voice file');
        voice.textToSpeech("Hello! I am Mimic, and I synthesize text.", (err, result) => {
            assert(typeof result.numChannels === 'number');
            assert(typeof result.sampleRate === 'number');
            assert(result.buffer instanceof Buffer);
            console.log('Sample rate: ' + result.sampleRate);
            console.log('Num channels: ' + result.numChannels);

            let writer = new wav.Writer({
                channels: result.numChannels,
                sampleRate: result.sampleRate,
                bitDepth: 16
            });
            writer.pipe(fs.createWriteStream('output.wav'));
            writer.end(result.buffer);
        });
    });
}
main();
