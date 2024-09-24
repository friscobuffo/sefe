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

const sefeSvgOutputRed = document.getElementById('sefe-svg-red');
const sefeSvgOutputBlue = document.getElementById('sefe-svg-blue');

function visualizeSefe() {
    visualizeSvg('/embedding-blue.svg', sefeSvgOutputBlue);
    visualizeSvg('/embedding-red.svg', sefeSvgOutputRed);
}

const redInputLabel = document.getElementById('red-graph-upload');
const fileInput1 = document.getElementById('file1InputSefe');
fileInput1.addEventListener('change', (event) => {
    loadFile('/red.txt', event);
    redInputLabel.textContent = event.target.files[0].name;
});

const blueInputLabel = document.getElementById('blue-graph-upload');
const fileInput2 = document.getElementById('file2InputSefe');
fileInput2.addEventListener('change', (event) => {
    loadFile('/blue.txt', event);
    blueInputLabel.textContent = event.target.files[0].name;
});

function writeFile(fileName, content) {
    try {
        FS.writeFile(fileName, content);
        console.log(`File ${fileName} written successfully.`);
    } catch (e) {
        console.error('Error writing file:', e);
    }
}

function sefe() {
    let value = Module.ccall('sefeLoadedFiles', null, [], []);
    if (value === 1) {
        visualizeSefe();
    }
    else if (value === 0) {
        alert("Graphs do not admit a SEFE");
    }
}

const redSvgDownload = document.getElementById('download-svg-red');
redSvgDownload.addEventListener('click', (_) => {
    console.log("click");
    const svgContent = document.getElementById('sefe-svg-red').innerHTML;
    const svgFileName = "red.svg";

    const blob = new Blob([svgContent], { type: 'image/svg+xml' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = svgFileName;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
});