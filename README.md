# node-mimic

Node.js wrappers for https://github.com/MycroftAI/mimic (voice synthesis software)

## Building

The module assumes that you installed mimic somewhere where pkg-config can find it.
Adjust PKG_CONFIG_PATH if you installed mimic in a non-standard location.

Then you can build the module with:

    npm install
    
## API:

    const mimic = require('node-mimic');
    
    mimic.loadVoice("path_to_voice_file.flitevox", (err, voice) => {
        voice.textToSpeech("Phrase to synthesize", (err, result) => {
            let sampleRate = result.sampleRate;
            let channels = result.numChannels;
            let buffer = result.buffer; // A standard Buffer, in PCM S16NE format
        });
    });

## Licence:

See [LICENCE].