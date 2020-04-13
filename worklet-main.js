import Module from './mpt-worklet.js';

let load_;
let process1_ = () => {};
let process2_ = () => {};
let process4_ = () => {};
let setRepeatCount_ = (rc) => {};
let setPosition_ = (p) => {};
let getPosition_ = () => 0;
let bufferSize;
let leftPtr;
let rightPtr;

let leftArray;
let rightArray;

let initResolver = null;
const init = new Promise((resolve) => { initResolver = resolve; });

Module.onRuntimeInitialized = function() {
    const BUFFERSIZE = 480;

    bufferSize = BUFFERSIZE;
    leftPtr = Module._malloc(BUFFERSIZE * 4);
    leftArray = Module.HEAPF32.subarray(leftPtr >> 2, (leftPtr >> 2) + BUFFERSIZE);
    leftArray.fill(0);

    rightPtr = Module._malloc(BUFFERSIZE * 4);
    rightArray = Module.HEAPF32.subarray(rightPtr >> 2, (rightPtr >> 2) + BUFFERSIZE);
    rightArray.fill(0);

    load_ = Module.cwrap('load', 'void', ['number', 'number']);
    process1_ = Module.cwrap('process1', 'void', ['number', 'number']);
    process2_ = Module.cwrap('process2', 'void', ['number', 'number', 'number']);
    process4_ = Module.cwrap('process4', 'void', ['number', 'number', 'number', 'number', 'number']);
    setRepeatCount_ = Module.cwrap('setRepeatCount', 'void', ['number']);
    setPosition_ = Module.cwrap('setPosition', 'void', ['number']);
    getPosition_ = Module.cwrap('getPosition', 'number', []);

    initResolver();
}

class LibopenmptProcessor extends AudioWorkletProcessor {
    constructor() {
        super();

        this.port.onmessage = async (event) => {
            await init;

            const {songData, setRepeatCount, setPosition, getPosition} = event.data;

            if (songData != null) { // ArrayBuffer
                if (songData.byteLength == 0) {
                    load_(0, 0);
                } else {
                    const srcArray = new Uint8Array(songData);

                    const dataPtr = Module._malloc(srcArray.length);
                    Module.HEAPU8.set(srcArray, dataPtr);

                    load_(dataPtr, srcArray.length);

                    Module._free(dataPtr);
                }
            }

            if (setRepeatCount != null) {
                setRepeatCount_(setRepeatCount);
            }

            if (setPosition != null) {
                setPosition_(setPosition);
            }

            if (getPosition != null) {
                this.port.postMessage({position: getPosition_()});
            }
        };
    }

    process(inputs, outputs, parameters) {
        if (!leftPtr || !rightPtr) {
            return true;
        }

        const left = outputs[0][0]; // Float32Array
        const right = outputs[0][1];

        process2_(left.length, leftPtr, rightPtr);

        left.set(Module.HEAPF32.subarray(leftPtr >> 2, (leftPtr >> 2) + left.length));
        right.set(Module.HEAPF32.subarray(rightPtr >> 2, (rightPtr >> 2) + right.length));

        return true;
    }
}

registerProcessor('libopenmpt-processor', LibopenmptProcessor);
