function loadFile(fileName, event) {
    const file = event.target.files[0];
    if (file) {
        const reader = new FileReader();
        reader.onload = function(e) {
            const contents = e.target.result;
            FS.writeFile(fileName, contents);
        };
        reader.readAsText(file);
    }
}

function visualizeSvg(fileName, svgElement) {
    try {
        const svgContent = FS.readFile(fileName, { encoding: 'utf8' });
        svgElement.innerHTML = svgContent;
    } catch (e) {
        console.error('Error loading file:', e);
    }
}

// AUSLANDER & PARTER

const fileInput = document.getElementById('fileInput');

fileInput.addEventListener('change', (event) => {
    loadFile('/input.txt', event);
});

function embed() {
    Module.ccall('embedLoadedFile', null, [], []);
}

const visualizeButton = document.getElementById('visualize');
const svgOutput = document.getElementById('svgOutput');

visualizeButton.addEventListener('click', () => {
    visualizeSvg('/embedding.svg', svgOutput);
});

// SEFE

function testSefe() {
    Module.ccall('sefeMainTest', null, [], []);
}

const sefeSvgOutputRed = document.getElementById('sefe-svg-output-red');
const sefeSvgOutputBlue = document.getElementById('sefe-svg-output-blue');

function visualizeSefe() {
    visualizeSvg('/embedding-blue.svg', sefeSvgOutputBlue);
    visualizeSvg('/embedding-red.svg', sefeSvgOutputRed);
}

const fileInput1 = document.getElementById('file1InputSefe');
fileInput1.addEventListener('change', (event) => {
    loadFile('/red.txt', event);
});

const fileInput2 = document.getElementById('file2InputSefe');
fileInput2.addEventListener('change', (event) => {
    loadFile('/blue.txt', event);
});

function sefe() {
    Module.ccall('sefeLoadedFiles', null, [], []);
}